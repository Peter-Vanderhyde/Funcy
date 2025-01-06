#pragma once
#include <variant>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>

class Value;
enum class ValueType;

class Scope {
public:
    Scope();
    void set(std::string name, std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(std::string name) const;
    bool contains(std::string name) const;
    const std::vector<std::pair<std::string, std::shared_ptr<Value>>> getPairs() const;
    void display() const;
private:
    std::unordered_map<std::string, std::shared_ptr<Value>> variables;
};

class Environment {
public:
    Environment();
    Environment(const Environment& other);
    void setClassEnv(Scope& class_scope);
    bool isClassEnv() const;
    void set(std::string name, std::shared_ptr<Value> value, bool is_member_var = false);
    bool contains(std::string name, bool is_member_var = false) const;
    std::shared_ptr<Value> get(std::string name, bool is_member_var = false) const;

    int scopeDepth() const;
    void addScope();
    void addScope(Scope& scope);
    void addClassScope();
    Scope getScope();
    void removeScope();
    void removeClassScope();
    std::vector<Scope> copyScopes() const;
    void addLoop();
    void removeLoop();
    bool inLoop() const;
    void resetLoop();

    void addFunction(const std::string& name, std::shared_ptr<Value> func);
    std::shared_ptr<Value> getFunction(const std::string& name) const;
    bool hasFunction(const std::string& name) const;

    void addMember(ValueType type, const std::string& name, std::shared_ptr<Value> func);
    std::shared_ptr<Value> getMember(ValueType type, const std::string& name) const;
    bool hasMember(ValueType type, const std::string& name) const;

    void addGlobal(std::string name);
    void resetGlobals();
    void removeGlobalScope();
    bool isGlobal(std::string name) const;
    void setGlobalValue(std::string name, std::shared_ptr<Value> value);
    Scope& getClassScope();
    void setClassScope(const Scope& class_scope);
    Scope& getClassAttrs();
    void setClassAttrs(Scope& scope);

    void display(bool show_attrs = false) const;

    bool is_top_scope = false;
private:
    std::vector<Scope> scopes;
    std::vector<Scope> class_scopes;
    Scope class_attrs;
    bool class_env;
    bool class_depth = 0;
    int loop_depth = 0;
    std::unordered_map<std::string, std::shared_ptr<Value>> built_in_functions;
    std::unordered_map<ValueType, std::unordered_map<std::string, std::shared_ptr<Value>>> member_functions;
    std::unordered_map<int, std::vector<std::string>> scoped_globals;
};

class BreakException : public std::exception {};
class ContinueException : public std::exception {};
class ReturnException : public std::exception {
public:
    ReturnException(std::optional<std::shared_ptr<Value>> value)
        : value{value} {}

    std::optional<std::shared_ptr<Value>> value;
};
class StackOverflowException : public std::exception {};