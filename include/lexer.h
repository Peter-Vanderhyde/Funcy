#pragma once
#include <variant>
#include <string>
#include <vector>
#include "token.h"

class Lexer {
public:
    Lexer(const std::string& source_code);

    std::vector<Token> tokenize();
    [[noreturn]] void lexerError(std::string message, int line, int column) const;

private:
    std::string source_code;
    int current_position = 0;
    int line = 1;
    int column = 0;

    char grabNextCharacter();
    char peekNextCharacter(int ahead = 0);
};