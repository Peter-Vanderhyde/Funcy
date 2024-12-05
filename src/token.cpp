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
    {TokenType::_EOF, "eof"},
    {TokenType::_Compare, "=="},
    {TokenType::_And, "and"},
    {TokenType::_Or, "or"},
    {TokenType::_Not, "not"},
    {TokenType::_Exclamation, "!"},
    {TokenType::_If, "if"},
    {TokenType::_Elif, "elif"},
    {TokenType::_Else, "else"},
    {TokenType::_CurlyOpen, "{"},
    {TokenType::_CurlyClose, "}"},
    {TokenType::_While, "while"}
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
    {'!', TokenType::_Exclamation},
    {'{', TokenType::_CurlyOpen},
    {'}', TokenType::_CurlyClose}
};

std::unordered_map<std::string, TokenType> keyword_tokens {
    {"and", TokenType::_And},
    {"or", TokenType::_Or},
    {"not", TokenType::_Not},
    {"if", TokenType::_If},
    {"elif", TokenType::_Elif},
    {"else", TokenType::_Else},
    {"while", TokenType::_While}
};

std::unordered_map<std::string, TokenType> scoped_keyword_tokens {
    {"if", TokenType::_If},
    {"elif", TokenType::_Elif},
    {"else", TokenType::_Else},
    {"while", TokenType::_While}
    // Also change in parser.cpp
};


Token::Token(TokenType type, TokenValue value, int line, int column)
    : type{type}, value{value}, line{line}, column{column} {}

Token::Token(TokenType type, int line, int column)
    : type{type}, value{0}, line{line}, column{column} {}