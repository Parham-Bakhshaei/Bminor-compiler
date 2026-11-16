#include "ast.h"

NodePtr makeNode(NodeKind kind, const std::string& text, const Location& loc) {
    return std::make_unique<Node>(kind, text, loc);
}

const char* kindName(NodeKind kind) {
    switch (kind) {
        case NodeKind::Program:       return "Program";
        case NodeKind::VarDecl:       return "VarDecl";
        case NodeKind::FuncDecl:      return "FuncDecl";
        case NodeKind::Param:         return "Param";
        case NodeKind::BasicType:     return "BasicType";
        case NodeKind::ArrayType:     return "ArrayType";
        case NodeKind::FuncType:      return "FuncType";
        case NodeKind::Block:         return "Block";
        case NodeKind::IfStmt:        return "IfStmt";
        case NodeKind::ForStmt:       return "ForStmt";
        case NodeKind::WhileStmt:     return "WhileStmt";
        case NodeKind::PrintStmt:     return "PrintStmt";
        case NodeKind::ReturnStmt:    return "ReturnStmt";
        case NodeKind::ExprStmt:      return "ExprStmt";
        case NodeKind::AssignExpr:    return "AssignExpr";
        case NodeKind::BinaryExpr:    return "BinaryExpr";
        case NodeKind::UnaryExpr:     return "UnaryExpr";
        case NodeKind::PostfixExpr:   return "PostfixExpr";
        case NodeKind::CallExpr:      return "CallExpr";
        case NodeKind::IndexExpr:     return "IndexExpr";
        case NodeKind::ArrayInit:     return "ArrayInit";
        case NodeKind::Identifier:    return "Identifier";
        case NodeKind::IntLiteral:    return "IntLiteral";
        case NodeKind::BoolLiteral:   return "BoolLiteral";
        case NodeKind::CharLiteral:   return "CharLiteral";
        case NodeKind::StringLiteral: return "StringLiteral";
        case NodeKind::Empty:         return "Empty";
    }
    return "Unknown";
}

void dumpAst(const Node& node, std::ostream& out, int depth) {
    out << std::string(depth * 2, ' ') << kindName(node.kind);
    if (!node.text.empty()) out << " '" << node.text << "'";
    out << " @" << node.loc.line << ":" << node.loc.col;
    if (!node.type.empty()) out << " : " << node.type;
    out << "\n";
    for (const NodePtr& child : node.children) {
        dumpAst(*child, out, depth + 1);
    }
}
