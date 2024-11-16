#pragma once
#include <variant>
#include <string>
#include <vector>

using TokenValue = std::variant<int, double, std::string>;

enum class TokenType{
    _Integer,
    _Float,
    _Plus,
    _Minus,
    _Multiply,
    _DoubleMultiply,
    _Divide,
    _DoubleDivide,
    _EOF,
    _Semi,
    _OpenParen,
    _CloseParen
};

std::string getTokenTypeLabel(TokenType type);

class Token {
public:
    TokenType type;
    TokenValue value;
    int line, column;

    Token(TokenType type, TokenValue value, int line, int column);
    Token(TokenType type, int line, int column);
};

class Lexer {
public:
    Lexer(const std::string& source_code);

    std::vector<Token> tokenize();
    void lexerError(std::string message, int line, int column);

private:
    std::string source_code;
    int current_position = 0;
    int line = 1;
    int column = 0;

    char grabNextCharacter();
    char peekNextCharacter(int ahead = 0);
};