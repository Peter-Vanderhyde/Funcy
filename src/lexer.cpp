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

            // Check if it's a keyword
            if (keyword_tokens.contains(literal)) {
                Token token{keyword_tokens[literal], l, c};
                tokens.push_back(token);
            }
            else if (literal == "true") {
                tokens.push_back(Token{TokenType::_Boolean, true, l, c});
            } else if (literal == "false") {
                tokens.push_back(Token{TokenType::_Boolean, false, l, c});
            } else {
                tokens.push_back(Token{TokenType::_Identifier, literal, l, c});
            }
        }

        else if (character == '"' || character == '\'') {
            // It's a string

            std::string literal = "";
            literal += character;
            int l = line;
            int c = column;

            char starting_char = character;
            literal = "";
            char next = peekNextCharacter();
            while (current_position < source_code.length()) {
                if (next == '\\') {
                    grabNextCharacter(); // Consume the backslash
                    if (current_position >= source_code.length()) break; // Avoid out of bounds
                    char escape_char = grabNextCharacter(); // Get the escaped character

                    // Handle escape sequences
                    switch (escape_char) {
                        case 'n': literal += '\n'; break;    // Newline
                        case 't': literal += '\t'; break;    // Tab
                        case 'r': literal += '\r'; break;    // Carriage return
                        case 'b': literal += '\b'; break;    // Backspace
                        case 'f': literal += '\f'; break;    // Formfeed
                        case '\\': literal += '\\'; break;   // Backslash
                        case '\"': literal += '\"'; break;   // Double quote
                        case '\'': literal += '\''; break;   // Single quote
                        case '0': literal += '\0'; break;    // Null character
                        default:
                            // Invalid escape sequence
                            lexerError("Unknown escape sequence \\" + std::string(1, escape_char), l, c);
                            break;
                    }
                } else if (next == starting_char) {
                    break;
                } else {
                    literal += grabNextCharacter();
                }
                next = peekNextCharacter();
            }

            if (current_position >= source_code.length()) {
                lexerError("Expected closing quotes", l, c);
            }

            grabNextCharacter();
            Token token{TokenType::_String, literal, l, c};
            tokens.push_back(token);
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
                tokens.push_back(Token(TokenType::_DoubleMultiply, line, column));
                grabNextCharacter();
            }
            else if (character == '/' && peekNextCharacter() == '/') {
                tokens.push_back(Token(TokenType::_DoubleDivide, line, column));
                grabNextCharacter();
            }
            else if (peekNextCharacter() == '=') {
                TokenType op;
                switch (character) {
                    case '=' :
                        op = TokenType::_Compare;
                        break;
                    case '>' :
                        op = TokenType::_GreaterEquals;
                        break;
                    case '<' :
                        op = TokenType::_LessEquals;
                        break;
                    case '!' :
                        op = TokenType::_NotEqual;
                        break;
                    default:
                        handleError("Unrecognized operator", line, column, "Syntax Error");
                }

                Token token{op, line, column};
                grabNextCharacter();
                tokens.push_back(token);
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