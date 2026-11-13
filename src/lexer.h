// lexer.h - turns source text into a stream of tokens.
#pragma once

#include <string>
#include <vector>

#include "diagnostic.h"
#include "token.h"

class Lexer {
public:
    Lexer(const std::string& source, const std::string& filename);

    // Scans the whole input. The result contains every token (including
    // whitespace and comments) and ends with END_OF_FILE.
    std::vector<Token> tokenize();

    const std::vector<Diagnostic>& errors() const { return errors_; }

private:
    // --- character helpers ---
    bool atEnd() const { return pos_ >= src_.size(); }
    char peek() const { return atEnd() ? '\0' : src_[pos_]; }
    char peekNext() const { return pos_ + 1 < src_.size() ? src_[pos_ + 1] : '\0'; }
    char advance();                       // consumes one char, updates line/col

    // --- token builders ---
    Token nextToken();
    Token make(TokenType type, size_t startPos, const Location& start) const;
    Token scanWhitespace(size_t startPos, const Location& start);
    Token scanWord(size_t startPos, const Location& start);      // ident / keyword / bool
    Token scanNumber(size_t startPos, const Location& start);
    Token scanOperator(size_t startPos, const Location& start);  // operators, delimiters, invalid

    void report(const Location& loc, const std::string& message);

    std::string src_;
    std::string file_;
    size_t pos_ = 0;
    int line_ = 1;
    int col_ = 1;
    std::vector<Diagnostic> errors_;
};
