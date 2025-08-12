#include "values.h"
#include <vector>
#include "errorDefs.h"

class FuncNode;

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



List::List(std::vector<std::shared_ptr<Value>> elements)
    : elements{elements} {}

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

void List::set(size_t index, std::shared_ptr<Value> value) {
    if (index >= elements.size()) {
        throw std::out_of_range("Index out of range");
    }
    elements[index] = value;
}

bool compareValues(std::shared_ptr<Value> left, std::shared_ptr<Value> right) {
    if (left->getType() == right->getType()) {
        ValueType type = left->getType();
        switch (type) {
            case ValueType::Integer:
                return left->get<int>() == right->get<int>();
            case ValueType::Float:
                return left->get<double>() == right->get<double>();
            case ValueType::Boolean:
                return left->get<bool>() == right->get<bool>();
            case ValueType::String:
                return left->get<std::string>() == right->get<std::string>();
            case ValueType::List: {
                auto l_list = left->get<std::shared_ptr<List>>();
                auto r_list = right->get<std::shared_ptr<List>>();
                if (l_list->size() == r_list->size()) {
                    for (int i = 0; i < l_list->size(); i++) {
                        if (!compareValues(l_list->at(i), r_list->at(i))) {
                            return false;
                        }
                    }
                } else {
                    return false;
                }
            }
            case ValueType::Dictionary: {
                auto l_dict = left->get<std::shared_ptr<Dictionary>>();
                auto r_dict = right->get<std::shared_ptr<Dictionary>>();
                if (l_dict->size() == r_dict->size()) {
                    for (int i = 0; i < l_dict->size(); i++) {
                        auto l_index_pair = std::next(l_dict->begin(), i);
                        auto r_index_pair = std::next(r_dict->begin(), i);
                        if (!compareValues(l_index_pair->first, r_index_pair->first)) {
                            return false;
                        } else if (!compareValues(l_index_pair->second, r_index_pair->second)) {
                            return false;
                        }
                    }
                    return true;
                } else {
                    return false;
                }
            }
            case ValueType::None:
                return true;
            case ValueType::Function:
                return left->get<std::shared_ptr<ASTNode>>() == right->get<std::shared_ptr<ASTNode>>();
            case ValueType::BuiltInFunction:
                return left->get<std::shared_ptr<BuiltInFunction>>() == right->get<std::shared_ptr<BuiltInFunction>>();
            case ValueType::Class:
                return left->get<std::shared_ptr<Class>>() == right->get<std::shared_ptr<Class>>();
            case ValueType::Instance:
                return left->get<std::shared_ptr<Instance>>() == right->get<std::shared_ptr<Instance>>();
            case ValueType::Type:
                return left->get<ValueType>() == right->get<ValueType>();
            default:
                throw std::runtime_error("Comparing unkown types");
        }
    } else if ((left->getType() == ValueType::Integer && right->getType() == ValueType::Float) ||
                (left->getType() == ValueType::Float && right->getType() == ValueType::Integer)) {
        double l_float, r_float;
        if (left->getType() == ValueType::Integer) {
            l_float = static_cast<double>(left->get<int>());
            r_float = right->get<double>();
        } else {
            l_float = left->get<double>();
            r_float = static_cast<double>(right->get<int>());
        }

        return l_float == r_float;
    } else {
        return false;
    }
}

void List::erase(const std::shared_ptr<Value> value) {
    for (auto it = elements.begin(); it != elements.end(); it++) {
        if (compareValues(*it, value)) {
            elements.erase(it);
            return;
        }
    }
    throw std::runtime_error(getValueStr(value) + " is not in list");
}

int List::index(const std::shared_ptr<Value> value, int start, int end) const {
    for (int i = start; i < end; i++) {
        if (elements.begin() + i == elements.end()) {
            throw std::out_of_range("Index out of range");
        }
        auto it = elements.begin() + i;
        if (compareValues(*it, value)) {
            return i;
        }
    }
    throw std::runtime_error(getValueStr(value) + " is not in list");
}

std::shared_ptr<Value> List::at(size_t index) const {
    if (index >= elements.size() || index < 0) {
        throw std::out_of_range("Index out of range");
    }
    return elements.at(index);
}

std::vector<std::shared_ptr<Value>> List::getElements() {
    return elements;
}

size_t List::size() const {
    return elements.size();
}

bool List::empty() const {
    return elements.empty();
}

void List::clear() {
    elements.clear();
}


Class::Class(std::string name, Environment& class_env)
        : name{name}, class_env{class_env} {}

std::shared_ptr<Instance> Class::createInstance() {
    return std::make_shared<Instance>(name, class_env);
}

std::string Class::getName() const {
    return name;
}


std::shared_ptr<Value> Instance::getConstructor(std::shared_ptr<Instance> this_reference) {
    auto constructor = instance_env.get(class_name, true);
    if (constructor->getType() != ValueType::Function) {
        runtimeError(class_name + " Class constructor does not exist", "");
    }
    return constructor;
}

Environment& Instance::getEnvironment() {
    return instance_env;
}

Environment Instance::copyEnvironment() {
    return Environment{instance_env};
}

std::string Instance::getClassName() const {
    return class_name;
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

Value::Value(std::shared_ptr<Class> v)
    : value{v}, value_type{ValueType::Class} {}

Value::Value(std::shared_ptr<Instance> v)
    : value{v}, value_type{ValueType::Instance} {}

// Get the current type of the Value
ValueType Value::getType() const {
    return value_type;
}

std::string Value::getPrintable(int tabs) {
    Style style{}; // assumes fields like .light_blue, .purple, .green, .blue, .red, .reset

    switch (value_type) {
        case ValueType::Integer: {
            return style.light_blue + std::to_string(std::get<int>(value)) + style.reset;
        }

        case ValueType::Float: {
            // keep your existing formatting (std::to_string), just color it
            return style.light_blue + std::to_string(std::get<double>(value)) + style.reset;
        }

        case ValueType::Boolean: {
            const bool b = std::get<bool>(value);
            return style.purple + std::string(b ? "true" : "false") + style.reset;
        }

        case ValueType::String: {
            // keep double quotes, just color the whole token like printValue (green)
            const auto& s = std::get<std::string>(value);
            return style.green + "'" + s + "'" + style.reset;
        }

        case ValueType::List: {
            auto list = std::get<std::shared_ptr<List>>(value);
            std::string str = "[";
            for (int i = 0; i < list->size(); ++i) {
                if (i != 0) str += ", ";
                // elements print with their own colors
                str += list->at(i)->getPrintable();
            }
            str += "]";
            return str;
        }

        case ValueType::Dictionary: {
            auto dict = std::get<std::shared_ptr<Dictionary>>(value);
            std::string str = "{\n";
            for (const auto& pair : *dict) {
                // keep your formatting & indent; keys/values color via recursion
                for (int i = 0; i < tabs; i++) {
                    str += "|   ";
                }
                str += pair.first->getPrintable()
                    + " : "
                    + pair.second->getPrintable()
                    + "\n";
            }
            str += std::string(tabs, ' ') + "}";
            return str;
        }

        case ValueType::Function:
            return style.blue + std::string("<function>") + style.reset;

        case ValueType::BuiltInFunction:
            return style.blue + std::string("<builtin_function>") + style.reset;

        case ValueType::Type:
            return style.blue + getTypeStr(std::get<ValueType>(value)) + style.reset;

        case ValueType::Class:
            return style.blue + std::string("<class>") + style.reset;

        case ValueType::Instance:
            return style.blue + std::string("<instance>") + style.reset;

        case ValueType::None:
            return style.blue + std::string("null") + style.reset;

        default:
            runtimeError("Attempted to get printable string of unrecognized Value type.", "");
            return "";
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
        case ValueType::Class:
            return "class";
        case ValueType::Instance:
            return "instance";
        case ValueType::None:
            return "null";
        default:
            runtimeError("Attempted to get string of unrecognized Value type.", "");
    }
}

std::string getValueStr(std::shared_ptr<Value> value) {
    return getValueStr(*value);
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
        {ValueType::Class, "Type:Class"},
        {ValueType::Instance, "Type:Instance"},
        {ValueType::None, "Null"}
    };
    if (types.count(type) != 0) {
        return types[type];
    } else {
        runtimeError("No such type string found", "");
    }
}