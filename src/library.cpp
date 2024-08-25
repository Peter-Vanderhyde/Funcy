#include "library.h"
#include "parser.h"
#include "pch.h"
#include <iostream>
#include <fstream>


std::string readSourceCodeFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the file's contents into the buffer

    return buffer.str(); // Return the contents as a std::string
}


///  BUILT-IN FUNCTIONS  ///


BuiltInFunctionReturn print(const std::vector<std::shared_ptr<Value>>& args) {
    for (const auto& arg : args) {
        printValue(arg);
    }
    std::cout << std::endl;
    return std::nullopt;
}

BuiltInFunctionReturn intConverter(const std::vector<std::shared_ptr<Value>>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Int conversion takes exactly one argument");
    }

    auto arg = args[0];

    if (std::holds_alternative<int>(*arg)) {
        return arg; // If it's already an int, return as is
    } else if (std::holds_alternative<double>(*arg)) {
        int int_value = static_cast<int>(std::get<double>(*arg));
        return std::make_shared<Value>(int_value);
    } else if (std::holds_alternative<std::string>(*arg)) {
        try {
            int int_value = std::stoi(std::get<std::string>(*arg));
            return std::make_shared<Value>(int_value);
        } catch (const std::invalid_argument&) {
            throw std::runtime_error("Cannot convert string to int");
        } catch (const std::out_of_range&) {
            throw std::runtime_error("String value out of range for int conversion");
        }
    } else if (std::holds_alternative<bool>(*arg)) {
        bool bool_value = std::get<bool>(*arg);
        if (bool_value) {
            return std::make_shared<Value>(1);
        } else {
            return std::make_shared<Value>(0);
        }
    } else {
        throw std::runtime_error("Unsupported type " + getValueStr(arg) + " for int conversion");
    }
}

std::optional<std::shared_ptr<Value>> floatConverter(const std::vector<std::shared_ptr<Value>>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Double conversion takes exactly one argument");
    }

    auto arg = args[0];

    if (std::holds_alternative<double>(*arg)) {
        return arg; // If it's already a double, return as is
    } else if (std::holds_alternative<int>(*arg)) {
        double doubleValue = static_cast<double>(std::get<int>(*arg));
        return std::make_shared<Value>(doubleValue);
    } else if (std::holds_alternative<std::string>(*arg)) {
        try {
            double doubleValue = std::stod(std::get<std::string>(*arg));
            return std::make_shared<Value>(doubleValue);
        } catch (const std::invalid_argument&) {
            throw std::runtime_error("Cannot convert string to double");
        } catch (const std::out_of_range&) {
            throw std::runtime_error("String value out of range for double conversion");
        }
    } else if (std::holds_alternative<bool>(*arg)) {
        bool bool_value = std::get<bool>(*arg);
        if (bool_value) {
            return std::make_shared<Value>(1.0);
        } else {
            return std::make_shared<Value>(0.0);
        }
    } else {
        throw std::runtime_error("Unsupported type for double conversion");
    }
}

std::optional<std::shared_ptr<Value>> boolConverter(const std::vector<std::shared_ptr<Value>>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Bool conversion takes exactly one argument");
    }

    auto arg = args[0];

    if (std::holds_alternative<bool>(*arg)) {
        return arg; // If it's already a bool, return as is
    } else if (std::holds_alternative<int>(*arg)) {
        return std::make_shared<Value>(std::get<int>(*arg) != 0);
    } else if (std::holds_alternative<double>(*arg)) {
        return std::make_shared<Value>(std::get<double>(*arg) != 0.0);
    } else if (std::holds_alternative<std::string>(*arg)) {
        std::string strValue = std::get<std::string>(*arg);
        if (strValue.empty() || strValue == "false") {
            return std::make_shared<Value>(false);
        } else {
            return std::make_shared<Value>(true);
        }
    } else if (std::holds_alternative<std::shared_ptr<List>>(*arg)) {
        return std::make_shared<Value>(!std::get<std::shared_ptr<List>>(*arg)->empty());
    }else {
        throw std::runtime_error("Unsupported type for bool conversion");
    }
}

std::string toString(double value){
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

std::optional<std::shared_ptr<Value>> stringConverter(const std::vector<std::shared_ptr<Value>>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("String conversion takes exactly one argument");
    }

    auto arg = args[0];

    if (std::holds_alternative<std::string>(*arg)) {
        return arg; // If it's already a string, return as is
    } else if (std::holds_alternative<int>(*arg)) {
        return std::make_shared<Value>(std::to_string(std::get<int>(*arg)));
    } else if (std::holds_alternative<double>(*arg)) {
        return std::make_shared<Value>(toString(std::get<double>(*arg)));
    } else if (std::holds_alternative<bool>(*arg)) {
        return std::make_shared<Value>(std::get<bool>(*arg) ? "true" : "false");
    } else if (std::holds_alternative<std::shared_ptr<List>>(*arg)) {
        std::shared_ptr<List> list = std::get<std::shared_ptr<List>>(*arg);
        std::string result = "[";
        for (size_t i = 0; i < list->size(); ++i) {
            auto str_return = stringConverter({list->at(i)}).value();
            result += std::get<std::string>(*str_return);
            if (i < list->size() - 1) {
                result += ", ";
            }
        }
        result += "]";
        return std::make_shared<Value>(result);
    }else {
        throw std::runtime_error("Unsupported type for string conversion");
    }
}

std::optional<std::shared_ptr<Value>> listConverter(const std::vector<std::shared_ptr<Value>>& args) {
    auto list = std::make_shared<List>();

    if (args.size() == 1 && std::holds_alternative<std::string>(*args[0])) {
        // If there's one argument and it's a string, split it into characters
        std::string str = std::get<std::string>(*args[0]);
        for (char c : str) {
            list->push_back(std::make_shared<Value>(std::string(1, c)));
        }
    } else if (args.size() == 1 && std::holds_alternative<std::shared_ptr<List>>(*args[0])) {
        return args[0];
    } else {
        for (const auto& arg : args) {
            list->push_back(arg);
        }
    }

    return std::make_shared<Value>(list);
}

BuiltInFunctionReturn input(const std::vector<std::shared_ptr<Value>>& args) {
    if (args.size() > 1) {
        throw std::runtime_error("Input takes one argument.");
    }

    if (args.size() == 1) {
        printValue(args[0]);
    }
    if (std::holds_alternative<std::string>(*args[0])) {
        std::string in;
        std::getline(std::cin, in);
        return std::make_shared<Value>(in);
    } else {
        throw std::runtime_error("Input expected a string argument.");
    }
}

BuiltInFunctionReturn getType(const std::vector<std::shared_ptr<Value>>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Type takes exactly one argument.");
    }

    return std::make_shared<Value>(getValueType(args[0]));
}


///  MEMBER FUNCTIONS  ///


BuiltInFunctionReturn listSize(const std::vector<std::shared_ptr<Value>>& args) {
    if (args.size() != 1) {
        throw std::runtime_error("Size takes exactly 1 argument.");
    }

    if (std::holds_alternative<std::shared_ptr<List>>(*args[0])) {
        auto list = std::get<std::shared_ptr<List>>(*args[0]);
        int size = list->size();
        return std::make_shared<Value>(size);
    } else {
        throw std::runtime_error("Size expected a list but got a " + getValueStr(args[0]));
    }
    return std::nullopt;
}

BuiltInFunctionReturn listAppend(const std::vector<std::shared_ptr<Value>>& args) {
    if (args.size() != 2) {
        throw std::runtime_error("Append takes exactly 2 arguments.");
    }

    auto list = std::get<std::shared_ptr<List>>(*args[0]);
    list->push_back(args[1]);
    return std::nullopt;
}

BuiltInFunctionReturn listPop(const std::vector<std::shared_ptr<Value>>& args) {
    if (args.size() != 1 && args.size() != 2) {
        throw std::runtime_error("Pop takes at most 1 argument.");
    }

    int index;
    if (args.size() == 1) {
        index = -1;
    } else {
        if (!std::holds_alternative<int>(*args[1])) {
            throw std::runtime_error("Pop expected int.");
        }
        index = std::get<int>(*args[1]);
    }
    auto list = std::get<std::shared_ptr<List>>(*args[0]);
    int size = list->size();

    if (index >= size || index < size * -1) {
        throw std::runtime_error("List pop index out of range.");
    } else if (index < 0) {
        auto value = list->at(list->size() - -index);
        list->erase(list->end() - -index);
        return value;
    } else {
        auto value = list->at(index);
        list->erase(list->begin() + index);
        return value;
    }
}