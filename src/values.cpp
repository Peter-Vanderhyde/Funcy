#include "values.h"
#include <vector>
#include "errorDefs.h"


void List::append(std::shared_ptr<Value> value) {
        elements.push_back(value);
    }

std::shared_ptr<Value> List::pop(size_t index) {
    auto element = get(index);
    elements.erase(elements.begin() + index);
    return element;
}

void List::insert(size_t index, std::shared_ptr<Value> value) {
    if (index >= elements.size() + 1 || index < 0) {
        throw std::out_of_range("Index out of range");
    }
    elements.insert(elements.begin() + index, value);
}

std::shared_ptr<Value> List::get(size_t index) const {
    if (index >= elements.size() || index < 0) {
        throw std::out_of_range("Index out of range");
    }
    return elements[index];
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

// Get the current type of the Value
ValueType Value::getType() const {
    return value_type;
}