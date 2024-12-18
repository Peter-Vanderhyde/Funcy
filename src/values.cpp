#include "values.h"
#include <vector>
#include "errorDefs.h"


void List::push_back(std::shared_ptr<Value> value) {
        elements.push_back(value);
    }

std::shared_ptr<Value> List::pop(size_t index) {
    if (index < 0) {
        auto value = elements.at(elements.size() - -index);
        elements.erase(elements.end() - -index);
        return value;
    } else {
        auto value = elements.at(index);
        elements.erase(elements.begin() + index);
        return value;
    }
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

Value::Value(std::shared_ptr<BuiltInFunction> v)
    : value{v}, value_type{ValueType::BuiltInFunction} {}

Value::Value(ValueType v)
    : value{v}, value_type{ValueType::Type} {}

// Get the current type of the Value
ValueType Value::getType() const {
    return value_type;
}


std::string getValueStr(std::shared_ptr<Value> value) {
    switch(value->getType()) {
        case ValueType::Integer:
            return "integer";
        case ValueType::Float:
            return "float";
        case ValueType::Boolean:
            return "boolean";
        case ValueType::String:
            return "string";
        case ValueType::List:
            return "list";
        case ValueType::Function:
            return "function";
        case ValueType::BuiltInFunction:
            return "builtin function";
        case ValueType::Type:
            return "type";
        case ValueType::None:
            return "null";
        default:
            runtimeError("Attempted to get string of unrecognized Value type.");
    }
}

std::string getValueStr(Value value) {
    switch(value.getType()) {
        case ValueType::Integer:
            return "integer";
        case ValueType::Float:
            return "float";
        case ValueType::Boolean:
            return "boolean";
        case ValueType::String:
            return "string";
        case ValueType::List:
            return "list";
        case ValueType::Function:
            return "function";
        case ValueType::BuiltInFunction:
            return "builtin function";
        case ValueType::Type:
            return "type";
        case ValueType::None:
            return "null";
        default:
            runtimeError("Attempted to get string of unrecognized Value type.");
    }
}

std::string getTypeStr(ValueType type) {
    std::unordered_map<ValueType, std::string> types = {
        {ValueType::Integer, "Type:Integer"},
        {ValueType::Float, "Type:Float"},
        {ValueType::Boolean, "Type:Boolean"},
        {ValueType::String, "Type:String"},
        {ValueType::Function, "Type:Function"},
        {ValueType::List, "Type:List"},
        {ValueType::BuiltInFunction, "Type:BuiltInFunction"},
        {ValueType::Type, "Type:Type"},
        {ValueType::None, "Type:Null"}
    };
    if (types.count(type) != 0) {
        return types[type];
    } else {
        runtimeError("No such type string found");
    }
}