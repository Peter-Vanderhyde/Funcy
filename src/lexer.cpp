#include "lexer.h"
#include <iostream>
#include <format>
#include "token.h"
#include "library.h"


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

void Lexer::lexerError(std::string message, int line, int column) const {
    handleError(message, line, column, "Syntax Error");
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
        else if (isalpha(character) || character == '_') {
            int l = line;
            int c = column;
            std::string literal;
            literal += character;

            while (current_position < source_code.length()) {
                char next = peekNextCharacter();

                if (isalnum(next) || next == '_') {
                    literal += grabNextCharacter();
                } else {
                    break;
                }
            }

            if (literal == "true") {
                tokens.push_back(Token{TokenType::_Boolean, true, l, c});
            } else if (literal == "false") {
                tokens.push_back(Token{TokenType::_Boolean, false, l, c});
            } else {
                tokens.push_back(Token{TokenType::_Identifier, literal, l, c});
            }
        }

        else if (isdigit(character)) { // Integer or float
            int l = line;
            int c = column;
            bool found_decimal = false;
            std::string literal;
            literal += character;

            while (current_position < source_code.length()) {
                char next = peekNextCharacter();
                
                if (isdigit(next)) {
                    literal += grabNextCharacter();
                }
                else if (next == '.' && !found_decimal) { // First decimal point
                    found_decimal = true;
                    literal += grabNextCharacter();
                }
                else if (next == '.' && found_decimal) { // Second decimal point
                    lexerError("Invalid number format: multiple decimal points", line, column + 1);
                    break;
                }
                else {
                    break;
                }
            }

            // Push token based on whether a decimal point was found
            if (found_decimal) {
                tokens.push_back(Token(TokenType::_Float, std::stod(literal), l, c));
            } else {
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

    tokens.push_back(Token{TokenType::_EOF, line, column});
    return tokens;
}