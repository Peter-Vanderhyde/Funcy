#include <library.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <format>
#include <iomanip>
#include <vector>
#include <memory>
#include "errorDefs.h"
#include "values.h"

std::string readSourceCodeFromFile(const std::string& filename) {
    if (filename.size() < 3 || filename.substr(filename.size() - 3) != ".fy") {
        throw std::runtime_error("Error: File must have a .fy extension.");
        return "";
    }

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the file's contents into the buffer

    return buffer.str(); // Return the contents as a std::string
}

void printValue(const std::shared_ptr<Value> value) {
    Style style{};
    switch(value->getType()) {
        case ValueType::Integer: {
            int int_value = value->get<int>();
            std::cout << style.light_blue << int_value << style.reset;
            return;
        }
        case ValueType::Float: {
            double float_value = value->get<double>();
            if (float_value == static_cast<int>(float_value)) {
                std::cout << style.light_blue << float_value << ".0" << style.reset;
            } else {
                std::cout << style.light_blue << float_value << style.reset;
            }
            return;
        }
        case ValueType::Boolean: {
            bool bool_value = value->get<bool>();
            std::cout << style.purple << std::boolalpha << bool_value << style.reset;
            return;
        }
        case ValueType::String: {
            std::string string_value = value->get<std::string>();
            std::cout << style.green << "'" << string_value << "'" << style.reset;
            return;
        }
        case ValueType::List: {
            std::shared_ptr<List> list_value = value->get<std::shared_ptr<List>>();
            std::cout << "[";
            bool first = true;
            for (int i = 0; i < list_value->size(); i++) {
                auto item = list_value->at(i);
                if (!first) {
                    std::cout << ", ";
                } else {
                    first = false;
                }
                printValue(item);
            }
            std::cout << "]";
            return;
        }
        case ValueType::None: {
            std::cout << style.blue << "Null" << style.reset;
            return;
        }
        case ValueType::Function: {
            std::cout << style.blue << "Type:Function" << style.reset;
            return;
        }
        case ValueType::BuiltInFunction: {
            std::cout << style.blue << "Type:BuiltInFunction" << style.reset;
            return;
        }
        case ValueType::Type: {
            std::cout << style.blue << getTypeStr(value->getType()) << style.reset;
            return;
        }
        default:
            return;
    }
}

std::vector<std::variant<int, double>> transformNums(std::shared_ptr<Value> first, std::shared_ptr<Value> second) {
    // Takes any combination of bool/int/double and turns them into the same type for adding, dividing, etc

    // Check if either Value contains a string
    if (first->getType() == ValueType::String || second->getType() == ValueType::String) {
        handleError("Attempted to transformNum with a string", 0, 0, "Runtime Error");
    }

    std::variant<int, double> first_num, second_num;

    // Transform the first Value into int or double
    switch (first->getType()) {
        case ValueType::Boolean:
            first_num = first->get<bool>() ? 1 : 0;  // Convert bool to int
            break;
        case ValueType::Integer:
            first_num = first->get<int>();
            break;
        case ValueType::Float:
            first_num = first->get<double>();
            break;
        default:
            handleError("Unsupported type in transformNums for the first value", 0, 0, "Runtime Error");
    }

    // Transform the second Value into int or double
    switch (second->getType()) {
        case ValueType::Boolean:
            second_num = second->get<bool>() ? 1 : 0;  // Convert bool to int
            break;
        case ValueType::Integer:
            second_num = second->get<int>();
            break;
        case ValueType::Float:
            second_num = second->get<double>();
            break;
        default:
            handleError("Unsupported type in transformNums for the second value", 0, 0, "Runtime Error");
    }

    // Check if either number is a double, promoting integers to doubles if necessary
    if (std::holds_alternative<double>(first_num) || std::holds_alternative<double>(second_num)) {
        if (std::holds_alternative<int>(first_num)) {
            return {static_cast<double>(std::get<int>(first_num)), std::get<double>(second_num)};
        } else if (std::holds_alternative<int>(second_num)) {
            return {std::get<double>(first_num), static_cast<double>(std::get<int>(second_num))};
        } else {
            return {std::get<double>(first_num), std::get<double>(second_num)};
        }
    } else {
        // Both are integers
        return {std::get<int>(first_num), std::get<int>(second_num)};
    }
}


BuiltInFunctionReturn print(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    for (const auto& arg : args) {
        printValue(arg);
        std::cout << " ";
    }
    std::cout << std::endl;
    return std::nullopt;
}

BuiltInFunctionReturn intConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("int() conversion takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto arg = args[0];

    switch(arg->getType()) {
        case ValueType::Integer:
            return arg; // If it's already an int, return as is
        case ValueType::Float: {
            int int_value = static_cast<int>(arg->get<double>());
            return std::make_shared<Value>(int_value);
        }
        case ValueType::String: {
            try {
                int int_value = std::stoi(arg->get<std::string>());
                return std::make_shared<Value>(int_value);
            } catch (const std::invalid_argument&) {
                throw std::runtime_error("Cannot convert string to int");
            } catch (const std::out_of_range&) {
                throw std::runtime_error("String value out of range for int conversion");
            }
        }
        case ValueType::Boolean: {
            bool bool_value = arg->get<bool>();
            if (bool_value) {
                return std::make_shared<Value>(1);
            } else {
                return std::make_shared<Value>(0);
            }
        }
        default:
            throw std::runtime_error("Unsupported type " + getValueStr(arg) + " for int conversion");
    }
}

BuiltInFunctionReturn floatConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("float() conversion takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto arg = args[0];

    switch (arg->getType()) {
        case ValueType::Float:
            return arg; // Already a float
        case ValueType::Integer:
            return std::make_shared<Value>(static_cast<double>(arg->get<int>()));
        case ValueType::String: {
            try {
                double doubleValue = std::stod(arg->get<std::string>());
                return std::make_shared<Value>(doubleValue);
            } catch (const std::invalid_argument&) {
                throw std::runtime_error("Cannot convert string to double");
            } catch (const std::out_of_range&) {
                throw std::runtime_error("String value out of range for double conversion");
            }
        }
        case ValueType::Boolean:
            return std::make_shared<Value>(arg->get<bool>() ? 1.0 : 0.0);
        default:
            throw std::runtime_error("Unsupported type for float conversion");
    }
}

BuiltInFunctionReturn boolConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("bool() conversion takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto arg = args[0];

    switch (arg->getType()) {
        case ValueType::Boolean:
            return arg; // Already a bool
        case ValueType::Integer:
            return std::make_shared<Value>(arg->get<int>() != 0);
        case ValueType::Float:
            return std::make_shared<Value>(arg->get<double>() != 0.0);
        case ValueType::String: {
            const std::string& strValue = arg->get<std::string>();
            return std::make_shared<Value>(!strValue.empty() && strValue != "false");
        }
        case ValueType::List:
            return std::make_shared<Value>(!arg->get<std::shared_ptr<List>>()->empty());
        default:
            throw std::runtime_error("Unsupported type for bool conversion");
    }
}

std::string toString(double value){
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

BuiltInFunctionReturn stringConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("string() conversion takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto arg = args[0];

    switch (arg->getType()) {
        case ValueType::String:
            return arg; // Already a string
        case ValueType::Integer:
            return std::make_shared<Value>(std::to_string(arg->get<int>()));
        case ValueType::Float:
            return std::make_shared<Value>(toString(arg->get<double>()));
        case ValueType::Boolean:
            return std::make_shared<Value>(arg->get<bool>() ? "true" : "false");
        case ValueType::List: {
            std::shared_ptr<List> list = arg->get<std::shared_ptr<List>>();
            std::string result = "[";
            for (size_t i = 0; i < list->size(); ++i) {
                auto str_return = stringConverter({list->at(i)}, env).value();
                result += str_return->get<std::string>();
                if (i < list->size() - 1) {
                    result += ", ";
                }
            }
            result += "]";
            return std::make_shared<Value>(result);
        }
        default:
            throw std::runtime_error("Unsupported type for string conversion");
    }
}

BuiltInFunctionReturn listConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    auto list = std::make_shared<List>();

    if (args.size() == 0) {
        throw std::runtime_error("list() conversion takes at least 1 argument. None were given");
    }

    auto arg = args[0];
    switch (arg->getType()) {
        case ValueType::String: {
            // Split string into characters
            const std::string& str = arg->get<std::string>();
            for (char c : str) {
                list->push_back(std::make_shared<Value>(std::string(1, c)));
            }
            break;
        }
        case ValueType::List:
            return arg; // Already a list
        default: {
            for (const auto& element : args) {
                list->push_back(element);
            }
            break;
        }
    }
    return std::make_shared<Value>(list);
}



///  MEMBER FUNCTIONS  ///


BuiltInFunctionReturn listSize(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        runtimeError("size() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() == ValueType::List) {
        auto list = args[0]->get<std::shared_ptr<List>>();
        int size = list->size();
        return std::make_shared<Value>(size);
    } else {
        runtimeError("size() expected a list but got " + getValueStr(args[0]));
    }
    return std::nullopt;
}

BuiltInFunctionReturn listAppend(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        runtimeError("append() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() == ValueType::List) {
        auto list = args[0]->get<std::shared_ptr<List>>();
        if (args[1]->getType() == ValueType::List) {
            auto orig = args[1]->get<std::shared_ptr<List>>();
            auto copy = std::make_shared<List>(*orig);
            list->push_back(std::make_shared<Value>(copy));
        }
        // Add one for dictionaries too
        else {
            list->push_back(args[1]);
        }
        return std::make_shared<Value>();
    } else {
        runtimeError("append() expected a list but got " + getValueStr(args[0]));
    }
    return std::nullopt;
}

BuiltInFunctionReturn listPop(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1 && args.size() != 2) {
        runtimeError("pop() takes 1-2 argument. " + std::to_string(args.size()) + " were given");
    }

    int index;
    if (args.size() == 1) {
        index = -1;
    } else {
        if (args[1]->getType() != ValueType::Integer) {
            runtimeError("pop() expected an integer index but got " + getValueStr(args[1]));
        }
        index = args[1]->get<int>();
    }

    if (args[0]->getType() != ValueType::List) {
        runtimeError("pop() expected a list but got " + getValueStr(args[0]));
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    int size = list->size();

    if (index >= size || index < size * -1) {
        runtimeError("pop() index out of range");
    } else {
        return list->pop(index);
    }
}