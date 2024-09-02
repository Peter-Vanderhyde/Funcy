#pragma once
#include <vector>
#include <optional>
#include <memory>
#include <math.h>
#include <functional>
#include <map>
#include "lexer.h"

class ASTNode;
class Environment;

struct Value;

// ValueCompare struct declaration
struct ValueCompare {
    bool operator()(const std::shared_ptr<Value>& lhs, const std::shared_ptr<Value>& rhs) const;
};

enum class ValueType {
    Integer,
    Float,
    Boolean,
    String,
    List,
    Dictionary,
    Function,
    BuiltInFunction,
    Null,
    Type
};

extern std::unordered_map<TokenType, ValueType> token_value_map;

using List = std::vector<std::shared_ptr<Value>>;
using Dictionary = std::map<std::shared_ptr<Value>, std::shared_ptr<Value>, ValueCompare>;
using BuiltInFunction = std::function<std::optional<std::shared_ptr<Value>>(
    const std::vector<std::shared_ptr<Value>>& args, 
    Environment& env
)>;
using VariantType = std::variant<int, double, bool, TokenType, std::string, std::shared_ptr<ASTNode>,
                        std::shared_ptr<List>, std::shared_ptr<Dictionary>, std::shared_ptr<BuiltInFunction>, ValueType>;

using ASTList = std::vector<std::shared_ptr<ASTNode>>;
using ASTDictionary = std::vector<std::pair<std::shared_ptr<ASTNode>, std::shared_ptr<ASTNode>>>;

std::ostream& operator<<(std::ostream& os, const List& list);

struct Value : public VariantType {
    using VariantType::VariantType;
};

// Custom hash and equality functions for using Value in dictionaries
namespace std {
    template <>
    struct hash<std::shared_ptr<Value>> {
        std::size_t operator()(const std::shared_ptr<Value>& valuePtr) const {
            if (!valuePtr) return 0;

            // Access the underlying VariantType directly
            const VariantType& variant = static_cast<const VariantType&>(*valuePtr);
            return std::visit([](auto&& value) -> std::size_t {
                using T = std::decay_t<decltype(value)>;
                return std::hash<T>{}(value);
            }, variant);
        }
    };
}


struct ValueEqual {
    bool operator()(const std::shared_ptr<Value>& lhs, const std::shared_ptr<Value>& rhs) const {
        if (lhs == rhs) return true;  // Both pointers are the same
        if (!lhs || !rhs) return false;  // One is null and the other isn't

        // Access the underlying VariantType directly
        const VariantType& lhs_variant = static_cast<const VariantType&>(*lhs);
        const VariantType& rhs_variant = static_cast<const VariantType&>(*rhs);

        return std::visit([](auto&& lhsVal, auto&& rhsVal) -> bool {
            using T1 = std::decay_t<decltype(lhsVal)>;
            using T2 = std::decay_t<decltype(rhsVal)>;
            if constexpr (std::is_same_v<T1, T2>) {
                return lhsVal == rhsVal;
            }
            return false;
        }, lhs_variant, rhs_variant);
    }
};


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
    void addLoop();
    void removeLoop();
    bool inLoop() const;
    void resetLoop();

    void addFunction(const std::string& name, std::shared_ptr<Value> func);
    std::shared_ptr<Value> getFunction(const std::string& name) const;
    bool hasFunction(const std::string& name) const;

    void addMember(ValueType type, const std::string& name, std::shared_ptr<Value> func);
    std::shared_ptr<Value> getMember(std::shared_ptr<ValueType> type, const std::string& name) const;
    bool hasMember(std::shared_ptr<ValueType> type, const std::string& name) const;
private:
    std::vector<Scope> scopes;
    int loop_depth;
    std::unordered_map<std::string, std::shared_ptr<Value>> built_in_functions;
    std::unordered_map<ValueType, std::unordered_map<std::string, std::shared_ptr<Value>>> member_functions;
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
std::optional<std::shared_ptr<Value>> doArithmetic(T1 lhs, T2 rhs, TokenType op, int line, int column);

void printValue(const std::shared_ptr<Value> value);


class ASTNode {
public:
    ASTNode(int line, int column)
        : line{line}, column{column} {}
    virtual ~ASTNode() noexcept = default;
    virtual std::optional<std::shared_ptr<Value>> evaluate(Environment& env) = 0;

    bool debug = false;
    int line, column;
};

// Node for numeric literals
class AtomNode : public ASTNode {
public:
    AtomNode(std::variant<int, double, bool, std::string> value, int line, int column)
        : ASTNode{line, column}, value{std::move(value)} {}

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
    IdentifierNode(std::string value, int line, int column)
        : ASTNode{line, column}, value{value} {}

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    std::optional<std::shared_ptr<Value>> evaluate(Environment& env, std::shared_ptr<ValueType> member_type);

    std::string value;
};

class ListNode : public ASTNode {
public:
    ListNode(ASTList list, int line, int column)
        : ASTNode{line, column}, list{list} {}
    ~ListNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    ASTList list;
};

class DictionaryNode : public ASTNode {
public:
    DictionaryNode(ASTDictionary dictionary, int line, int column)
        : ASTNode{line, column}, dictionary{dictionary} {}
    ~DictionaryNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    ASTDictionary dictionary;
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

// Node for binary operations (e.g., +, -, *, /, //, ^)
class BinaryOpNode : public ASTNode {
public:
    BinaryOpNode(std::shared_ptr<ASTNode> left, TokenType op, std::shared_ptr<ASTNode> right, int line, int column)
        : ASTNode{line, column}, left{left}, op{op}, right{right} {}
    
    ~BinaryOpNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    std::shared_ptr<ASTNode> left;
    TokenType op;  // Operator like _Plus, _Minus
    std::shared_ptr<ASTNode> right;
};

class UnaryOpNode : public ASTNode {
public:
    UnaryOpNode(TokenType op, std::shared_ptr<ASTNode> right, int line, int column)
        : ASTNode{line, column}, op{op}, right{right} {}
    
    ~UnaryOpNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    TokenType op;
    std::shared_ptr<ASTNode> right;
};

class ParenthesisOpNode : public ASTNode {
public:
    ParenthesisOpNode(std::shared_ptr<ASTNode> expr, int line, int column)
        : ASTNode{line, column}, expr{expr} {}
    
    ~ParenthesisOpNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    std::shared_ptr<ASTNode> expr;
};

class ScopeNode : public ASTNode {
public:
    ScopeNode(std::vector<std::shared_ptr<ASTNode>> block, int line, int column)
        : ASTNode{line, column}, block{block} {}
    
    ~ScopeNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;

    std::vector<std::shared_ptr<ASTNode>> block;
};

class ScopedNode : public ASTNode {
public:
    ScopedNode(TokenType keyword, std::shared_ptr<ScopedNode> if_link, std::shared_ptr<ASTNode> comparison,
                std::vector<std::shared_ptr<ASTNode>> statements_block, int line, int column)
        : ASTNode{line, column}, keyword{keyword}, if_link{if_link}, comparison{comparison}, last_comparison_result{false}, statements_block{statements_block} {}

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
    ForNode(TokenType keyword, std::shared_ptr<ASTNode> initialization, std::shared_ptr<std::string> init_string,
            std::shared_ptr<ASTNode> condition_value, std::shared_ptr<ASTNode> increment,
            std::vector<std::shared_ptr<ASTNode>> block, int line, int column)
        : ASTNode{line, column}, keyword{keyword}, initialization{initialization}, init_string{init_string},
            condition_value{condition_value}, increment{increment}, block{block} {}
    
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

class FuncNode : public ASTNode {
public:
    FuncNode(std::shared_ptr<std::string> func_name, std::vector<std::shared_ptr<ASTNode>> args, std::vector<std::shared_ptr<ASTNode>> block, int line, int column)
        : ASTNode{line, column}, args{args}, block{block} {}
    
    ~FuncNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    void setArgs(List values, Environment& base_env, Environment& local_env);
    std::optional<std::shared_ptr<Value>> callFunc(List values, Environment& env);
    
    std::shared_ptr<std::string> func_name;
    std::vector<std::shared_ptr<ASTNode>> args;
    Environment local_env;
    std::vector<std::shared_ptr<ASTNode>> block;
};

class FuncCallNode : public ASTNode {
public:
    FuncCallNode(std::shared_ptr<ASTNode> identifier, std::vector<std::shared_ptr<ASTNode>> values, int line, int column)
        : ASTNode{line, column}, identifier{identifier}, values{values} {}
    
    ~FuncCallNode() noexcept override = default;

    std::optional<std::shared_ptr<Value>> evaluate(Environment& env) override;
    std::optional<std::shared_ptr<Value>> evaluate(Environment& env, std::shared_ptr<ValueType> member_type);
    List evaluateArgs(Environment& env);

    std::shared_ptr<ASTNode> identifier;
    std::vector<std::shared_ptr<ASTNode>> values;
    std::shared_ptr<Value> member_value;
    std::shared_ptr<Environment> base_env = nullptr;
};

class Parser {
public:
    Parser(const std::vector<Token>& tokens)
        : tokens{tokens} {}
    
    std::vector<std::shared_ptr<ASTNode>> parse();

    void addIfElseScope();
    void removeIfElseScope();
    bool debug = false;
private:
    const std::vector<Token>& tokens;
    size_t token_index = 0;
    std::vector<std::shared_ptr<ScopedNode>> last_if_else{nullptr};

    const Token* getToken() const;
    const Token* consume();
    std::optional<const Token*> peek(int ahead) const;
    std::string getTokenStr() const;
    bool tokenIs(std::string str) const;
    bool nextTokenIs(std::string str, int ahead) const;

    std::shared_ptr<ASTNode> parseFoundation();
    std::shared_ptr<ASTNode> parseControlFlowStatement();
    std::shared_ptr<ASTNode> parseKeyword();
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
    std::shared_ptr<ASTNode> parseMemberAccess();
    std::shared_ptr<ASTNode> parseIndexing(std::shared_ptr<ASTNode> left = nullptr);
    std::shared_ptr<ASTNode> parseCollection();
    std::shared_ptr<ASTNode> parseAtom();
    std::shared_ptr<ASTNode> parseFuncCall(std::shared_ptr<ASTNode> identifier = nullptr);
    std::shared_ptr<ASTNode> parseIdentifier(std::shared_ptr<std::string> varString = nullptr);
};


std::string getValueStr(std::shared_ptr<Value> value);
ValueType getValueType(std::shared_ptr<Value> value);
std::string getTypeStr(ValueType value);