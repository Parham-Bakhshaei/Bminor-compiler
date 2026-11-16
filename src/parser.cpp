#include "parser.h"

namespace {
std::string found(const Token& token) {
    if (token.type == TokenType::END_OF_FILE) return "end of file";
    return "'" + token.lexeme + "'";
}

}

Parser::Parser(const std::vector<Token>& tokens) {
    for (const Token& t : tokens) {
        if (isTrivia(t.type) || t.type == TokenType::INVALID) continue;
        tokens_.push_back(t);
    }

    if (tokens_.empty() || tokens_.back().type != TokenType::END_OF_FILE) {
        Location where = tokens_.empty() ? Location{} : tokens_.back().loc;
        tokens_.push_back(Token{TokenType::END_OF_FILE, "", where});
    }
}

const Token& Parser::peek(size_t offset) const {
    size_t i = pos_ + offset;
    if (i >= tokens_.size()) i = tokens_.size() - 1;
    return tokens_[i];
}

const Token& Parser::advance() {
    const Token& current = tokens_[pos_];
    if (!atEnd()) pos_++;
    return current;
}

bool Parser::checkOp(const std::string& op) const {
    return peek().type == TokenType::OP && peek().lexeme == op;
}

bool Parser::checkAnyOp(const std::vector<std::string>& ops) const {
    for (const std::string& op : ops) {
        if (checkOp(op)) return true;
    }
    return false;
}

bool Parser::checkKeyword(const std::string& word) const {
    return peek().type == TokenType::KEYWORD && peek().lexeme == word;
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

bool Parser::matchKeyword(const std::string& word) {
    if (!checkKeyword(word)) return false;
    advance();
    return true;
}

const Token& Parser::expect(TokenType type, const std::string& what) {
    if (!check(type)) fail(peek(), "expected " + what + ", got " + found(peek()));
    return advance();
}

void Parser::errorAt(const Token& token, const std::string& message) {
    errors_.push_back({"syntax", token.loc, message});
}

void Parser::fail(const Token& token, const std::string& message) {
    errorAt(token, message);
    throw ParseError{};
}

NodePtr Parser::parseProgram() {
    NodePtr program = makeNode(NodeKind::Program, "", peek().loc);
    try {
        while (!atEnd()) {
            program->add(parseDeclaration());
        }
    } catch (const ParseError&) {

    }
    return program;
}

NodePtr Parser::parseDeclaration() {
    if (!check(TokenType::IDENT)) fail(peek(), "expected declaration, got " + found(peek()));
    const Token& name = advance();
    expect(TokenType::COLON, "':' after the declared name");

    NodePtr type = parseType();

    if (type->kind == NodeKind::FuncType) {
        NodePtr func = makeNode(NodeKind::FuncDecl, name.lexeme, name.loc);
        func->add(std::move(type));
        if (match(TokenType::SEMI)) return func;
        expect(TokenType::ASSIGN, "'=' or ';' after the function type");
        func->add(parseBlock());
        return func;
    }

    NodePtr var = makeNode(NodeKind::VarDecl, name.lexeme, name.loc);
    var->add(std::move(type));
    if (match(TokenType::ASSIGN)) {
        var->add(check(TokenType::LBRACE) ? parseArrayInit() : parseExpr());
    }
    expect(TokenType::SEMI, "';' after the declaration");
    return var;
}

NodePtr Parser::parseType() {
    const Token& first = peek();

    if (matchKeyword("array")) {
        NodePtr node = makeNode(NodeKind::ArrayType, "", first.loc);
        expect(TokenType::LBRACKET, "'[' after 'array'");
        NodePtr size;
        if (!check(TokenType::RBRACKET)) size = parseExpr();
        expect(TokenType::RBRACKET, "']' in the array type");
        node->add(parseType());
        if (size) node->add(std::move(size));
        return node;
    }

    if (matchKeyword("function")) {
        NodePtr node = makeNode(NodeKind::FuncType, "", first.loc);
        node->add(parseType());
        expect(TokenType::LPAREN, "'(' after the return type");
        if (!check(TokenType::RPAREN)) {
            do {
                node->add(parseParam());
            } while (match(TokenType::COMMA));
        }
        expect(TokenType::RPAREN, "')' after the parameters");
        return node;
    }

    if (check(TokenType::KEYWORD) && isTypeKeyword(first.lexeme)) {
        advance();
        return makeNode(NodeKind::BasicType, first.lexeme, first.loc);
    }

    fail(first, "expected a type, got " + found(first));
}

NodePtr Parser::parseParam() {
    const Token& name = expect(TokenType::IDENT, "parameter name");
    expect(TokenType::COLON, "':' after the parameter name");
    NodePtr param = makeNode(NodeKind::Param, name.lexeme, name.loc);
    param->add(parseType());
    return param;
}

NodePtr Parser::parseArrayInit() {
    const Token& open = expect(TokenType::LBRACE, "'{'");
    NodePtr init = makeNode(NodeKind::ArrayInit, "", open.loc);
    if (!check(TokenType::RBRACE)) {
        do {
            init->add(check(TokenType::LBRACE) ? parseArrayInit() : parseExpr());
        } while (match(TokenType::COMMA));
    }
    expect(TokenType::RBRACE, "'}' to close the array initializer");
    return init;
}

NodePtr Parser::parseStatement() {
    if (check(TokenType::LBRACE)) return parseBlock();

    if (check(TokenType::IDENT) && peek(1).type == TokenType::COLON) return parseDeclaration();

    if (checkKeyword("return")) return parseReturn();
    if (checkKeyword("print")) return parsePrint();
    return parseExprStatement();
}

NodePtr Parser::parseBlock() {
    const Token& open = expect(TokenType::LBRACE, "'{'");
    NodePtr block = makeNode(NodeKind::Block, "", open.loc);
    while (!check(TokenType::RBRACE) && !atEnd()) {
        block->add(parseStatement());
    }
    expect(TokenType::RBRACE, "'}' to close the block");
    return block;
}

NodePtr Parser::parseReturn() {
    const Token& keyword = advance();
    NodePtr node = makeNode(NodeKind::ReturnStmt, "", keyword.loc);
    if (!check(TokenType::SEMI)) node->add(parseExpr());
    expect(TokenType::SEMI, "';' after 'return'");
    return node;
}

NodePtr Parser::parsePrint() {
    const Token& keyword = advance();
    NodePtr node = makeNode(NodeKind::PrintStmt, "", keyword.loc);
    if (!check(TokenType::SEMI)) {
        do {
            node->add(parseExpr());
        } while (match(TokenType::COMMA));
    }
    expect(TokenType::SEMI, "';' after 'print'");
    return node;
}

NodePtr Parser::parseExprStatement() {
    Location start = peek().loc;
    NodePtr node = makeNode(NodeKind::ExprStmt, "", start);
    node->add(parseExpr());
    expect(TokenType::SEMI, "';' after the expression");
    return node;
}

NodePtr Parser::parseExpr() { return parseAssignment(); }

NodePtr Parser::parseAssignment() {
    NodePtr left = parseBinary(0);
    if (check(TokenType::ASSIGN)) {
        const Token& equals = advance();
        NodePtr value = parseAssignment();

        if (left->kind != NodeKind::Identifier && left->kind != NodeKind::IndexExpr) {
            errorAt(equals, "invalid assignment target");
        }
        NodePtr node = makeNode(NodeKind::AssignExpr, "=", left->loc);
        node->add(std::move(left));
        node->add(std::move(value));
        return node;
    }
    return left;
}

NodePtr Parser::parseBinary(size_t level) {

    static const std::vector<std::vector<std::string>> levels = {
        {"||"},
        {"&&"},
        {"<", "<=", ">", ">=", "==", "!="},
        {"+", "-"},
        {"*", "/", "%"}};

    if (level == levels.size()) return parsePower();

    NodePtr left = parseBinary(level + 1);
    while (checkAnyOp(levels[level])) {
        const Token& op = advance();
        NodePtr right = parseBinary(level + 1);
        NodePtr node = makeNode(NodeKind::BinaryExpr, op.lexeme, left->loc);
        node->add(std::move(left));
        node->add(std::move(right));
        left = std::move(node);
    }
    return left;
}

NodePtr Parser::parsePower() {
    NodePtr base = parseUnary();
    if (checkOp("^")) {
        const Token& op = advance();
        NodePtr exponent = parsePower();
        NodePtr node = makeNode(NodeKind::BinaryExpr, op.lexeme, base->loc);
        node->add(std::move(base));
        node->add(std::move(exponent));
        return node;
    }
    return base;
}

NodePtr Parser::parseUnary() {
    if (checkOp("-") || checkOp("!")) {
        const Token& op = advance();
        NodePtr node = makeNode(NodeKind::UnaryExpr, op.lexeme, op.loc);
        node->add(parseUnary());
        return node;
    }
    return parsePostfix();
}

NodePtr Parser::parsePostfix() {
    NodePtr expr = parsePrimary();
    while (true) {
        if (match(TokenType::LBRACKET)) {
            NodePtr index = parseExpr();
            expect(TokenType::RBRACKET, "']' after the index");
            NodePtr node = makeNode(NodeKind::IndexExpr, "", expr->loc);
            node->add(std::move(expr));
            node->add(std::move(index));
            expr = std::move(node);
        } else if (checkOp("++") || checkOp("--")) {
            const Token& op = advance();
            NodePtr node = makeNode(NodeKind::PostfixExpr, op.lexeme, expr->loc);
            node->add(std::move(expr));
            expr = std::move(node);
        } else {
            break;
        }
    }
    return expr;
}

NodePtr Parser::parsePrimary() {
    const Token& tok = peek();
    switch (tok.type) {
        case TokenType::INT_LIT:
            advance();
            return makeNode(NodeKind::IntLiteral, tok.lexeme, tok.loc);
        case TokenType::BOOL_LIT:
            advance();
            return makeNode(NodeKind::BoolLiteral, tok.lexeme, tok.loc);
        case TokenType::CHAR_LIT:
            advance();
            return makeNode(NodeKind::CharLiteral, tok.lexeme, tok.loc);
        case TokenType::STRING_LIT:
            advance();
            return makeNode(NodeKind::StringLiteral, tok.lexeme, tok.loc);

        case TokenType::IDENT: {
            advance();
            if (match(TokenType::LPAREN)) {
                NodePtr call = makeNode(NodeKind::CallExpr, tok.lexeme, tok.loc);
                if (!check(TokenType::RPAREN)) {
                    do {
                        call->add(parseExpr());
                    } while (match(TokenType::COMMA));
                }
                expect(TokenType::RPAREN, "')' after the arguments");
                return call;
            }
            return makeNode(NodeKind::Identifier, tok.lexeme, tok.loc);
        }

        case TokenType::LPAREN: {
            advance();
            NodePtr inner = parseExpr();
            expect(TokenType::RPAREN, "')' to close the parenthesis");
            return inner;
        }

        default:
            fail(tok, "expected expression, got " + found(tok));
    }
}