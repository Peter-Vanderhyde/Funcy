#include "library.h"
#include "pch.h"
#include "global_context.h"
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


BuiltInFunctionReturn print(const List& args, Environment& env) {
    for (const auto& arg : args) {
        printValue(arg);
        std::cout << " ";
    }
    std::cout << std::endl;
    return std::nullopt;
}

BuiltInFunctionReturn intConverter(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("Int conversion takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
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

BuiltInFunctionReturn floatConverter(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("Float conversion takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
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

BuiltInFunctionReturn boolConverter(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("Bool conversion takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
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

BuiltInFunctionReturn stringConverter(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("String conversion takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
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
            auto str_return = stringConverter({list->at(i)}, env).value();
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

BuiltInFunctionReturn listConverter(const List& args, Environment& env) {
    auto list = std::make_shared<List>();

    if (args.size() == 1 && std::holds_alternative<std::string>(*args[0])) {
        // If there's one argument and it's a string, split it into characters
        std::string str = std::get<std::string>(*args[0]);
        for (char c : str) {
            list->push_back(std::make_shared<Value>(std::string(1, c)));
        }
    } else if (args.size() == 1 && std::holds_alternative<std::shared_ptr<List>>(*args[0])) {
        return args[0];
    } else if (args.size() == 0) {
        throw std::runtime_error("List conversion takes at least 1 argument. " + std::to_string(args.size()) + " were given.");
    } else {
        for (const auto& arg : args) {
            list->push_back(arg);
        }
    }

    return std::make_shared<Value>(list);
}

BuiltInFunctionReturn input(const List& args, Environment& env) {
    if (args.size() > 1) {
        throw std::runtime_error("Input takes 0-1 arguments. " + std::to_string(args.size()) + " were given.");
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

BuiltInFunctionReturn getType(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("Type takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
    }

    return std::make_shared<Value>(getValueType(args[0]));
}

BuiltInFunctionReturn range(const List& args, Environment& env) {
    if (args.size() < 1 || args.size() > 3) {
        throw std::runtime_error("Range takes 1-3 arguments. " + std::to_string(args.size()) + " were given.");
    }

    for (int i = 0; i < args.size(); i++) {
        if (!std::holds_alternative<int>(*args[i])) {
            throw std::runtime_error("Range expected int, not " + getValueStr(args[i]));
        }
    }

    int start, end, step;

    if (args.size() == 1) {
        end = std::get<int>(*args[0]);
        start = 0;
        step = 1;
    } else if (args.size() == 2) {
        start = std::get<int>(*args[0]);
        end = std::get<int>(*args[1]);
        step = 1;
    } else if (args.size() == 3) {
        start = std::get<int>(*args[0]);
        end = std::get<int>(*args[1]);
        step = std::get<int>(*args[2]);
        if (step == 0) {
            throw std::runtime_error("Range third argument must not be zero.");
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

BuiltInFunctionReturn map(const List& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("Map requires exactly 2 arguments: a function and a list.");
    }

    auto func = args[0];
    auto list_value = args[1];

    if (!std::holds_alternative<std::shared_ptr<List>>(*list_value)) {
        throw std::runtime_error("Second argument to map must be a list.");
    }

    auto list = std::get<std::shared_ptr<List>>(*list_value);
    std::shared_ptr<List> result_list = std::make_shared<List>();

    for (const auto& element : *list) {
        // Prepare the argument list for the function call
        List func_args = { element };

        // Check if the function is a built-in function
        if (std::holds_alternative<std::shared_ptr<BuiltInFunction>>(*func)) {
            auto built_in_func = std::get<std::shared_ptr<BuiltInFunction>>(*func);
            auto result = (*built_in_func)(func_args, env);
            if (result) {
                result_list->push_back(result.value());
            }
        } else if (std::holds_alternative<std::shared_ptr<ASTNode>>(*func)) {
            auto func_node = dynamic_cast<FuncNode*>(std::get<std::shared_ptr<ASTNode>>(*func).get());

            if (func_node) {
                auto result = func_node->callFunc(func_args, env);
                if (result) {
                    result_list->push_back(result.value());
                }
            } else {
                throw std::runtime_error("Map function argument must be a callable function.");
            }
        } else {
            throw std::runtime_error("First argument to map must be a function.");
        }
    }

    return std::make_shared<Value>(result_list);
}

BuiltInFunctionReturn all(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("All takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(*args[0])) {
        throw std::runtime_error("All expected argument 1 to be a list.");
    }

    auto list = std::get<std::shared_ptr<List>>(*args[0]);
    for (auto item : *list) {
        auto result = boolConverter(List{item}, env);
        if (result.has_value()) {
            bool bool_result = std::get<bool>(*result.value());
            if (!bool_result) {
                return std::make_shared<Value>(false);
            }
        }
    }
    return std::make_shared<Value>(true);
}

BuiltInFunctionReturn any(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("Any takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(*args[0])) {
        throw std::runtime_error("Any expected argument 1 to be a list.");
    }

    auto list = std::get<std::shared_ptr<List>>(*args[0]);
    for (auto item : *list) {
        auto result = boolConverter(List{item}, env);
        if (result.has_value()) {
            bool bool_result = std::get<bool>(*result.value());
            if (bool_result) {
                return std::make_shared<Value>(true);
            }
        }
    }
    return std::make_shared<Value>(false);
}


///  MEMBER FUNCTIONS  ///


BuiltInFunctionReturn listSize(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("Size takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
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

BuiltInFunctionReturn listAppend(const List& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("Append takes exactly 2 arguments. " + std::to_string(args.size()) + " were given.");
    }

    auto list = std::get<std::shared_ptr<List>>(*args[0]);
    list->push_back(args[1]);
    return std::nullopt;
}

BuiltInFunctionReturn listPop(const List& args, Environment& env) {
    if (args.size() != 1 && args.size() != 2) {
        throw std::runtime_error("Pop takes 1-2 argument. " + std::to_string(args.size()) + " were given.");
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

BuiltInFunctionReturn stringLower(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("String lower takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
    }

    std::string string = std::get<std::string>(*args[0]);
    std::transform(string.begin(), string.end(), string.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    
    return std::make_shared<Value>(string);
}

BuiltInFunctionReturn stringUpper(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("String upper takes exactly 1 argument. " + std::to_string(args.size()) + " were given.");
    }

    std::string string = std::get<std::string>(*args[0]);
    std::transform(string.begin(), string.end(), string.begin(),
                    [](unsigned char c) { return std::toupper(c); });
    
    return std::make_shared<Value>(string);
}

BuiltInFunctionReturn stringStrip(const List& args, Environment& env) {
    if (args.size() > 2) {
        throw std::runtime_error("String strip takes 1-2 arguments. " + std::to_string(args.size()) + " were given.");
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
        if (std::holds_alternative<std::string>(*args[1])) {
            strip_chars = std::get<std::string>(*args[1]);
        } else {
            throw std::runtime_error("String strip expected an argument of type string.");
        }
    }

    return std::make_shared<Value>(ltrim(rtrim(std::get<std::string>(*args[0]), strip_chars), strip_chars));
}

BuiltInFunctionReturn stringIsDigit(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("String isDigit requires exactly 1 argument. " + std::to_string(args.size()) + " were given.");
    }

    std::string string = std::get<std::string>(*args[0]);
    for (char c : string) {
        if (!std::isdigit(c)) {
            return std::make_shared<Value>(false);
        }
    }
    return std::make_shared<Value>(true);
}

BuiltInFunctionReturn stringLength(const List& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("String length requires exactly 1 argument. " + std::to_string(args.size()) + " were given.");
    }

    std::string string = std::get<std::string>(*args[0]);
    int length = string.length();
    return std::make_shared<Value>(length);
}