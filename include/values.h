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
#include "errorDefs.h"

enum class SpecialIndex {
    Front,
    Back
};

class FuncNode;
class Value;
class ASTNode;
class Scope;
class Class;
class Instance;

struct ValueCompare {
    bool operator()(const std::shared_ptr<Value>& lhs, const std::shared_ptr<Value>& rhs) const;
};

using Dictionary = std::map<std::shared_ptr<Value>, std::shared_ptr<Value>, ValueCompare>;
using BuiltInFunction = std::function<std::optional<std::shared_ptr<Value>>(
    const std::vector<std::shared_ptr<Value>>& args, Scope& scope
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

class Class : public std::enable_shared_from_this<Class> {
private:
    std::string name;
    std::unordered_map<std::string, std::shared_ptr<Value>> public_member_values;

public:
    Class(std::string name);
    
    std::shared_ptr<Instance> createInstance();
    std::string getName() const;
    bool contains(const std::string name) const;
    std::shared_ptr<Value> copyValue(const std::string name) const;
    void setValue(const std::string name, const std::shared_ptr<Value> value);
};

class Instance {
private:
    std::shared_ptr<Class> parent_class;
    std::unordered_map<std::string, std::shared_ptr<Value>> edited_member_variables;

public:
    Instance(const std::shared_ptr<Class>& parent_class);
    
    std::shared_ptr<Value> getConstructor() const;
    void set(const std::string name, const std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(const std::string name) const;
    bool contains(const std::string name) const;
    void remove(const std::string name);
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
    bool is_private;

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
            throwError(ErrorType::Runtime, "Incorrect type access in value");
        }
        return std::get<T>(value);
    }

    bool isPrivate() const;

    std::string getPrintable(int tabs=0, bool error=false);

};


std::string getValueStr(std::shared_ptr<Value> value);
std::string getValueStr(Value value);
std::string getTypeStr(ValueType type);