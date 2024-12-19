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
        runtimeError("Error: File must have a .fy extension.");
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
            std::cout << style.blue << getTypeStr(value->get<ValueType>()) << style.reset;
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
            runtimeError("Unsupported type in transformNums for the first value");
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
            runtimeError("Unsupported type in transformNums for the second value");
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

BuiltInFunctionReturn getType(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("type() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    return std::make_shared<Value>(args[0]->getType());
}

BuiltInFunctionReturn range(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() < 1 || args.size() > 3) {
        throw std::runtime_error("range() takes 1-3 arguments. " + std::to_string(args.size()) + " were given");
    }

    for (int i = 0; i < args.size(); i++) {
        if (args[i]->getType() != ValueType::Integer) {
            throw std::runtime_error("range() expected int, not " + getTypeStr(args[i]->getType()));
        }
    }

    int start, end, step;

    if (args.size() == 1) {
        end = args[0]->get<int>();
        start = 0;
        step = 1;
    } else if (args.size() == 2) {
        start = args[0]->get<int>();
        end = args[1]->get<int>();
        step = 1;
    } else if (args.size() == 3) {
        start = args[0]->get<int>();
        end = args[1]->get<int>();
        step = args[2]->get<int>();
        if (step == 0) {
            throw std::runtime_error("range() third argument must not be zero");
        }
    }

    std::shared_ptr<List> nums = std::make_shared<List>();
    if (step < 0) {
        for (int i = start; i > end; i += step) {
            nums->push_back(std::make_shared<Value>(i));
        }
    } else {
        for (int i = start; i < end; i += step) {
            nums->push_back(std::make_shared<Value>(i));
        }
    }

    return std::make_shared<Value>(nums);
}



///  MEMBER FUNCTIONS  ///


BuiltInFunctionReturn listSize(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("size() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() == ValueType::List) {
        auto list = args[0]->get<std::shared_ptr<List>>();
        int size = list->size();
        return std::make_shared<Value>(size);
    } else {
        throw std::runtime_error("size() expected a list but got " + getValueStr(args[0]));
    }
    return std::nullopt;
}

BuiltInFunctionReturn listAppend(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("append() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
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
        throw std::runtime_error("append() expected a list but got " + getValueStr(args[0]));
    }
    return std::nullopt;
}

BuiltInFunctionReturn listPop(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1 && args.size() != 2) {
        throw std::runtime_error("pop() takes 1-2 argument. " + std::to_string(args.size()) + " were given");
    }

    int index;
    if (args.size() == 1) {
        index = -1;
    } else {
        if (args[1]->getType() != ValueType::Integer) {
            throw std::runtime_error("pop() expected an integer index but got " + getValueStr(args[1]));
        }
        index = args[1]->get<int>();
    }

    if (args[0]->getType() != ValueType::List) {
        throw std::runtime_error("pop() expected a list but got " + getValueStr(args[0]));
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    int size = list->size();

    if (index >= size || index < size * -1) {
        throw std::runtime_error("pop() index out of range");
    } else {
        return list->pop(index);
    }
}



BuiltInFunctionReturn stringLower(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("lower() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    std::transform(string.begin(), string.end(), string.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    
    return std::make_shared<Value>(string);
}

BuiltInFunctionReturn stringUpper(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("upper() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    std::transform(string.begin(), string.end(), string.begin(),
                    [](unsigned char c) { return std::toupper(c); });
    
    return std::make_shared<Value>(string);
}

BuiltInFunctionReturn stringStrip(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() > 2) {
        throw std::runtime_error("strip() takes 1-2 arguments. " + std::to_string(args.size()) + " were given");
    }

    // Trim from the start (left) of the string
    auto ltrim = [](const std::string& s, const std::string& chars) -> std::string {
        size_t start = s.find_first_not_of(chars);
        return (start == std::string::npos) ? "" : s.substr(start);
    };

    // Trim from the end (right) of the string
    auto rtrim = [](const std::string& s, const std::string& chars) -> std::string {
        size_t end = s.find_last_not_of(chars);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    };

    std::string strip_chars = " \t\n\r\f\v";
    if (args.size() == 2) {
        if (args[1]->getType() == ValueType::String) {
            strip_chars = args[1]->get<std::string>();
        } else {
            throw std::runtime_error("strip() expected an argument of type string");
        }
    }

    return std::make_shared<Value>(ltrim(rtrim(args[0]->get<std::string>(), strip_chars), strip_chars));
}

BuiltInFunctionReturn stringSplit(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() > 2) {
        throw std::runtime_error("split() takes 1-2 arguments. " + std::to_string(args.size()) + " were given");
    }

    // Get the string to split
    if (args[0]->getType() != ValueType::String) {
        throw std::runtime_error("split() expected a string as the first argument");
    }
    std::string str = args[0]->get<std::string>();

    std::string delimiter = " "; // Default delimiter is space
    if (args.size() == 2) {
        if (args[1]->getType() == ValueType::String) {
            delimiter = args[1]->get<std::string>();
        } else {
            throw std::runtime_error("split() expected a string as the delimiter");
        }
    }

    List result;
    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        result.push_back(std::make_shared<Value>(token));
        str.erase(0, pos + delimiter.length());
    }
    result.push_back(std::make_shared<Value>(str)); // Add the last token

    return std::make_shared<Value>(std::make_shared<List>(result));
}

BuiltInFunctionReturn stringIsDigit(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("isDigit() requires exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    for (char c : string) {
        if (!std::isdigit(c)) {
            return std::make_shared<Value>(false);
        }
    }
    return std::make_shared<Value>(true);
}

BuiltInFunctionReturn stringLength(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("length() requires exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    int length = string.length();
    return std::make_shared<Value>(length);
}

BuiltInFunctionReturn stringReplace(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 3) {
        throw std::runtime_error("replace() takes exactly 3 arguments. " + std::to_string(args.size()) + " were given");
    }

    // Get the string to modify
    std::string str = args[0]->get<std::string>();

    // Get the substring to replace
    if (args[1]->getType() != ValueType::String) {
        throw std::runtime_error("replace() expected a string as the second argument (substring to replace)");
    }
    std::string to_replace = args[1]->get<std::string>();

    // Get the replacement substring
    if (args[2]->getType() != ValueType::String) {
        throw std::runtime_error("replace() expected a string as the third argument (replacement substring)");
    }
    std::string replacement = args[2]->get<std::string>();

    // Perform the replacement
    size_t pos = 0;
    while ((pos = str.find(to_replace, pos)) != std::string::npos) {
        str.replace(pos, to_replace.length(), replacement);
        pos += replacement.length(); // Move past the last replacement
    }

    return std::make_shared<Value>(str); // Return the modified string
}