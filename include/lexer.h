#pragma once
#include <variant>
#include <string>
#include <vector>
#include <map>
#include <iostream>

enum class TokenType{
    _Identifier,
    _Float,
    _Integer,
    _String,
    _Semi,
    _Plus,
    _Minus,
    _Multiply,
    _Divide,
    _Equals,
    _OpenParen,
    _CloseParen,
    _EndOfFile,
    _Boolean,
    _OpenCurly,
    _CloseCurly,
    _Caret
};

std::ostream& operator<<(std::ostream& os, TokenType type);

extern std::map<TokenType, std::string> token_labels;

extern std::map<std::string, TokenType> keyword_tokens;

extern std::map<char, TokenType> char_tokens;

class Token{
public:
    TokenType type;
    std::variant<int, double, std::string, bool> value;
    int line, column;

    Token(TokenType type, const std::variant<int, double, std::string, bool>& value, int line, int column)
        : type{type}, value{value}, line{line}, column{column} {}
    
    Token(TokenType type, int line, int column)
        : type{type}, line{line}, column{column}
        {
            value = 0;
        }
};

std::ostream& operator<<(std::ostream& os, const Token& token);

class Lexer {
public:
    Lexer(std::string source_code)
        : source_code{std::move(source_code)} {}
    
    std::vector<Token> tokenize();
private:
    std::string source_code;
    int current_position, line, column;
    std::vector<Token> tokens;

    char getNextCharacter();
    char peekNextCharacter();
    void handleError(std::string message, int l, int c);
};