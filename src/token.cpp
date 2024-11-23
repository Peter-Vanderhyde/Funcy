#include "token.h"
#include <unordered_map>


std::unordered_map<TokenType, std::string> token_labels{
    {TokenType::_Integer, "integer"},
    {TokenType::_Float, "float"},
    {TokenType::_Boolean, "boolean"},
    {TokenType::_String, "string"},
    {TokenType::_Identifier, "identifier"},
    {TokenType::_Plus, "+"},
    {TokenType::_Minus, "-"},
    {TokenType::_Multiply, "*"},
    {TokenType::_DoubleMultiply, "**"},
    {TokenType::_Divide, "/"},
    {TokenType::_DoubleDivide, "//"},
    {TokenType::_OpenParen, "("},
    {TokenType::_CloseParen, ")"},
    {TokenType::_Semi, ";"},
    {TokenType::_Caret, "^"},
    {TokenType::_Dot, "."},
    {TokenType::_Equals, "="},
    {TokenType::_EOF, "endOfFile"},
    {TokenType::_Compare, "=="},
    {TokenType::_And, "and"},
    {TokenType::_Or, "or"},
    {TokenType::_Not, "not"},
    {TokenType::_Exclamation, "!"}
    //{TokenType::, "T:"}
};

std::string getTokenTypeLabel(TokenType type) {
    if (token_labels.find(type) != token_labels.end()) {
        return token_labels[type];
    }
    else {
        return "T:UNKNOWN";
    }
}

std::unordered_map<char, TokenType> char_tokens {
    {';', TokenType::_Semi},
    {'+', TokenType::_Plus},
    {'-', TokenType::_Minus},
    {'*', TokenType::_Multiply},
    {'/', TokenType::_Divide},
    {'^', TokenType::_Caret},
    {'(', TokenType::_OpenParen},
    {')', TokenType::_CloseParen},
    {'=', TokenType::_Equals},
    {'!', TokenType::_Exclamation}
};

std::unordered_map<std::string, TokenType> keyword_tokens {
    {"and", TokenType::_And},
    {"or", TokenType::_Or},
    {"not", TokenType::_Not}
};


Token::Token(TokenType type, TokenValue value, int line, int column)
    : type{type}, value{value}, line{line}, column{column} {}

Token::Token(TokenType type, int line, int column)
    : type{type}, value{0}, line{line}, column{column} {}