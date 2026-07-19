#pragma once
#include <variant>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>

class Value;
class Instance;
class Class;
enum class ValueType;

extern bool DETECT_RECURSION;

class Scope : public std::enable_shared_from_this<Scope> {
public:
    Scope(std::shared_ptr<Scope> parent = nullptr);
    void set(const std::string name, const std::shared_ptr<Value> value, const bool member_variable = false);
    std::shared_ptr<Value> get(const std::string name, const bool member_variable = false) const;
    bool contains(const std::string name, const bool member_variable = false) const;
    bool find(const std::string name, const bool member_variable = false) const;

    void addFunction(const std::string name, const std::shared_ptr<Value> func);
    std::shared_ptr<Value> getFunction(const std::string name) const;
    bool hasFunction(const std::string name) const;
    void addMember(const ValueType value_type, const std::string name, const std::shared_ptr<Value> func);
    std::shared_ptr<Value> getMember(const ValueType value_type, const std::string name) const;
    bool hasMember(const ValueType value_type, const std::string name) const;

    std::shared_ptr<Scope> getGlobalScope();
    std::shared_ptr<Scope> enterScope();
    std::shared_ptr<Scope> exitScope();

    void assignClass(const std::shared_ptr<Class> defined_class);
    bool hasClassAssigned() const;
    std::shared_ptr<Class> getAssignedClass() const;

    void addGlobal(const std::string name);
    void setGlobal(const std::string name, const std::shared_ptr<Value> value);
    std::shared_ptr<Value> getGlobal(const std::string name) const;
    void setThis(const std::shared_ptr<Instance> instance);
    std::shared_ptr<Instance> getThis() const;
    std::shared_ptr<Instance> findThis() const;
    bool hasThis() const;

    void addLoop();
    void removeLoop();
    bool inLoop() const;
    void resetLoop();

    const std::vector<std::pair<std::string, std::shared_ptr<Value>>> getPairs() const;
    void display(int depth = 0) const;

private:
    void set(const std::string name,
        const std::shared_ptr<Value> value,
        Scope& original_scope,
        std::unordered_map<std::string, std::shared_ptr<Value>>& original_scope_variables);

    int loop_depth = 0;
    std::vector<std::string> global_list;
    std::shared_ptr<Instance> this_ref;
    std::shared_ptr<Scope> parent;
    std::shared_ptr<Class> assigned_class; // For scopes to assign member variables to classes
    std::unordered_map<std::string, std::shared_ptr<Value>> variables;
    std::unordered_map<std::string, std::shared_ptr<Value>> member_variables;
    std::unordered_map<std::string, std::shared_ptr<Value>> built_in_functions;
    std::unordered_map<ValueType, std::unordered_map<std::string, std::shared_ptr<Value>>> type_members;
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