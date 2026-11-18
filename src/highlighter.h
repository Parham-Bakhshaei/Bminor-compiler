#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "ast.h"
#include "token.h"

enum class Style {
    Plain, Keyword, Type, Variable, Function, Number, Text, Operator, Comment, Error
};

enum class Format {
    Ansi,        
    Html,        
    PlainText    
};

class Highlighter {
public:
    void analyze(const Node& root);

    std::string render(const std::vector<Token>& tokens, Format format,
                       const std::string& title) const;

private:
    void visit(const Node& node);
    Style styleOf(const Token& token) const;

    std::map<std::pair<int, int>, Style> identStyles_;
};
