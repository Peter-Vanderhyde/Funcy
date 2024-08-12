#pragma once
#include <vector>
#include <format>
#include <optional>
#include <memory>
#include <math.h>
#include <unordered_map>
#include <algorithm>
#include "lexer.h"

class ASTNode;

using Value = std::variant<int, double, bool, std::string, std::shared_ptr<ASTNode>>;

class Scope {
public:
    Scope() {};

    void set(const std::string variable, std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(const std::string variable) const;
    bool has(const std::string variable) const;
    // Method to display the contents of the environment
    void display() const;
private:
    std::unordered_map<std::string, std::shared_ptr<Value>> variables;
};


class Environment {
public:
    Environment() {};

    void addScope();
    void removeScope();

    void set(const std::string variable, std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(const std::string variable) const;
    bool has(const std::string variable) const;
    // Method to display the contents of the environment
    void display() const;
    int scopeDepth() const;
private:
    std::vector<Scope> scopes;
};

class BreakException : public std::exception {};
class ContinueException : public std::exception {};
class ReturnException : public std::exception {
public:
    ReturnException(std::optional<std::shared_ptr<Value>> value)
        : value{value} {}

    std::optional<std::shared_ptr<Value>> value;
};


template <typename T1, typename T2>
std::optional<std::shared_ptr<Value>> doArithmetic(T1 lhs, T2 rhs, TokenType op);

void printValue(const std::shared_ptr<Value> value);


class ASTNode {
public:
    virtual ~ASTNode() noexcept = default;
    virtual std::optional<std::shared_ptr<Value>> evaluate(Environment& env) = 0;
};

// Node for numeric literals
class AtomNode : public ASTNode {
public:
    AtomNode(std::variant<int, double, bool, std::string> value) : value{value} {}

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

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

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    std::string value;
};

// Node for binary operations (e.g., +, -, *, /, //, ^)
class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(std::shared_ptr<ASTNode> left, TokenType op, std::shared_ptr<ASTNode> right)
        : left{left}, op{op}, right{right} {}
    
    ~BinaryOpNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    std::shared_ptr<ASTNode> left;
    TokenType op;  // Operator like _Plus, _Minus
    std::shared_ptr<ASTNode> right;
};

class UnaryOpNode : public ASTNode {
public:
    UnaryOpNode(TokenType op, std::shared_ptr<ASTNode> right)
        : op{op}, right{right} {}
    
    ~UnaryOpNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    TokenType op;
    std::shared_ptr<ASTNode> right;
};

class ParenthesisOpNode : public ASTNode {
public:
    ParenthesisOpNode(std::shared_ptr<ASTNode> expr)
        : expr{expr} {}
    
    ~ParenthesisOpNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    std::shared_ptr<ASTNode> expr;
};

class ScopeNode : public ASTNode {
public:
    ScopeNode(std::vector<std::shared_ptr<ASTNode>> block)
        : block{block} {}
    
    ~ScopeNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    std::vector<std::shared_ptr<ASTNode>> block;
};

class ScopedNode : public ASTNode {
public:
    ScopedNode(TokenType keyword, std::shared_ptr<ScopedNode> if_link, std::shared_ptr<ASTNode> comparison, std::vector<std::shared_ptr<ASTNode>> statements_block)
        : keyword{keyword}, if_link{if_link}, comparison{comparison}, last_comparison_result{false}, statements_block{statements_block} {}

    ~ScopedNode() noexcept override = default;

    bool getComparisonValue(Environment& env) const;
    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    TokenType keyword;
    const std::shared_ptr<ScopedNode> if_link;
    std::shared_ptr<ASTNode> comparison;
    bool last_comparison_result;
    std::vector<std::shared_ptr<ASTNode>> statements_block;
};

class ForNode : public ASTNode {
public:
    ForNode(TokenType keyword, std::shared_ptr<ASTNode> initialization, std::shared_ptr<std::string> init_string, std::shared_ptr<ASTNode> condition_value, std::shared_ptr<ASTNode> increment, std::vector<std::shared_ptr<ASTNode>> block)
        : keyword{keyword}, initialization{initialization}, init_string{init_string}, condition_value{condition_value}, increment{increment}, block{block} {}
    
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
    KeywordNode(TokenType keyword, std::shared_ptr<ASTNode> right=nullptr)
        : keyword{keyword}, right{right} {}
    
    ~KeywordNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    TokenType keyword;
    std::shared_ptr<ASTNode> right;
};

class FuncNode : public ASTNode {
public:
    FuncNode(std::vector<std::shared_ptr<ASTNode>> args, std::vector<std::shared_ptr<ASTNode>> block)
        : args{args}, block{block} {}
    
    ~FuncNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    void setArgs(std::vector<std::shared_ptr<Value>> values, Environment& base_env, Environment& local_env);
    std::optional<std::shared_ptr<Value>> callFunc(std::vector<std::shared_ptr<Value>> values, Environment& env);
    
    std::vector<std::shared_ptr<ASTNode>> args;
    Environment local_env;
    std::vector<std::shared_ptr<ASTNode>> block;
};

class FuncCallNode : public ASTNode {
public:
    FuncCallNode(std::shared_ptr<ASTNode> identifier, std::vector<std::shared_ptr<ASTNode>> values)
        : identifier{identifier}, values{values} {}
    
    ~FuncCallNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    std::vector<std::shared_ptr<Value>> evaluateArgs(Environment& env);

    std::shared_ptr<ASTNode> identifier;
    std::vector<std::shared_ptr<ASTNode>> values;
    std::shared_ptr<Environment> base_env = nullptr;
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens)
        : tokens{tokens} {}
    
    std::vector<std::shared_ptr<ASTNode>> parse();

    void addIfElseScope();
    void removeIfElseScope();
private:
    const std::vector<Token>& tokens;
    size_t token_index = 0;
    std::vector<std::shared_ptr<ScopedNode>> last_if_else{nullptr};

    const Token* getToken() const;
    const Token* consume();
    std::optional<const Token*> peek(int ahead) const;
    std::string getTokenStr() const;
    bool tokenIs(std::string str) const;
    bool nextTokenIs(std::string str) const;

    std::shared_ptr<ASTNode> parseFoundation();
    std::shared_ptr<ASTNode> parseControlFlowStatement();
    std::shared_ptr<ASTNode> parseStatement(std::shared_ptr<std::string> varString);
    std::shared_ptr<ASTNode> parseLogicalOr();
    std::shared_ptr<ASTNode> parseLogicalAnd();
    std::shared_ptr<ASTNode> parseEquality();
    std::shared_ptr<ASTNode> parseRelation();
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseTerm();
    std::shared_ptr<ASTNode> parseFactor();
    std::shared_ptr<ASTNode> parsePower();
    std::shared_ptr<ASTNode> parseLogicalNot();
    std::shared_ptr<ASTNode> parsePrimary();
    std::shared_ptr<ASTNode> parseAtom();
    std::shared_ptr<ASTNode> parseFuncCall();
    std::shared_ptr<ASTNode> parseIdentifier(std::shared_ptr<std::string> varString);
};


std::string getValueStr(std::shared_ptr<Value> value);