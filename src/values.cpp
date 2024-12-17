#include "values.h"
#include <vector>
#include "errorDefs.h"


void List::append(std::shared_ptr<Value> value) {
        elements.push_back(value);
    }

std::shared_ptr<Value> List::pop(size_t index) {
    auto element = at(index);
    elements.erase(elements.begin() + index);
    return element;
}

void List::insert(size_t index, std::shared_ptr<Value> value) {
    if (index >= elements.size() + 1 || index < 0) {
        throw std::out_of_range("Index out of range");
    }
    elements.insert(elements.begin() + index, value);
}

// Overload for inserting a List directly
void List::insert(const std::shared_ptr<List>& other) {
    if (!other) {
        throw std::invalid_argument("Other list is null");
    }
    elements.insert(elements.end(), other->elements.begin(), other->elements.end());
}

std::shared_ptr<Value> List::at(size_t index) const {
    if (index >= elements.size() || index < 0) {
        throw std::out_of_range("Index out of range");
    }
    return elements.at(index);
}

size_t List::size() const {
    return elements.size();
}

bool List::empty() const {
    return elements.empty();
}


Value::Value()  // Defaults to monostate (NONE)
    : value_type{ValueType::None} {}

Value::Value(int v)
    : value{v}, value_type{ValueType::Integer} {}

Value::Value(double v)
    : value{v}, value_type{ValueType::Float} {}

Value::Value(bool v)
    : value{v}, value_type{ValueType::Boolean} {}

Value::Value(const std::string& v)
    : value{v}, value_type{ValueType::String} {}

Value::Value(std::shared_ptr<List> v)
    : value{v}, value_type{ValueType::List} {}

Value::Value(SpecialIndex v)
    : value{v}, value_type{ValueType::Index} {}

Value::Value(std::shared_ptr<ASTNode> v)
    : value{v}, value_type{ValueType::Function} {}

// Get the current type of the Value
ValueType Value::getType() const {
    return value_type;
}