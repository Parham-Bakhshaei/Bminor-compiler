// token.h - token types, source locations and small helpers.
#pragma once

#include <string>

// Where a token (or an AST node) starts in the source file.
struct Location {
    std::string file;
    int line = 1;  
    int col = 1;   
};

inline std::string toString(const Location& loc) {
    return loc.file + ":" + std::to_string(loc.line) + ":" + std::to_string(loc.col);
}

enum class TokenType {
    // words
    KEYWORD, IDENT,
    // literals
    INT_LIT, BOOL_LIT, STRING_LIT, CHAR_LIT,
    // operators and delimiters
    OP, ASSIGN,
    LPAREN, RPAREN, LBRACE, RBRACE, LBRACKET, RBRACKET,
    SEMI, COLON, COMMA,
    // "trivia": the lexer keeps them so the highlighter can rebuild the
    // source exactly; the parser ignores them
    WHITESPACE, LINE_COMMENT, BLOCK_COMMENT,
    // special
    INVALID, END_OF_FILE
};

struct Token {
    TokenType type = TokenType::END_OF_FILE;
    std::string lexeme;   // exactly the text matched in the source
    Location loc;         // position of the first character
};

const char* tokenTypeName(TokenType type);

bool isKeyword(const std::string& word);      // array, if, return, ...
bool isTypeKeyword(const std::string& word);  // integer, boolean, char, string, void, auto
bool isTrivia(TokenType type);                // whitespace and comments

// Short human readable form, e.g. KW(if), IDENT(n), INT(42), OP(<=), SEMI
std::string describe(const Token& token);
