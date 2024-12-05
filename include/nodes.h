#pragma once
#include <variant>
#include <string>
#include <memory>
#include "token.h"
#include <optional>
#include "environment.h"


enum class ValueType {
    Integer,
    Float,
    Boolean,
    String,
    Null
};

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
public:
    std::variant<int, double, bool, std::string> value;

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

std::string getValueStr(std::shared_ptr<Value> value);
std::string getValueStr(Value value);