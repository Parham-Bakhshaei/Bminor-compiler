#include "token.h"

#include <set>

const char* tokenTypeName(TokenType type) {
    switch (type) {
        case TokenType::KEYWORD:       return "KEYWORD";
        case TokenType::IDENT:         return "IDENT";
        case TokenType::INT_LIT:       return "INT_LIT";
        case TokenType::BOOL_LIT:      return "BOOL_LIT";
        case TokenType::STRING_LIT:    return "STRING_LIT";
        case TokenType::CHAR_LIT:      return "CHAR_LIT";
        case TokenType::OP:            return "OP";
        case TokenType::ASSIGN:        return "ASSIGN";
        case TokenType::LPAREN:        return "LPAREN";
        case TokenType::RPAREN:        return "RPAREN";
        case TokenType::LBRACE:        return "LBRACE";
        case TokenType::RBRACE:        return "RBRACE";
        case TokenType::LBRACKET:      return "LBRACKET";
        case TokenType::RBRACKET:      return "RBRACKET";
        case TokenType::SEMI:          return "SEMI";
        case TokenType::COLON:         return "COLON";
        case TokenType::COMMA:         return "COMMA";
        case TokenType::WHITESPACE:    return "WHITESPACE";
        case TokenType::LINE_COMMENT:  return "LINE_COMMENT";
        case TokenType::BLOCK_COMMENT: return "BLOCK_COMMENT";
        case TokenType::INVALID:       return "INVALID";
        case TokenType::END_OF_FILE:   return "EOF";
    }
    return "UNKNOWN";
}

bool isKeyword(const std::string& word) {
    static const std::set<std::string> keywords = {
        "array", "auto", "boolean", "char", "else", "for", "function",
        "if", "integer", "print", "return", "string", "void", "while"};
    return keywords.count(word) > 0;
}

bool isTypeKeyword(const std::string& word) {
    static const std::set<std::string> types = {
        "integer", "boolean", "char", "string", "void", "auto"};
    return types.count(word) > 0;
}

bool isTrivia(TokenType type) {
    return type == TokenType::WHITESPACE || type == TokenType::LINE_COMMENT ||
           type == TokenType::BLOCK_COMMENT;
}

std::string describe(const Token& t) {
    switch (t.type) {
        case TokenType::KEYWORD:    return "KW(" + t.lexeme + ")";
        case TokenType::IDENT:      return "IDENT(" + t.lexeme + ")";
        case TokenType::INT_LIT:    return "INT(" + t.lexeme + ")";
        case TokenType::BOOL_LIT:   return "BOOL(" + t.lexeme + ")";
        case TokenType::STRING_LIT: return "STR(" + t.lexeme + ")";
        case TokenType::CHAR_LIT:   return "CHAR(" + t.lexeme + ")";
        case TokenType::OP:         return "OP(" + t.lexeme + ")";
        case TokenType::INVALID:    return "INVALID(" + t.lexeme + ")";
        default:                    return tokenTypeName(t.type);
    }
}
