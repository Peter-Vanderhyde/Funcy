#pragma once
#include <vector>
#include <format>
#include <optional>
#include <memory>
#include <math.h>
#include "lexer.h"




class ASTNode {
public:
    virtual ~ASTNode() noexcept = default;
    virtual double evaluate() const = 0;
};

// Node for numeric literals
class NumberNode : public ASTNode {
public:
    NumberNode(double value) : value(value) {}
    NumberNode(int value) : value(value) {}

    double evaluate() const override;

    std::variant<int, double> getValue() const;
    bool isInteger() const;
    bool isFloat() const;
    int getInteger() const;
    double getFloat() const;
    
private:
    std::variant<int, double> value;
};

// Node for binary operations (e.g., +, -, *, /, ^)
class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(std::unique_ptr<ASTNode> left, char op, std::unique_ptr<ASTNode> right)
        : left{std::move(left)}, op{op}, right{std::move(right)} {}
    
    ~BinaryOpNode() noexcept override = default;

    double evaluate() const override;

    std::unique_ptr<ASTNode> left;
    char op;  // Operator like +, -, *, /
    std::unique_ptr<ASTNode> right;
};

class UnaryOpNode : public ASTNode {
public:
    UnaryOpNode(char op, std::unique_ptr<ASTNode> right)
        : op{op}, right{std::move(right)} {}
    
    ~UnaryOpNode() noexcept override = default;

    double evaluate() const override;

    char op;
    std::unique_ptr<ASTNode> right;
};

class ParenthesisOpNode : public ASTNode {
public:
    ParenthesisOpNode(char open, std::unique_ptr<ASTNode> expr, char close)
        : open{open}, expr{std::move(expr)}, close{close} {}
    
    ~ParenthesisOpNode() noexcept override = default;

    double evaluate() const override;

    char open;
    std::unique_ptr<ASTNode> expr;
    char close;
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens)
        : tokens{tokens} {}
    
    std::vector<std::unique_ptr<ASTNode>> parse();
private:
    const std::vector<Token>& tokens;
    size_t token_index = 0;

    const Token* getToken() const;
    const Token* consume();
    std::optional<const Token*> peek(int ahead=1) const;
    std::string getTokenStr() const;
    bool tokenIs(std::string str) const;

    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parsePower();
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseNumber();
};