#include "highlighter.h"

namespace {


const char* ansiCode(Style style) {
    switch (style) {
        case Style::Keyword:  return "\033[1;34m";
        case Style::Type:     return "\033[36m";
        case Style::Function: return "\033[33m";
        case Style::Number:   return "\033[38;5;208m";
        case Style::Text:     return "\033[32m";
        case Style::Operator: return "\033[37m";
        case Style::Comment:  return "\033[3;90m";
        case Style::Error:    return "\033[4;31m";   
        default:              return "";              
    }
}

const char* cssClass(Style style) {
    switch (style) {
        case Style::Keyword:  return "kw";
        case Style::Type:     return "ty";
        case Style::Variable: return "var";
        case Style::Function: return "fn";
        case Style::Number:   return "lit";
        case Style::Text:     return "str";
        case Style::Operator: return "op";
        case Style::Comment:  return "cm";
        case Style::Error:    return "err";
        default:              return "";
    }
}

const char* kHtmlHead =
    "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "<meta charset=\"utf-8\">\n"
    "<title>";

const char* kHtmlStyle =
    "</title>\n"
    "<style>\n"
    "body { background: #1e1e1e; margin: 0; padding: 1.5em; }\n"
    "pre  { font-family: Consolas, \"DejaVu Sans Mono\", Menlo, monospace;\n"
    "       font-size: 14px; line-height: 1.5; color: #e6e6e6; }\n"
    ".kw  { color: #4da3ff; font-weight: bold; }\n"
    ".ty  { color: #2ec4b6; }\n"
    ".var { color: #f0f0f0; }\n"
    ".fn  { color: #e6c229; }\n"
    ".lit { color: #ff9f43; }\n"
    ".str { color: #8fc97a; }\n"
    ".op  { color: #b0b0b0; }\n"
    ".cm  { color: #7a7a7a; font-style: italic; }\n"
    ".err { color: #ff5555; text-decoration: underline wavy #ff5555; }\n"
    "</style>\n"
    "</head>\n"
    "<body>\n"
    "<pre>";

const char* kHtmlTail =
    "</pre>\n"
    "</body>\n"
    "</html>\n";

std::string htmlEscape(const std::string& text) {
    std::string out;
    for (char c : text) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default:  out += c;
        }
    }
    return out;
}

std::string ansiWrap(const std::string& text, const char* code) {
    std::string out;
    std::string line;
    for (char c : text) {
        if (c == '\n') {
            if (!line.empty()) out += code + line + "\033[0m";
            out += '\n';
            line.clear();
        } else {
            line += c;
        }
    }
    if (!line.empty()) out += code + line + "\033[0m";
    return out;
}

} 

void Highlighter::analyze(const Node& root) { visit(root); }

void Highlighter::visit(const Node& node) {
    std::pair<int, int> where(node.loc.line, node.loc.col);
    switch (node.kind) {
        case NodeKind::FuncDecl:
        case NodeKind::CallExpr:
            identStyles_[where] = Style::Function;
            break;
        case NodeKind::VarDecl:
        case NodeKind::Param:
        case NodeKind::Identifier:
            identStyles_[where] = Style::Variable;
            break;
        default:
            break;
    }
    for (const NodePtr& child : node.children) {
        visit(*child);
    }
}

Style Highlighter::styleOf(const Token& token) const {
    switch (token.type) {
        case TokenType::KEYWORD:
            return isTypeKeyword(token.lexeme) ? Style::Type : Style::Keyword;
        case TokenType::IDENT: {
            auto it = identStyles_.find({token.loc.line, token.loc.col});
            return it != identStyles_.end() ? it->second : Style::Variable;
        }
        case TokenType::INT_LIT:
        case TokenType::BOOL_LIT:
            return Style::Number;
        case TokenType::STRING_LIT:
        case TokenType::CHAR_LIT:
            return Style::Text;
        case TokenType::OP:
        case TokenType::ASSIGN:
        case TokenType::LPAREN:
        case TokenType::RPAREN:
        case TokenType::LBRACE:
        case TokenType::RBRACE:
        case TokenType::LBRACKET:
        case TokenType::RBRACKET:
        case TokenType::SEMI:
        case TokenType::COLON:
        case TokenType::COMMA:
            return Style::Operator;
        case TokenType::LINE_COMMENT:
        case TokenType::BLOCK_COMMENT:
            return Style::Comment;
        case TokenType::INVALID:
            return Style::Error;
        default:
            return Style::Plain; 
    }
}

std::string Highlighter::render(const std::vector<Token>& tokens, Format format,
                                const std::string& title) const {
    std::string out;
    if (format == Format::Html) out += kHtmlHead + htmlEscape(title) + kHtmlStyle;

    for (const Token& token : tokens) {
        if (token.type == TokenType::END_OF_FILE) continue;
        Style style = styleOf(token);

        if (format == Format::PlainText || style == Style::Plain) {
            out += format == Format::Html ? htmlEscape(token.lexeme) : token.lexeme;
        } else if (format == Format::Ansi) {
            const char* code = ansiCode(style);
            out += *code ? ansiWrap(token.lexeme, code) : token.lexeme;
        } else {
            out += std::string("<span class=\"") + cssClass(style) + "\">" +
                   htmlEscape(token.lexeme) + "</span>";
        }
    }

    if (format == Format::Html) out += kHtmlTail;
    return out;
}
