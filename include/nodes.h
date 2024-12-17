#pragma once
#include <variant>
#include <string>
#include <memory>
#include "token.h"
#include <optional>
#include "environment.h"
#include "values.h"


class ASTNode;
using ASTList = std::vector<std::shared_ptr<ASTNode>>;

[[noreturn]] void runtimeError(std::string message, int line, int column);
[[noreturn]] void runtimeError(std::string message);

class ASTNode {
public:
    bool debug = false;
    int line, column;

    ASTNode(int line, int column);
    virtual ~ASTNode() = default;

    virtual std::optional<std::shared_ptr<Value>> evaluate(Environment&) = 0;
};


class AtomNode : public ASTNode {
private:
    std::variant<int, double, bool, std::string> value;

public:
    AtomNode(std::variant<int, double, bool, std::string> value, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    bool isInt();
    bool isFloat();
    bool isBool();
    bool isString();

    int getInt();
    double getFloat();
    bool getBool();
    std::string getString();
};

class UnaryOpNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> right;
    TokenType op;

    UnaryOpNode(TokenType op, std::shared_ptr<ASTNode> right, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
};

class BinaryOpNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> left, right;
    TokenType op;

    BinaryOpNode(std::shared_ptr<ASTNode> left, TokenType op, std::shared_ptr<ASTNode> right, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    std::optional<std::shared_ptr<Value>> performOperation(std::shared_ptr<Value> left_value,
                                                            std::shared_ptr<Value>(right_value));
};

class ParenthesisOpNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> expr;

    ParenthesisOpNode(std::shared_ptr<ASTNode> expr, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
};

class IdentifierNode : public ASTNode {
public:
    std::string name;

    IdentifierNode(std::string name, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
};

class ScopedNode : public ASTNode {
public:
    TokenType keyword;
    const std::shared_ptr<ScopedNode> if_link;
    std::shared_ptr<ASTNode> comparison;
    bool last_comparison_result;
    std::vector<std::shared_ptr<ASTNode>> statements_block;

    ScopedNode(TokenType keyword, std::shared_ptr<ScopedNode> if_link, std::shared_ptr<ASTNode> comparison,
                std::vector<std::shared_ptr<ASTNode>> statements_block, int line, int column);

    ~ScopedNode() noexcept override = default;

    bool getComparisonValue(Environment& env) const;
    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
};

class ForNode : public ASTNode {
public:
    ForNode(TokenType keyword, std::shared_ptr<ASTNode> initialization, std::shared_ptr<std::string> init_string,
            std::shared_ptr<ASTNode> condition_value, std::shared_ptr<ASTNode> increment,
            std::vector<std::shared_ptr<ASTNode>> block, int line, int column);
    
    ~ForNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    
    TokenType keyword;
    std::shared_ptr<ASTNode> initialization;
    std::shared_ptr<std::string> init_string;
    std::shared_ptr<ASTNode> condition_value;
    std::shared_ptr<ASTNode> increment;
    std::vector<std::shared_ptr<ASTNode>> block;
};

class KeywordNode : public ASTNode {
public:
    KeywordNode(TokenType keyword, std::shared_ptr<ASTNode> right, int line, int column)
        : ASTNode{line, column}, keyword{keyword}, right{right} {}
    
    ~KeywordNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    TokenType keyword;
    std::shared_ptr<ASTNode> right;
};

class ListNode : public ASTNode {
public:
    ListNode(ASTList list, int line, int column)
        : ASTNode{line, column}, list{list} {}
    ~ListNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    ASTList list;
};

std::string getValueStr(std::shared_ptr<Value> value);
std::string getValueStr(Value value);
std::string getTypeStr(ValueType type);