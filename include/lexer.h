#include <variant>
#include <string>
#include <vector>
#include <map>

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
    _Boolean
};

std::map<std::string, TokenType> keyword_tokens{
};

std::map<char, TokenType> op_tokens{
    {'+', TokenType::_Plus},
    {'-', TokenType::_Minus},
    {'*', TokenType::_Multiply},
    {'/', TokenType::_Divide},
    {'=', TokenType::_Equals}
};

class Token{
public:
    TokenType type;
    std::variant<int, float, std::string> value;
    int line, column;

    Token(TokenType type, const std::variant<int, float, std::string>& value, int line, int column)
        : type{type}, value{value}, line{line}, column{column} {}
    
    Token(TokenType type, int line, int column)
        : type{type}, line{line}, column{column}
        {
            value = 0;
        }
};

class Lexer {
public:
    Lexer(std::string source_code)
        : source_code{std::move(source_code)} {}
private:
    std::string source_code;
    int current_position, line, column;
    std::vector<Token> tokens;

    char get_next_character();
    char peek_next_character();
    void tokenize();
};