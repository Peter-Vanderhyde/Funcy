#pragma once

#include <variant>
#include <string>
#include <memory>
#include <iostream>
#include <vector>
#include <functional>
#include <optional>
#include <map>
#include "environment.h"

enum class SpecialIndex {
    Front,
    Back
};

class FuncNode;
class Value;
class ASTNode;
class Environment;
class Scope;
class Class;
class Instance;

struct ValueCompare {
    bool operator()(const std::shared_ptr<Value>& lhs, const std::shared_ptr<Value>& rhs) const;
};

using Dictionary = std::map<std::shared_ptr<Value>, std::shared_ptr<Value>, ValueCompare>;
using BuiltInFunction = std::function<std::optional<std::shared_ptr<Value>>(
    const std::vector<std::shared_ptr<Value>>& args, Environment& env
)>;
using ValueList = std::vector<std::shared_ptr<Value>>;

class List {
private:
    std::vector<std::shared_ptr<Value>> elements;

public:
    List() {}
    List(std::vector<std::shared_ptr<Value>> elements);

    void push_back(std::shared_ptr<Value> value);
    std::shared_ptr<Value> pop(int index);
    void insert(size_t index, std::shared_ptr<Value> value);
    void insert(const std::shared_ptr<List>& other); // Overload for inserting a List directly
    void set(size_t index, std::shared_ptr<Value> value);
    void erase(const std::shared_ptr<Value> value);
    int index(const std::shared_ptr<Value> value, int start, int end) const;
    std::shared_ptr<Value> at(size_t index) const;
    std::vector<std::shared_ptr<Value>> getElements();
    size_t size() const;
    bool empty() const;
    void clear();

};

/*
Add instance comparison
*/

class Class {
private:
    std::string name;
    Environment class_env;

public:
    Class(std::string name, Environment& class_env);
    
    std::shared_ptr<Instance> createInstance();
    std::string getName() const;
};

class Instance {
private:
    std::string class_name;
    Environment instance_env;

public:
    Instance(std::string class_name, Environment instance_env)
        : class_name{class_name}, instance_env{instance_env} {}
    
    std::shared_ptr<Value> getConstructor(std::shared_ptr<Instance> this_reference);
    Environment& getEnvironment();
    Environment copyEnvironment();
    std::string getClassName() const;
};

enum class ValueType {
    Integer,
    Boolean,
    String,
    Float,
    List,
    Dictionary,
    None,
    Function,
    Index,
    BuiltInFunction,
    Class,
    Instance,
    Type
};

class Value {
private:
    std::variant<std::monostate, int, double, bool, std::string, std::shared_ptr<List>,
                SpecialIndex, std::shared_ptr<ASTNode>, std::shared_ptr<BuiltInFunction>, ValueType,
                std::shared_ptr<Dictionary>, std::shared_ptr<Class>, std::shared_ptr<Instance>> value;
    ValueType value_type;

public:
    Value();
    Value(int v);
    Value(double v);
    Value(bool v);
    Value(const std::string& v);
    Value(std::shared_ptr<List> v);
    Value(SpecialIndex v);
    Value(std::shared_ptr<ASTNode> v);
    Value(std::shared_ptr<BuiltInFunction> v);
    Value(ValueType v);
    Value(std::shared_ptr<Dictionary> v);
    Value(std::shared_ptr<Class> v);
    Value(std::shared_ptr<Instance> v);

    ValueType getType() const;

    // Templated getter function
    template <typename T>
    const T& get() const {
        if (!std::holds_alternative<T>(value)) {
            throw std::runtime_error("Incorrect type access in Value");
        }
        return std::get<T>(value);
    }

    std::string getPrintable(int tabs=0);

};


std::string getValueStr(std::shared_ptr<Value> value);
std::string getValueStr(Value value);
std::string getTypeStr(ValueType type);