#pragma once

#include <variant>
#include <string>
#include <memory>
#include <iostream>
#include <vector>

enum class SpecialIndex {
    Begin,
    End
};

class Value;
class ASTNode;

class List {
private:
    std::vector<std::shared_ptr<Value>> elements;

public:
    List() {}

    void append(std::shared_ptr<Value> value);
    std::shared_ptr<Value> pop(size_t index);
    void insert(size_t index, std::shared_ptr<Value> value);
    void insert(const std::shared_ptr<List>& other); // Overload for inserting a List directly
    std::shared_ptr<Value> at(size_t index) const;
    size_t size() const;
    bool empty() const;
};

enum class ValueType {
    Integer,
    Boolean,
    String,
    Float,
    List,
    None,
    Function,
    Index
};

class Value {
private:
    std::variant<std::monostate, int, double, bool, std::string, std::shared_ptr<List>,
                SpecialIndex, std::shared_ptr<ASTNode>> value;
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

    ValueType getType() const;

    // Templated getter function
    template <typename T>
    const T& get() const {
        if (!std::holds_alternative<T>(value)) {
            throw std::runtime_error("Incorrect type access in Value");
        }
        return std::get<T>(value);
    }

};