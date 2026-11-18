#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ast.h"
#include "highlighter.h"
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
    std::cerr <<
        "usage: bminor [options] <file.bminor>\n"
        "  --tokens            print the token stream\n"
        "  --ast               print the syntax tree\n"
        "  --highlight=ansi    colored source for the terminal (default)\n"
        "  --highlight=html    standalone HTML page\n"
        "  --highlight=plain   the source without colors\n"
        "  -o <file>           write the output to a file instead of stdout\n";
}

void printTokens(const std::vector<Token>& tokens, std::ostream& out) {
    for (const Token& t : tokens) {
        if (t.type == TokenType::WHITESPACE) continue;
        out << t.loc.line << ":" << t.loc.col << "\t" << tokenTypeName(t.type)
            << "\t" << escapeNewlines(t.lexeme) << "\n";
    }
}

}  

int main(int argc, char** argv) {
    enum class Mode { Tokens, Ast, Highlight };
    Mode mode = Mode::Highlight;
    Format format = Format::Ansi;
    std::string path;
    std::string outputPath;

    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--tokens") {
            mode = Mode::Tokens;
        } else if (arg == "--ast") {
            mode = Mode::Ast;
        } else if (arg == "--highlight=ansi") {
            mode = Mode::Highlight;
            format = Format::Ansi;
        } else if (arg == "--highlight=html") {
            mode = Mode::Highlight;
            format = Format::Html;
        } else if (arg == "--highlight=plain") {
            mode = Mode::Highlight;
            format = Format::PlainText;
        } else if (arg == "-o" && i + 1 < argc) {
            outputPath = argv[++i];
        } else if (!arg.empty() && arg[0] == '-') {
            usage();
            return 2;
        } else {
            path = arg;
        }
    }
    if (path.empty()) {
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

    std::ostringstream out;
    if (mode == Mode::Tokens) {
        printTokens(tokens, out);
    } else {
        Parser parser(tokens);
        NodePtr program = parser.parseProgram();
        errors.insert(errors.end(), parser.errors().begin(), parser.errors().end());

        if (mode == Mode::Ast) {
            dumpAst(*program, out);
        } else {
            Highlighter highlighter;
            highlighter.analyze(*program);
            out << highlighter.render(tokens, format, path);
        }
    }

    if (outputPath.empty()) {
        std::cout << out.str() << std::flush;
    } else {
        std::ofstream file(outputPath, std::ios::binary);
        if (!file) {
            std::cerr << "bminor: cannot write '" << outputPath << "'\n";
            return 2;
        }
        file << out.str();
    }

    for (const Diagnostic& d : errors) {
        std::cerr << formatDiagnostic(d) << "\n";
    }
    return errors.empty() ? 0 : 1;
}
