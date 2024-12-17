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
    _PlusEquals,
    _Minus,
    _MinusEquals,
    _Multiply,
    _DoubleMultiply,
    _MultiplyEquals,
    _Divide,
    _DoubleDivide,
    _DivideEquals,
    _EOF,
    _Semi,
    _ParenOpen,
    _ParenClose,
    _Caret,
    _Dot,
    _Equals,
    _Compare,
    _And,
    _Or,
    _Not,
    _Exclamation,
    _If,
    _Elif,
    _Else,
    _CurlyOpen,
    _CurlyClose,
    _While,
    _For,
    _GreaterThan,
    _GreaterEquals,
    _LessThan,
    _LessEquals,
    _NotEqual,
    _Break,
    _Continue,
    _Comma,
    _Func,
    _List,
    _SquareOpen,
    _SquareClose,
    _Colon,
    _Return
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