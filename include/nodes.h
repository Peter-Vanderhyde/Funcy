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
using ASTDictionary = std::vector<std::pair<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>>>;

[[noreturn]] void runtimeError(std::string message, int line, int column, std::string filename);
[[noreturn]] void runtimeError(std::string message, std::string filename);

bool check_truthy(const Value& value);

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
    std::variant<int, double, bool, std::string, SpecialIndex> value;

public:
    AtomNode(std::variant<int, double, bool, std::string, SpecialIndex> value, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    bool isInt();
    bool isFloat();
    bool isBool();
    bool isString();
    bool isIndex();

    int getInt();
    double getFloat();
    bool getBool();
    std::string getString();
    SpecialIndex getIndex();
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
                                                            std::shared_ptr<Value>(right_value), TokenType* custom_op = nullptr);
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
    bool member_variable = false;

    IdentifierNode(std::string name, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    std::optional<std::shared_ptr<Value>> evaluate(Environment& env, ValueType member_type);
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

class IndexNode : public ASTNode {
public:
    IndexNode(std::shared_ptr<ASTNode> container, std::shared_ptr<ASTNode> start_index, std::shared_ptr<ASTNode> end_index,
                int line, int column)
        : ASTNode{line, column}, container{container}, start_index{start_index}, end_index{end_index} {}
    ~IndexNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    std::optional<std::shared_ptr<Value>> getIndex(Environment& env,
                                                    std::variant<std::shared_ptr<std::string>,
                                                                std::shared_ptr<List>,
                                                                std::shared_ptr<Dictionary>> distr);
    void assignIndex(Environment& env, std::shared_ptr<Value> value);

    std::shared_ptr<ASTNode> container;
    std::shared_ptr<ASTNode> start_index;
    std::shared_ptr<ASTNode> end_index;
};

class FuncNode : public ASTNode {
public:
    FuncNode(bool member_func, std::shared_ptr<std::string> func_name, std::vector<std::shared_ptr<ASTNode>> args,
            std::map<std::string, std::shared_ptr<ASTNode>> default_arg_values, std::vector<std::shared_ptr<ASTNode>> block,
            int line, int column)
        : ASTNode{line, column}, member_func{member_func}, func_name{func_name}, args{args}, default_arg_nodes{default_arg_values}, block{block} {}
    
    ~FuncNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    void setArgs(std::vector<std::shared_ptr<Value>> values, std::map<std::string, std::shared_ptr<Value>> pairs, Scope& local_scope);
    std::optional<std::shared_ptr<Value>> callFunc(std::vector<std::shared_ptr<Value>> values,
                                                    std::map<std::string, std::shared_ptr<Value>> pairs,
                                                    Environment& global_env, bool member_func = false);
    
    Environment local_env;
    bool member_func;
    std::shared_ptr<std::string> func_name;
    std::vector<std::shared_ptr<ASTNode>> args;
    std::map<std::string, std::shared_ptr<ASTNode>> default_arg_nodes;
    std::map<std::string, std::shared_ptr<Value>> default_arg_values;
    std::vector<std::shared_ptr<ASTNode>> block;
    int recursion = 0;
    bool detect_recursion_limit = local_env.detect_recursion;
};

class MethodCallNode : public ASTNode {
public:
    MethodCallNode(std::shared_ptr<ASTNode> identifier, std::vector<std::shared_ptr<ASTNode>> values, int line, int column)
        : ASTNode{line, column}, identifier{identifier}, values{values} {}
    
    ~MethodCallNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    std::optional<std::shared_ptr<Value>> evaluate(Environment& env, ValueType member_type);
    void evaluateArgs(std::vector<std::shared_ptr<Value>>& args,
                    std::map<std::string, std::shared_ptr<Value>>& pairs, Environment& env);

    std::shared_ptr<ASTNode> identifier;
    std::vector<std::shared_ptr<ASTNode>> values;
    std::shared_ptr<Value> member_value;
    std::shared_ptr<Environment> parent_env = nullptr;
};

class DictionaryNode : public ASTNode {
public:
    DictionaryNode(ASTDictionary dictionary, int line, int column)
        : ASTNode{line, column}, dictionary{dictionary} {}
    ~DictionaryNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    ASTDictionary dictionary;
};

class ClassNode : public ASTNode {
public:
    ClassNode(std::shared_ptr<std::string> name, std::vector<std::shared_ptr<ASTNode>> block, int line, int column)
        : ASTNode{line, column}, name{*name}, block{block} {}

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    std::string name;
    std::vector<std::shared_ptr<ASTNode>> block;
    Scope local_scope;
};