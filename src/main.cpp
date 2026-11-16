#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ast.h"
#include "lexer.h"
#include "parser.h"

namespace {

std::string escapeNewlines(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else if (c == '\t') out += "\\t";
        else out += c;
    }
    return out;
}

bool readFile(const std::string& path, std::string& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::stringstream buffer;
    buffer << in.rdbuf();
    out = buffer.str();
    return true;
}

void usage() {
    std::cerr << "usage: bminor (--tokens | --ast) <file.bminor>\n";
}

void printTokens(const std::vector<Token>& tokens) {
    for (const Token& t : tokens) {
        if (t.type == TokenType::WHITESPACE) continue;
        std::cout << t.loc.line << ":" << t.loc.col << "\t" << tokenTypeName(t.type)
                  << "\t" << escapeNewlines(t.lexeme) << "\n";
    }
}

}

int main(int argc, char** argv) {
    enum class Mode { None, Tokens, Ast };
    Mode mode = Mode::None;
    std::string path;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--tokens") {
            mode = Mode::Tokens;
        } else if (arg == "--ast") {
            mode = Mode::Ast;
        } else if (!arg.empty() && arg[0] == '-') {
            usage();
            return 2;
        } else {
            path = arg;
        }
    }
    if (mode == Mode::None || path.empty()) {
        usage();
        return 2;
    }

    std::string source;
    if (!readFile(path, source)) {
        std::cerr << "bminor: cannot open '" << path << "'\n";
        return 2;
    }

    Lexer lexer(source, path);
    std::vector<Token> tokens = lexer.tokenize();
    std::vector<Diagnostic> errors = lexer.errors();

    if (mode == Mode::Tokens) {
        printTokens(tokens);
    } else {
        Parser parser(tokens);
        NodePtr program = parser.parseProgram();
        dumpAst(*program, std::cout);
        errors.insert(errors.end(), parser.errors().begin(), parser.errors().end());
    }

    std::cout << std::flush;
    for (const Diagnostic& d : errors) {
        std::cerr << formatDiagnostic(d) << "\n";
    }
    return errors.empty() ? 0 : 1;
}