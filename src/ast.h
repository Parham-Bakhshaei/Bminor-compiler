#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "token.h"

enum class NodeKind {
    Program,
    // declarations
    VarDecl, FuncDecl, Param,
    // types
    BasicType, ArrayType, FuncType,
    // statements
    Block, IfStmt, ForStmt, WhileStmt, PrintStmt, ReturnStmt, ExprStmt,
    // expressions
    AssignExpr, BinaryExpr, UnaryExpr, PostfixExpr, CallExpr, IndexExpr, ArrayInit,
    // leaves
    Identifier, IntLiteral, BoolLiteral, CharLiteral, StringLiteral,
    Empty
};

struct Node;
using NodePtr = std::unique_ptr<Node>;

struct Node {
    NodeKind kind;
    std::string text;
    Location loc; 
    std::string type;       
                            
    std::vector<NodePtr> children;

    Node(NodeKind k, std::string t, Location l)
        : kind(k), text(std::move(t)), loc(std::move(l)) {}

    bool isLeaf() const { return children.empty(); }
    void add(NodePtr child) { children.push_back(std::move(child)); }
};

NodePtr makeNode(NodeKind kind, const std::string& text, const Location& loc);

const char* kindName(NodeKind kind);

void dumpAst(const Node& node, std::ostream& out, int depth = 0);
