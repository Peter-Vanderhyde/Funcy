#pragma once
#include <variant>
#include <string>
#include <memory>
#include "token.h"
#include <optional>
#include "environment.h"
#include "values.h"


extern bool DEBUG_AST;

class ASTNode;
using ASTList = std::vector<std::shared_ptr<ASTNode>>;
using ASTDictionary = std::vector<std::pair<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>>>;

bool checkTruthy(const Value& value);

class ASTNode {
public:
    bool debug;
    int line, column;

    ASTNode(int line, int column);
    virtual ~ASTNode() = default;

    virtual std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>&) = 0;
    virtual void debugPrint(ValueList values) = 0;
    virtual std::string getPrintable() = 0;
};


class AtomNode : public ASTNode {
private:
    std::variant<int, double, bool, std::string, SpecialIndex> value;

public:
    AtomNode(std::variant<int, double, bool, std::string, SpecialIndex> value, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;

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

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;
};

class BinaryOpNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> left, right;
    TokenType op;

    BinaryOpNode(std::shared_ptr<ASTNode> left, TokenType op, std::shared_ptr<ASTNode> right, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;

    std::optional<std::shared_ptr<Value>> performOperation(std::shared_ptr<Value> left_value,
                                                            std::shared_ptr<Value>(right_value), TokenType* custom_op = nullptr);
};

class ParenthesisOpNode : public ASTNode {
public:
    std::shared_ptr<ASTNode> expr;

    ParenthesisOpNode(std::shared_ptr<ASTNode> expr, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;
};

class IdentifierNode : public ASTNode {
public:
    std::string name;
    bool member_variable = false;

    IdentifierNode(std::string name, int line, int column);

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope, ValueType member_type);
    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope, const std::string library_name);
    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope, const std::shared_ptr<Instance>& instance);
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;
};

class ScopedNode : public ASTNode {
public:
    TokenType keyword;
    const std::shared_ptr<ScopedNode> if_link;
    std::shared_ptr<ASTNode> comparison;
    std::vector<bool> last_comparison_results;
    std::vector<std::shared_ptr<ASTNode>> statements_block;

    ScopedNode(TokenType keyword, std::shared_ptr<ScopedNode> if_link, std::shared_ptr<ASTNode> comparison,
                std::vector<std::shared_ptr<ASTNode>> statements_block, int line, int column);

    ~ScopedNode() noexcept override = default;

    bool getComparisonValue(const std::shared_ptr<Scope>& scope) const;
    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;
};

class ForNode : public ASTNode {
public:
    ForNode(TokenType keyword, std::shared_ptr<ASTNode> initialization,
            std::shared_ptr<ASTNode> condition_value, std::shared_ptr<ASTNode> increment,
            std::vector<std::shared_ptr<ASTNode>> block, int line, int column);
    
    ~ForNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;
    
    TokenType keyword;
    std::shared_ptr<ASTNode> initialization;
    std::shared_ptr<ASTNode> condition_value;
    std::shared_ptr<ASTNode> increment;
    std::vector<std::shared_ptr<ASTNode>> block;
};

class KeywordNode : public ASTNode {
public:
    KeywordNode(TokenType keyword, std::shared_ptr<ASTNode> right, int line, int column)
        : ASTNode{line, column}, keyword{keyword}, right{right} {}
    
    ~KeywordNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;

    TokenType keyword;
    std::shared_ptr<ASTNode> right;
};

class ListNode : public ASTNode {
public:
    ListNode(ASTList list, int line, int column)
        : ASTNode{line, column}, list{list} {}
    ~ListNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;

    ASTList list;
};

class IndexNode : public ASTNode {
public:
    IndexNode(std::shared_ptr<ASTNode> container, std::shared_ptr<ASTNode> start_index, std::shared_ptr<ASTNode> end_index,
                int line, int column)
        : ASTNode{line, column}, container{container}, start_index{start_index}, end_index{end_index} {}
    ~IndexNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;
    std::optional<std::shared_ptr<Value>> getIndex(const std::shared_ptr<Scope>& scope,
                                                    std::variant<std::shared_ptr<std::string>,
                                                                std::shared_ptr<List>,
                                                                std::shared_ptr<Dictionary>> distr);
    void assignIndex(const std::shared_ptr<Scope>& scope, std::shared_ptr<Value> value);

    std::shared_ptr<ASTNode> container;
    std::shared_ptr<ASTNode> start_index;
    std::shared_ptr<ASTNode> end_index;
};

class FuncNode : public ASTNode {
public:
    FuncNode(std::shared_ptr<std::string> func_name, std::vector<std::shared_ptr<ASTNode>> args,
            std::map<std::string, std::shared_ptr<ASTNode>> default_arg_values, std::vector<std::shared_ptr<ASTNode>> block,
            int line, int column, std::string file_context)
        : ASTNode{line, column}, func_name{func_name}, args{args}, default_arg_nodes{default_arg_values}, block{block}, file_context{file_context} {}
    
    ~FuncNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;
    void setArgs(ValueList values, std::map<std::string, std::shared_ptr<Value>> pairs, const std::shared_ptr<Scope>& local_scope);
    std::optional<std::shared_ptr<Value>> callFunc(ValueList values,
        std::map<std::string, std::shared_ptr<Value>> pairs,
        std::shared_ptr<Instance> owner_instance = nullptr
    );
    
    std::shared_ptr<Scope> call_scope; // Saves the scope at definition so it can generate scopes from it at method call
    std::shared_ptr<std::string> func_name;
    std::vector<std::shared_ptr<ASTNode>> args;
    std::map<std::string, std::shared_ptr<ASTNode>> default_arg_nodes;
    std::map<std::string, std::shared_ptr<Value>> default_arg_values;
    std::vector<std::shared_ptr<ASTNode>> block;
    std::string file_context;
    int recursion = 0;
    bool detect_recursion_limit = DETECT_RECURSION;
};

class MethodCallNode : public ASTNode {
public:
    MethodCallNode(std::shared_ptr<ASTNode> stored_func, std::vector<std::shared_ptr<ASTNode>> values, int line, int column)
        : ASTNode{line, column}, stored_func{stored_func}, values{values} {}
    
    ~MethodCallNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;
    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope, ValueType member_type);
    void evaluateArgs(ValueList& args,
                    std::map<std::string, std::shared_ptr<Value>>& pairs, const std::shared_ptr<Scope>& scope);

    std::shared_ptr<ASTNode> stored_func;
    std::vector<std::shared_ptr<ASTNode>> values;
    std::shared_ptr<Value> member_value;
};

class DictionaryNode : public ASTNode {
public:
    DictionaryNode(ASTDictionary dictionary, int line, int column)
        : ASTNode{line, column}, dictionary{dictionary} {}
    ~DictionaryNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;

    ASTDictionary dictionary;
};

class ClassNode : public ASTNode {
public:
    ClassNode(std::shared_ptr<std::string> name, std::vector<std::shared_ptr<ASTNode>> block, int line, int column, std::string file_context)
        : ASTNode{line, column}, name{*name}, block{block}, file_context{file_context} {}

    std::optional<std::shared_ptr<Value>> evaluate(const std::shared_ptr<Scope>& scope) override;
    void debugPrint(ValueList values) override;
    std::string getPrintable() override;

    std::string name;
    std::vector<std::shared_ptr<ASTNode>> block;
    std::shared_ptr<Scope> local_scope;
    std::string file_context;
};