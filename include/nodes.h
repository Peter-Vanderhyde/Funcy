#pragma once
#include <variant>
#include <string>
#include <memory>
#include "token.h"
#include <optional>


using Value = std::variant<int, double, bool, std::string>;

[[noreturn]] void runtimeError(std::string message, int line, int column);
[[noreturn]] void runtimeError(std::string message);

class ASTNode {
public:
    bool debug = false;
    int line, column;

    ASTNode(int line, int column);
    virtual ~ASTNode() = default;

    virtual std::optional<std::shared_ptr<Value>> evaluate() = 0;
};


class AtomNode : public ASTNode {
public:
    std::variant<int, double, bool, std::string> value;

    AtomNode(std::variant<int, double, bool, std::string> value, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate() override;

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

    std::optional<std::shared_ptr<Value>> evaluate() override;
};

class BinaryOpNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> left, right;
    TokenType op;

    BinaryOpNode(std::shared_ptr<ASTNode> left, TokenType op, std::shared_ptr<ASTNode> right, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate() override;
};

class ParenthesisOpNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> expr;

    ParenthesisOpNode(std::shared_ptr<ASTNode> expr, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate() override;
};


std::string getValueStr(std::shared_ptr<Value> value);
std::string getValueStr(Value value);