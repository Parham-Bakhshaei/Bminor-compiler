// diagnostic.h - error messages shared by the lexer and the parser.
#pragma once

#include <string>

#include "token.h"

struct Diagnostic {
    std::string phase;  
    Location loc;
    std::string message;
};

// file:line:col: phase error: message
inline std::string formatDiagnostic(const Diagnostic& d) {
    return toString(d.loc) + ": " + d.phase + " error: " + d.message;
}
