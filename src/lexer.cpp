#include "lexer.h"
#include <iostream>
#include <format>
#include <unordered_map>


std::unordered_map<TokenType, std::string> token_labels{
    {TokenType::_Integer, "T:Integer"},
    {TokenType::_Float, "T:Float"},
    {TokenType::_Plus, "T:Plus"},
    {TokenType::_Minus, "T:Minus"},
    {TokenType::_Multiply, "T:Multiply"},
    {TokenType::_DoubleMultiply, "T:DoubleMultiply"},
    {TokenType::_Divide, "T:Divide"},
    {TokenType::_DoubleDivide, "T:DoubleDivide"},
    {TokenType::_OpenParen, "T:OpenParenthesis"},
    {TokenType::_CloseParen, "T:CloseParenthesis"},
    {TokenType::_Semi, "T:Semicolon"},
    {TokenType::_EOF, "T:EndOfFile"}
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
    {'/', TokenType::_Divide}
};


Token::Token(TokenType type, TokenValue value, int line, int column)
    : type{type}, value{value}, line{line}, column{column} {}

Token::Token(TokenType type, int line, int column)
    : type{type}, value{0}, line{line}, column{column} {}


Lexer::Lexer(const std::string& source_code)
    : source_code{source_code} {}

char Lexer::grabNextCharacter() {
    char character = source_code[current_position];
    current_position += 1;
    column += 1;
    if (character == '\n') {
        column = 0;
        line += 1;
    }
    return character;
}

char Lexer::peekNextCharacter(int ahead) {
    return source_code[current_position + ahead];
}

void Lexer::lexerError(std::string message, int line, int column) {
    throw std::runtime_error(std::format("\033[31mLexer Error:\033[38;5;214m {}\033[0m at \033[4m\033[38;5;129mline {} column {}\033[0m", message, line, column));
}

std::vector<Token> Lexer::tokenize() {
    current_position = 0;
    line = 1;
    column = 0;

    std::vector<Token> tokens;

    while (current_position < source_code.length()) {
        char character = grabNextCharacter();

        if (iswspace(character)) { // Ignore whitespace
            continue;
        }
        else if (character == '#') { // Ignore line after comment sign
            while (grabNextCharacter() != '\n') {
                continue;
            }
        }
        else if (character == '/' && peekNextCharacter() == '*') { // Block comments
            grabNextCharacter();
            while (grabNextCharacter() != '*' || peekNextCharacter() != '/') {
                continue;
            }
            grabNextCharacter();
        }

        else if (isdigit(character)) { // Integer or float
            int l = line;
            int c = column;
            bool found_decimal = false;
            std::string literal;
            literal += character;
            while (current_position < source_code.length()) {
                character = peekNextCharacter();
                if (isdigit(character) || character == '.') {
                    if (character == '.') {
                        if (found_decimal) { // TODO Check if second found decimal is for built in float member functions
                            lexerError("Second decimal point found", line, column + 1);
                        }
                        else {
                            found_decimal = true;
                        }
                    }
                    grabNextCharacter();
                    literal += character;
                }
                else {
                    break;
                }
            }
            if (found_decimal) { // It's a float
                tokens.push_back(Token(TokenType::_Float, std::stod(literal), l, c));
            } else { // It's an integer
                tokens.push_back(Token(TokenType::_Integer, std::stoi(literal), l, c));
            }
        }
        else if (char_tokens.find(character) != char_tokens.end()) {
            if (character == '*' && peekNextCharacter() == '*') {
                tokens.push_back(Token(TokenType::_DoubleMultiply, "**", line, column));
                grabNextCharacter();
            }
            else if (character == '/' && peekNextCharacter() == '/') {
                tokens.push_back(Token(TokenType::_DoubleDivide, "//", line, column));
                grabNextCharacter();
            }
            else {
                Token token{char_tokens[character], line, column};
                tokens.push_back(token);
            }
        }
        else {
            lexerError(std::format("Unrecognized character: {}", character), line, column);
        }
    }

    return tokens;
}