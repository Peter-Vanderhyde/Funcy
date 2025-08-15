#include "token.h"
#include <unordered_map>
#include <iostream>


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
    {TokenType::_ParenOpen, "("},
    {TokenType::_ParenClose, ")"},
    {TokenType::_Semi, ";"},
    {TokenType::_Caret, "^"},
    {TokenType::_Dot, "."},
    {TokenType::_Equals, "="},
    {TokenType::_EOF, "EndOfFile"},
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
    {TokenType::_While, "while"},
    {TokenType::_For, "for"},
    {TokenType::_LessThan, "<"},
    {TokenType::_LessEquals, "<="},
    {TokenType::_GreaterThan, ">"},
    {TokenType::_GreaterEquals, ">="},
    {TokenType::_NotEqual, "!="},
    {TokenType::_PlusEquals, "+="},
    {TokenType::_MinusEquals, "-="},
    {TokenType::_MultiplyEquals, "*="},
    {TokenType::_DivideEquals, "/="},
    {TokenType::_Break, "break"},
    {TokenType::_Continue, "continue"},
    {TokenType::_Comma, ","},
    {TokenType::_Func, "func"},
    {TokenType::_SquareOpen, "["},
    {TokenType::_SquareClose, "]"},
    {TokenType::_Colon, ":"},
    {TokenType::_Return, "return"},
    {TokenType::_Dot, "."},
    {TokenType::_NullType, "type:null"},
    {TokenType::_IntType, "type:integer"},
    {TokenType::_FloatType, "type:float"},
    {TokenType::_BoolType, "type:boolean"},
    {TokenType::_StrType, "type:string"},
    {TokenType::_ListType, "type:list"},
    {TokenType::_DictType, "type:dictionary"},
    {TokenType::_FuncType, "type:function"},
    {TokenType::_ClassType, "type:class"},
    {TokenType::_InstanceType, "type:instance"},
    {TokenType::_BuiltInType, "type:builtin function"},
    {TokenType::_Mod, "%"},
    {TokenType::_In, "in"},
    {TokenType::_Import, "import"},
    {TokenType::_Global, "global"},
    {TokenType::_Class, "class"},
    {TokenType::_Instance, "instance"},
    {TokenType::_Ampersand, "&"},
    {TokenType::_This, "this"},
    {TokenType::_Throw, "throw"}
    //{TokenType::, ""}
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
    {'(', TokenType::_ParenOpen},
    {')', TokenType::_ParenClose},
    {'=', TokenType::_Equals},
    {'!', TokenType::_Exclamation},
    {'{', TokenType::_CurlyOpen},
    {'}', TokenType::_CurlyClose},
    {'<', TokenType::_LessThan},
    {'>', TokenType::_GreaterThan},
    {',', TokenType::_Comma},
    {'[', TokenType::_SquareOpen},
    {']', TokenType::_SquareClose},
    {':', TokenType::_Colon},
    {'.', TokenType::_Dot},
    {'%', TokenType::_Mod},
    {'&', TokenType::_Ampersand}
};

std::unordered_map<std::string, TokenType> keyword_tokens {
    {"and", TokenType::_And},
    {"or", TokenType::_Or},
    {"not", TokenType::_Not},
    {"if", TokenType::_If},
    {"elif", TokenType::_Elif},
    {"else", TokenType::_Else},
    {"while", TokenType::_While},
    {"for", TokenType::_For},
    {"break", TokenType::_Break},
    {"continue", TokenType::_Continue},
    {"func", TokenType::_Func},
    {"return", TokenType::_Return},
    {"Integer", TokenType::_IntType},
    {"Float", TokenType::_FloatType},
    {"String", TokenType::_StrType},
    {"Boolean", TokenType::_BoolType},
    {"List", TokenType::_ListType},
    {"Dictionary", TokenType::_DictType},
    {"Function", TokenType::_FuncType},
    {"BuiltInFunction", TokenType::_BuiltInType},
    {"Class", TokenType::_ClassType},
    {"Instance", TokenType::_InstanceType},
    {"Null", TokenType::_NullType},
    {"in", TokenType::_In},
    {"import", TokenType::_Import},
    {"global", TokenType::_Global},
    {"class", TokenType::_Class},
    {"instance", TokenType::_Instance},
    {"this", TokenType::_This},
    {"throw", TokenType::_Throw}
};

std::unordered_map<std::string, TokenType> scoped_keyword_tokens {
    {"if", TokenType::_If},
    {"elif", TokenType::_Elif},
    {"else", TokenType::_Else},
    {"while", TokenType::_While},
    {"for", TokenType::_For},
    {"func", TokenType::_Func},
    {"class", TokenType::_Class}
    // Also change in parser.cpp
};


Token::Token(TokenType type, TokenValue value, int line, int column)
    : type{type}, value{value}, line{line}, column{column} {}

Token::Token(TokenType type, int line, int column)
    : type{type}, value{0}, line{line}, column{column} {}

const void Token::display() {
    std::cout << getTokenTypeLabel(type);
    if (type == TokenType::_Integer || !std::holds_alternative<int>(value)) {
        std::cout << " : ";
        if (std::holds_alternative<std::string>(value)) {
            std::cout << std::get<std::string>(value) << std::endl;
        }
        else if (std::holds_alternative<bool>(value)) {
            std::cout << std::boolalpha << std::get<bool>(value) << std::endl;
        }
        else if (std::holds_alternative<int>(value)) {
            std::cout << std::to_string(std::get<int>(value)) << std::endl;
        }
        else if (std::holds_alternative<double>(value)) {
            std::cout << std::to_string(std::get<double>(value)) << std::endl;
        } else {
            throw std::runtime_error("Token Error: Unable to display token of type " + getTokenTypeLabel(type) + ".");
        }
    } else {
        std::cout << std::endl;
    }
}