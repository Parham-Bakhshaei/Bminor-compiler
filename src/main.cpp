#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "lexer.h"

namespace {

// Newlines inside a lexeme would break the one-token-per-line output.
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
    std::cerr << "usage: bminor --tokens <file.bminor>\n";
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string(argv[1]) != "--tokens") {
        usage();
        return 2;
    }

    std::string path = argv[2];
    std::string source;
    if (!readFile(path, source)) {
        std::cerr << "bminor: cannot open '" << path << "'\n";
        return 2;
    }

    Lexer lexer(source, path);
    std::vector<Token> tokens = lexer.tokenize();

    for (const Token& t : tokens) {
        if (t.type == TokenType::WHITESPACE) continue;
        std::cout << t.loc.line << ":" << t.loc.col << "\t" << tokenTypeName(t.type)
                  << "\t" << escapeNewlines(t.lexeme) << "\n";
    }
    std::cout << std::flush;

    for (const Diagnostic& d : lexer.errors()) {
        std::cerr << formatDiagnostic(d) << "\n";
    }
    return lexer.errors().empty() ? 0 : 1;
}
