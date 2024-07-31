#include "lexer.h"
#include <format>



std::map<TokenType, std::string> token_labels {
    {TokenType::_Identifier, "Ident"},
    {TokenType::_Float, "Float"},
    {TokenType::_Integer, "Int"},
    {TokenType::_String, "Str"},
    {TokenType::_Semi, ";"},
    {TokenType::_Plus, "+"},
    {TokenType::_Minus, "-"},
    {TokenType::_Multiply, "*"},
    {TokenType::_Divide, "/"},
    {TokenType::_Equals, "="},
    {TokenType::_OpenParen, "("},
    {TokenType::_CloseParen, ")"},
    {TokenType::_EndOfFile, "EOF"},
    {TokenType::_Boolean, "Bool"},
    {TokenType::_OpenCurly, "{"},
    {TokenType::_CloseCurly, "}"}
};

std::map<std::string, TokenType> keyword_tokens{
};

std::map<char, TokenType> char_tokens{
    {'+', TokenType::_Plus},
    {'-', TokenType::_Minus},
    {'*', TokenType::_Multiply},
    {'/', TokenType::_Divide},
    {'=', TokenType::_Equals},
    {'(', TokenType::_OpenParen},
    {')', TokenType::_CloseParen},
    {';', TokenType::_Semi},
    {'{', TokenType::_OpenCurly},
    {'}', TokenType::_CloseCurly}
};


std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << "Token(";

    os << token_labels[token.type];
    if (token.type != TokenType::_Integer && token.type != TokenType::_Float) {
        auto check_val = std::get_if<int>(&token.value);
        if (!(check_val)) {
            // Print value
            os << ", ";
            if (token.type == TokenType::_Boolean) {
                // Print true or false, not 0 or 1
                std::visit([&os](const auto& val) { os << std::boolalpha << val; }, token.value);
            }
            else {
                std::visit([&os](const auto& val) { os << val; }, token.value);
            }
        }
    }
    else {
        // Always print integers and floats
        os << ", ";
        std::visit([&os](const auto& val) { os << val; }, token.value);
    }

    os << ")";
    
    return os;
}

char Lexer::getNextCharacter() {
    char next = source_code[current_position];
    current_position += 1;
    column += 1;
    if (next == '\n') {
        column = 0;
        line += 1;
    }
    return next;
}

char Lexer::peekNextCharacter() {
    return source_code[current_position];
}

void Lexer::handleError(std::string message, int l, int c) {
    throw std::runtime_error(std::format("Error! {} -> line:{} column:{}", message, l + 1, c));
}

std::vector<Token> Lexer::tokenize() {
    line = 0;
    column = 0;
    while (current_position < source_code.length()) {
        char character = getNextCharacter();

        if (iswspace(character)) {
            // Handle whitespace
            continue;
        }

        else if (character == '#') {
            // Grab characters until it's reached the next line
            while (getNextCharacter() != '\n') {
                continue;
            }
        }

        else if (isalpha(character) || character == '_') {
            // Handle identifiers
            int l = line;
            int c = column;
            std::string identifier = "";
            identifier += character;

            char next = peekNextCharacter();
            while (isalnum(next) || next == '_') {
                identifier += getNextCharacter();
                next = peekNextCharacter();
            }

            // Check if it's a keyword
            if (keyword_tokens.contains(identifier)) {
                Token token{keyword_tokens[identifier], l, c};
                tokens.push_back(token);
            }
            else if (identifier == "true" || identifier == "false") {
                bool val{identifier == "true"};
                Token token{TokenType::_Boolean, val, l, c};
                tokens.push_back(token);
            }
            else {
                Token token{TokenType::_Identifier, identifier, l, c};
                tokens.push_back(token);
            }
        }
        else if (isdigit(character) || character == '"') {
            // Handle literals
            std::string literal = "";
            literal += character;
            int l = line;
            int c = column;

            char next = peekNextCharacter();
            if (character == '"') {
                // It's a string
                literal = "";
                while (current_position <= source_code.length() && next != '"') {
                    literal += getNextCharacter();
                    next = peekNextCharacter();
                }

                if (current_position >= source_code.length()) {
                    handleError("String missing closing quotes.", l, c);
                }

                getNextCharacter();
                Token token{TokenType::_String, literal, l, c};
                tokens.push_back(token);
            }
            else {
                // It's an int or float
                bool has_decimal = false;
                while (isdigit(next) || next == '.') {
                    if (next == '.' && !has_decimal) {
                        has_decimal = true;
                    }
                    else if (next == '.') {
                        // Two decimals in a float
                        handleError("Float has too many decimals.", line, column);
                    }

                    literal += getNextCharacter();
                    next = peekNextCharacter();
                }

                if (literal.find('.') != std::string::npos) {
                    // It's a float
                    Token token{TokenType::_Float, std::stof(literal), l, c};
                    tokens.push_back(token);
                }
                else {
                    // It's an integer
                    Token token{TokenType::_Integer, std::stoi(literal), l, c};
                    tokens.push_back(token);
                }
            }
        }
        else if (char_tokens.contains(character)) {
            // Handle operators/single character tokens
            Token token{char_tokens[character], line, column};
            tokens.push_back(token);
        }
        else {
            handleError(std::format("Unrecognized character: {}", character), line, column);
        }

    }
    Token token{TokenType::_EndOfFile, line, column};
    tokens.push_back(token);
    return tokens;
}