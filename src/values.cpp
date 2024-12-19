#include "values.h"
#include <vector>
#include "errorDefs.h"


bool ValueCompare::operator()(const std::shared_ptr<Value>& lhs, const std::shared_ptr<Value>& rhs) const {
    // Handle null pointers
    if (!lhs || !rhs) {
        return lhs < rhs; // Compare pointers directly if either is null
    }

    // Compare ValueTypes first to establish ordering based on type
    if (lhs->getType() != rhs->getType()) {
        return lhs->getType() < rhs->getType();
    }

    // If the types are the same, compare the actual values
    switch (lhs->getType()) {
        case ValueType::Integer:
            return lhs->get<int>() < rhs->get<int>();
        case ValueType::Float:
            return lhs->get<double>() < rhs->get<double>();
        case ValueType::Boolean:
            return lhs->get<bool>() < rhs->get<bool>();
        case ValueType::String:
            return lhs->get<std::string>() < rhs->get<std::string>();
        case ValueType::List: {
            auto lhs_list = lhs->get<std::shared_ptr<List>>();
            auto rhs_list = rhs->get<std::shared_ptr<List>>();

            // Compare sizes first
            if (lhs_list->size() != rhs_list->size()) {
                return lhs_list->size() < rhs_list->size();
            }

            // Compare element-by-element if sizes are equal
            for (size_t i = 0; i < lhs_list->size(); ++i) {
                if (ValueCompare{}(lhs_list->at(i), rhs_list->at(i))) {
                    return true; // lhs[i] < rhs[i]
                }
                if (ValueCompare{}(rhs_list->at(i), lhs_list->at(i))) {
                    return false; // rhs[i] < lhs[i]
                }
            }

            // Lists are equal if all elements are equal
            return false;
        }
        case ValueType::Dictionary: {
            auto lhs_dict = lhs->get<std::shared_ptr<Dictionary>>();
            auto rhs_dict = rhs->get<std::shared_ptr<Dictionary>>();

            // Compare sizes first
            if (lhs_dict->size() != rhs_dict->size()) {
                return lhs_dict->size() < rhs_dict->size();
            }

            // Compare key-value pairs lexicographically
            auto lhs_iter = lhs_dict->begin();
            auto rhs_iter = rhs_dict->begin();
            while (lhs_iter != lhs_dict->end() && rhs_iter != rhs_dict->end()) {
                // Compare keys
                if (ValueCompare{}(lhs_iter->first, rhs_iter->first)) {
                    return true; // lhs_key < rhs_key
                }
                if (ValueCompare{}(rhs_iter->first, lhs_iter->first)) {
                    return false; // rhs_key < lhs_key
                }

                // Compare values
                if (ValueCompare{}(lhs_iter->second, rhs_iter->second)) {
                    return true; // lhs_value < rhs_value
                }
                if (ValueCompare{}(rhs_iter->second, lhs_iter->second)) {
                    return false; // rhs_value < lhs_value
                }

                ++lhs_iter;
                ++rhs_iter;
            }

            // Dictionaries are equal if all key-value pairs are equal
            return false;
        }
        default:
            // For unsupported types, use address comparison as a fallback
            return lhs < rhs;
    }
}



void List::push_back(std::shared_ptr<Value> value) {
        elements.push_back(value);
    }

std::shared_ptr<Value> List::pop(int index) {
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

Value::Value(std::shared_ptr<Dictionary> v)
    : value{v}, value_type{ValueType::Dictionary} {}

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
        case ValueType::Dictionary:
            return "dictionary";
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
        case ValueType::Dictionary:
            return "dictionary";
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
        {ValueType::Dictionary, "Type:Dictionary"},
        {ValueType::None, "Null"}
    };
    if (types.count(type) != 0) {
        return types[type];
    } else {
        runtimeError("No such type string found");
    }
}