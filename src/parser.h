#pragma once

#include <string>
#include <vector>

#include "ast.h"
#include "diagnostic.h"
#include "token.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    NodePtr parseProgram();

    const std::vector<Diagnostic>& errors() const { return errors_; }

private:
    struct ParseError {};   
    const Token& peek(size_t offset = 0) const;   
    const Token& previous() const;                
    const Token& advance();
    bool atEnd() const { return peek().type == TokenType::END_OF_FILE; }
    bool check(TokenType type) const { return peek().type == type; }
    bool checkOp(const std::string& op) const;
    bool checkAnyOp(const std::vector<std::string>& ops) const;
    bool checkKeyword(const std::string& word) const;
    bool match(TokenType type);
    bool matchKeyword(const std::string& word);
    const Token& expect(TokenType type, const std::string& what);

    void errorAt(const Token& token, const std::string& message);            
    [[noreturn]] void fail(const Token& token, const std::string& message); 
    bool atSyncPoint() const;
    void synchronize();
    void recover(size_t constructStart);

    NodePtr parseDeclaration();
    NodePtr parseType();
    NodePtr parseParam();
    NodePtr parseArrayInit();

    NodePtr parseStatement();
    NodePtr parseBlock();
    NodePtr parseIf();
    NodePtr parseFor();
    NodePtr parseWhile();
    NodePtr parseReturn();
    NodePtr parsePrint();
    NodePtr parseExprStatement();

    NodePtr parseExpr();
    NodePtr parseAssignment();
    NodePtr parseBinary(size_t level); 
    NodePtr parsePower();
    NodePtr parseUnary();
    NodePtr parsePostfix();
    NodePtr parsePrimary();

    std::vector<Token> tokens_;
    size_t pos_ = 0;
    std::vector<Diagnostic> errors_;
    size_t syncPos_ = static_cast<size_t>(-1);  
};
