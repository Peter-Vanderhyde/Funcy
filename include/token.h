#pragma once
#include <variant>
#include <string>
#include <unordered_map>

using TokenValue = std::variant<int, double, bool, std::string>;

enum class TokenType{
    _Integer,
    _Float,
    _Boolean,
    _String,
    _Identifier,
    _Plus,
    _Minus,
    _Multiply,
    _DoubleMultiply,
    _Divide,
    _DoubleDivide,
    _EOF,
    _Semi,
    _OpenParen,
    _CloseParen,
    _Caret,
    _Dot,
    _Equals,
    _Compare,
    _And,
    _Or,
    _Not,
    _Exclamation
};

std::string getTokenTypeLabel(TokenType type);

extern std::unordered_map<char, TokenType> char_tokens;

extern std::unordered_map<std::string, TokenType> keyword_tokens;

class Token {
public:
    TokenType type;
    TokenValue value;
    int line, column;

    Token(TokenType type, TokenValue value, int line, int column);
    Token(TokenType type, int line, int column);
};