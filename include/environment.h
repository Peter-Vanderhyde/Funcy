#pragma once
#include <variant>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>

class Value;
class Instance;
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
    void setClassEnv();
    bool isClassEnv() const;
    void set(std::string name, std::shared_ptr<Value> value, bool is_member_var = false);
    bool contains(std::string name, bool is_member_var = false) const;
    std::shared_ptr<Value> get(std::string name, bool is_member_var = false) const;

    int scopeDepth() const;
    void addScope();
    void addScope(Scope& scope);
    void addClassScope();
    Scope getScope();
    int classDepth();
    void removeScope();
    void removeClassScope(Scope& previous_attrs);
    std::vector<Scope> copyScopes() const;
    void addLoop();
    void removeLoop();
    bool inLoop() const;
    void resetLoop();

    void addFunction(const std::string& name, std::shared_ptr<Value> func);
    std::shared_ptr<Value> getFunction(const std::string& name) const;
    bool hasFunction(const std::string& name) const;

    void addMember(ValueType type, const std::string& name, std::shared_ptr<Value> func);
    void addMember(const std::string& name, std::shared_ptr<Value> value);
    std::shared_ptr<Value> getMember(ValueType type, const std::string& name) const;
    std::shared_ptr<Value> getMember(const std::string& name) const;
    bool hasMember(ValueType type, const std::string& name) const;
    bool hasMember(const std::string& name) const;

    void addGlobal(std::string name);
    void resetGlobals();
    void removeGlobalScope();
    bool isGlobal(std::string name) const;
    void setGlobalValue(std::string name, std::shared_ptr<Value> value);
    Scope& getClassAttrs();
    void setClassAttrs(Scope& scope);

    void setThis(std::shared_ptr<Value> inst_ref);
    std::shared_ptr<Value> getThis();

    void display(bool show_attrs = false) const;

    bool is_top_scope = false;
private:
    std::vector<Scope> scopes;
    Scope class_attrs;
    std::shared_ptr<Value> this_ref = nullptr;
    bool class_env;
    int class_depth = 0;
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