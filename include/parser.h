#pragma once
#include <vector>
#include <format>
#include <optional>
#include <memory>
#include <math.h>
#include "lexer.h"
#include "environment.h"


template <typename T1, typename T2>
std::optional<std::shared_ptr<Value>> doArithmetic(T1 lhs, T2 rhs, TokenType op);


class ASTNode {
public:
    virtual ~ASTNode() noexcept = default;
    virtual std::optional<std::shared_ptr<Value>> evaluate(Environment& env) const = 0;
};

// Node for numeric literals
class AtomNode : public ASTNode {
public:
    AtomNode(std::variant<int, double, bool, std::string> value) : value{value} {}

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) const override;

    std::variant<int, double, bool, std::string> getValue() const;
    bool isInteger() const;
    bool isFloat() const;
    bool isBool() const;
    bool isString() const;
    int getInteger() const;
    double getFloat() const;
    bool getBool() const;
    std::string getString() const;
    
    const std::variant<int, double, bool, std::string> value;
};

// Node for variables and keywords
class IdentifierNode : public ASTNode {
public:
    IdentifierNode(std::string value) : value{value} {}

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) const override;

    std::string value;
};

// Node for binary operations (e.g., +, -, *, /, //, ^)
class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(std::unique_ptr<ASTNode> left, TokenType op, std::unique_ptr<ASTNode> right)
        : left{std::move(left)}, op{op}, right{std::move(right)} {}
    
    ~BinaryOpNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) const override;

    std::unique_ptr<ASTNode> left;
    TokenType op;  // Operator like _Plus, _Minus
    std::unique_ptr<ASTNode> right;
};

class UnaryOpNode : public ASTNode {
public:
    UnaryOpNode(TokenType op, std::unique_ptr<ASTNode> right)
        : op{op}, right{std::move(right)} {}
    
    ~UnaryOpNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) const override;

    TokenType op;
    std::unique_ptr<ASTNode> right;
};

class ParenthesisOpNode : public ASTNode {
public:
    ParenthesisOpNode(std::unique_ptr<ASTNode> expr)
        : expr{std::move(expr)} {}
    
    ~ParenthesisOpNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) const override;

    std::unique_ptr<ASTNode> expr;
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
    std::optional<const Token*> peek(int ahead) const;
    std::string getTokenStr() const;
    bool tokenIs(std::string str) const;

    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<ASTNode> parseComparison();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<ASTNode> parseTerm();
    std::unique_ptr<ASTNode> parseFactor();
    std::unique_ptr<ASTNode> parsePower();
    std::unique_ptr<ASTNode> parsePrimary();
    std::unique_ptr<ASTNode> parseAtom();
    std::unique_ptr<ASTNode> parseIdentifier();
};