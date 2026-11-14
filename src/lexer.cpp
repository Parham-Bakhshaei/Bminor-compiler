#include "lexer.h"

namespace {

bool isLetter(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }
bool isDigit(char c) { return c >= '0' && c <= '9'; }
bool isSpace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

}  

Lexer::Lexer(const std::string& source, const std::string& filename)
    : src_(source), file_(filename) {}

char Lexer::advance() {
    char c = src_[pos_++];
    if (c == '\n') {
        line_++;
        col_ = 1;
    } else {
        col_++;
    }
    return c;
}

void Lexer::report(const Location& loc, const std::string& message) {
    errors_.push_back({"lexical", loc, message});
}

Token Lexer::make(TokenType type, size_t startPos, const Location& start) const {
    return Token{type, src_.substr(startPos, pos_ - startPos), start};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    while (!atEnd()) {
        tokens.push_back(nextToken());
    }
    tokens.push_back(Token{TokenType::END_OF_FILE, "", Location{file_, line_, col_}});
    return tokens;
}

Token Lexer::nextToken() {
    size_t startPos = pos_;
    Location start{file_, line_, col_};
    char c = peek();

    if (isSpace(c)) return scanWhitespace(startPos, start);
    if (isLetter(c)) return scanWord(startPos, start);
    if (isDigit(c)) return scanNumber(startPos, start);
    if (c == '"') return scanString(startPos, start);
    if (c == '\'') return scanChar(startPos, start);
    return scanOperator(startPos, start);
}

Token Lexer::scanWhitespace(size_t startPos, const Location& start) {
    while (!atEnd() && isSpace(peek())) advance();
    return make(TokenType::WHITESPACE, startPos, start);
}


Token Lexer::scanWord(size_t startPos, const Location& start) {
    while (!atEnd() && (isLetter(peek()) || isDigit(peek()))) advance();
    Token t = make(TokenType::IDENT, startPos, start);
    if (t.lexeme == "true" || t.lexeme == "false") {
        t.type = TokenType::BOOL_LIT;
    } else if (isKeyword(t.lexeme)) {
        t.type = TokenType::KEYWORD;
    }
    return t;
}


Token Lexer::scanNumber(size_t startPos, const Location& start) {
    while (!atEnd() && isDigit(peek())) advance();
    return make(TokenType::INT_LIT, startPos, start);
}

Token Lexer::scanString(size_t startPos, const Location& start) {
    advance();   // opening quote
    bool closed = false;
    while (!atEnd() && peek() != '\n') {
        char c = advance();
        if (c == '"') {
            closed = true;
            break;
        }
        if (c == '\\' && !atEnd() && peek() != '\n') advance();  
    }
    if (!closed) {
        report(start, "unterminated string literal");
        return make(TokenType::INVALID, startPos, start);
    }
    return make(TokenType::STRING_LIT, startPos, start);
}


Token Lexer::scanChar(size_t startPos, const Location& start) {
    advance();   

    if (peek() == '\'') {               
        advance();
        report(start, "empty character literal");
        return make(TokenType::INVALID, startPos, start);
    }

    bool closed = false;
    if (!atEnd() && peek() != '\n') {
        char c = advance();
        if (c == '\\' && !atEnd() && peek() != '\n') advance();   
        if (peek() == '\'') {
            advance();
            closed = true;
        }
    }
    if (!closed) {
        report(start, "malformed or unterminated character literal");
        return make(TokenType::INVALID, startPos, start);
    }
    return make(TokenType::CHAR_LIT, startPos, start);
}


Token Lexer::scanOperator(size_t startPos, const Location& start) {
    char c = advance();
    char n = peek();   

    switch (c) {
        case '(': return make(TokenType::LPAREN, startPos, start);
        case ')': return make(TokenType::RPAREN, startPos, start);
        case '{': return make(TokenType::LBRACE, startPos, start);
        case '}': return make(TokenType::RBRACE, startPos, start);
        case '[': return make(TokenType::LBRACKET, startPos, start);
        case ']': return make(TokenType::RBRACKET, startPos, start);
        case ';': return make(TokenType::SEMI, startPos, start);
        case ':': return make(TokenType::COLON, startPos, start);
        case ',': return make(TokenType::COMMA, startPos, start);

        case '=':
            if (n == '=') { advance(); return make(TokenType::OP, startPos, start); }
            return make(TokenType::ASSIGN, startPos, start);
        case '<':
        case '>':
        case '!':
            if (n == '=') advance();
            return make(TokenType::OP, startPos, start);
        case '+':
            if (n == '+') advance();
            return make(TokenType::OP, startPos, start);
        case '-':
            if (n == '-') advance();
            return make(TokenType::OP, startPos, start);
        case '*':
        case '/':
        case '%':
        case '^':
            return make(TokenType::OP, startPos, start);
        case '&':
            if (n == '&') { advance(); return make(TokenType::OP, startPos, start); }
            break;
        case '|':
            if (n == '|') { advance(); return make(TokenType::OP, startPos, start); }
            break;
        default:
            break;
    }


    while (!atEnd() && (static_cast<unsigned char>(peek()) & 0xC0) == 0x80) advance();
    Token bad = make(TokenType::INVALID, startPos, start);
    report(start, "invalid character '" + bad.lexeme + "'");
    return bad;
}
