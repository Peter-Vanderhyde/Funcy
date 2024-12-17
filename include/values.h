#pragma once

#include <variant>
#include <string>
#include <memory>
#include <iostream>
#include <vector>

class Value;

class List {
private:
    std::vector<std::shared_ptr<Value>> elements;

public:
    List() {}

    void append(std::shared_ptr<Value> value);
    std::shared_ptr<Value> pop(size_t index);
    void insert(size_t index, std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(size_t index) const;
    size_t size() const;
    bool empty() const;
};

enum class ValueType {
    Integer,
    Boolean,
    String,
    Float,
    List,
    None
};

class Value {
private:
    std::variant<std::monostate, int, double, bool, std::string, std::shared_ptr<List>> value;
    ValueType value_type;

public:
    Value();
    Value(int v);
    Value(double v);
    Value(bool v);
    Value(const std::string& v);
    Value(std::shared_ptr<List> v);

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