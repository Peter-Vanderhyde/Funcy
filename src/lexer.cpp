#include "lexer.h"
#include <format>



std::ostream& operator<<(std::ostream& os, TokenType type) {
    switch (type) {
        case TokenType::_Identifier: os << "Identifier"; break;
        case TokenType::_Float: os << "Float"; break;
        case TokenType::_Integer: os << "Integer"; break;
        case TokenType::_String: os << "String"; break;
        case TokenType::_Semi: os << "Semi"; break;
        case TokenType::_Plus: os << "Plus"; break;
        case TokenType::_Minus: os << "Minus"; break;
        case TokenType::_Multiply: os << "Multiply"; break;
        case TokenType::_Divide: os << "Divide"; break;
        case TokenType::_Equals: os << "Equals"; break;
        case TokenType::_OpenParen: os << "OpenParen"; break;
        case TokenType::_CloseParen: os << "CloseParen"; break;
        case TokenType::_EndOfFile: os << "EndOfFile"; break;
        case TokenType::_Boolean: os << "Boolean"; break;
        case TokenType::_OpenCurly: os << "OpenCurly"; break;
        case TokenType::_CloseCurly: os << "CloseCurly"; break;
        case TokenType::_Caret: os << "Caret"; break;
        case TokenType::_FloorDiv: os << "FloorDiv"; break;
        case TokenType::_Compare: os << "Compare"; break;
        case TokenType::_PlusEquals: os << "PlusEquals"; break;
        case TokenType::_MinusEquals: os << "MinusEquals"; break;
        case TokenType::_MultiplyEquals: os << "MultiplyEquals"; break;
        case TokenType::_DivideEquals: os << "DivideEquals"; break;
        case TokenType::_DoubleMultiply: os << "DoublelMultiply"; break;
        case TokenType::_GreaterThan: os << "GreaterThan"; break;
        case TokenType::_GreaterEquals: os << "GreaterEqual"; break;
        case TokenType::_LessThan: os << "LessThan"; break;
        case TokenType::_LessEquals: os << "LessEqual"; break;
        case TokenType::_Mod: os << "Mod"; break;
        case TokenType::_NotEqual: os << "NotEqual"; break;
        case TokenType::_If: os << "If"; break;
        case TokenType::_Else: os << "Else"; break;
        case TokenType::_Elif: os << "Elif"; break;
        case TokenType::_While: os << "While"; break;
        case TokenType::_Not: os << "Not"; break;
        case TokenType::_Exclamation: os << "Exclamation"; break;
        case TokenType::_And: os << "And"; break;
        case TokenType::_Or: os << "Or"; break;
        case TokenType::_Break: os << "Break"; break;
        case TokenType::_Continue: os << "Continue"; break;
        case TokenType::_For: os << "For"; break;
        case TokenType::_In: os << "In"; break;
        case TokenType::_Comma: os << "Comma"; break;
        //case TokenType::_: os << ""; break;
        default: os << "Unknown"; break;
    }
    return os;
}

std::map<TokenType, std::string> token_labels {
    {TokenType::_Identifier, "ident"},
    {TokenType::_Float, "float"},
    {TokenType::_Integer, "int"},
    {TokenType::_String, "string"},
    {TokenType::_Semi, ";"},
    {TokenType::_Plus, "+"},
    {TokenType::_Minus, "-"},
    {TokenType::_Multiply, "*"},
    {TokenType::_Divide, "/"},
    {TokenType::_Equals, "="},
    {TokenType::_OpenParen, "("},
    {TokenType::_CloseParen, ")"},
    {TokenType::_EndOfFile, "eof"},
    {TokenType::_Boolean, "bool"},
    {TokenType::_OpenCurly, "{"},
    {TokenType::_CloseCurly, "}"},
    {TokenType::_Caret, "^"},
    {TokenType::_FloorDiv, "//"},
    {TokenType::_PlusEquals, "+="},
    {TokenType::_MinusEquals, "-="},
    {TokenType::_MultiplyEquals, "*="},
    {TokenType::_DivideEquals, "/="},
    {TokenType::_DoubleMultiply, "**"},
    {TokenType::_GreaterThan, ">"},
    {TokenType::_GreaterEquals, ">="},
    {TokenType::_LessThan, "<"},
    {TokenType::_LessEquals, "<="},
    {TokenType::_Mod, "%"},
    {TokenType::_Compare, "=="},
    {TokenType::_NotEqual, "!="},
    {TokenType::_If, "if"},
    {TokenType::_Else, "else"},
    {TokenType::_Elif, "elif"},
    {TokenType::_While, "while"},
    {TokenType::_Not, "not"},
    {TokenType::_Exclamation, "!"},
    {TokenType::_And, "and"},
    {TokenType::_Or, "or"},
    {TokenType::_Break, "break"},
    {TokenType::_Continue, "continue"},
    {TokenType::_For, "for"},
    {TokenType::_In, "in"},
    {TokenType::_Comma, ","}
    //{TokenType::_, ""}
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
    {'}', TokenType::_CloseCurly},
    {'^', TokenType::_Caret},
    {'>', TokenType::_GreaterThan},
    {'<', TokenType::_LessThan},
    {'%', TokenType::_Mod},
    {'!', TokenType::_Exclamation},
    {',', TokenType::_Comma}
};

std::map<std::string, TokenType> scoped_keyword_tokens {
    {"if", TokenType::_If},
    {"else", TokenType::_Else},
    {"elif", TokenType::_Elif},
    {"while", TokenType::_While},
    {"for", TokenType::_For}
};

std::map<std::string, TokenType> keyword_tokens {
    {"if", TokenType::_If},
    {"else", TokenType::_Else},
    {"elif", TokenType::_Elif},
    {"while", TokenType::_While},
    {"not", TokenType::_Not},
    {"and", TokenType::_And},
    {"or", TokenType::_Or},
    {"break", TokenType::_Break},
    {"continue", TokenType::_Continue},
    {"for", TokenType::_For},
    {"in", TokenType::_In}
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
        // Always print integers and doubles
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
    throw std::runtime_error(std::format("{} at line {} column {}", message, l, c));
}

std::vector<Token> Lexer::tokenize() {
    line = 1;
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
                    handleError("Expected '\"'", l, c);
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
                        handleError("Float has too many decimals ", line, column);
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
            // Handle operators/single character tokens/double character ops
            if (character == '/' && peekNextCharacter() == '/') {
                Token token{TokenType::_FloorDiv, line, column};
                getNextCharacter();
                tokens.push_back(token);
            }
            else if (character == '*' && peekNextCharacter() == '*') {
                Token token{TokenType::_DoubleMultiply, line, column};
                getNextCharacter();
                tokens.push_back(token);
            }
            else if (peekNextCharacter() == '=') {
                TokenType op;
                switch (character) {
                    case '=' :
                        op = TokenType::_Compare;
                        break;
                    case '+' :
                        op = TokenType::_PlusEquals;
                        break;
                    case '-' :
                        op = TokenType::_MinusEquals;
                        break;
                    case '*' :
                        op = TokenType::_MultiplyEquals;
                        break;
                    case '/' :
                        op = TokenType::_DivideEquals;
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
                        handleError("Unrecognized operator", line, column);
                }

                Token token{op, line, column};
                getNextCharacter();
                tokens.push_back(token);
            }
            else {
                Token token{char_tokens[character], line, column};
                tokens.push_back(token);
            }
        }
        else {
            handleError(std::format("Unrecognized character: {}", character), line, column);
        }

    }
    Token token{TokenType::_EndOfFile, line, column};
    tokens.push_back(token);
    return tokens;
}