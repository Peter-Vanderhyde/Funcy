#include <library.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <format>
#include <iomanip>
#include <vector>
#include <memory>
#include <chrono>
#include <filesystem>
#include <cmath>
#include <cctype>
#include <random>
#include "errorDefs.h"
#include "values.h"
#include "nodes.h"
#include "context.h"
#include "parser.h"
#include "lexer.h"

std::string readSourceCodeFromFile(const std::string& filename) {
    if (filename.size() < 3 || filename.substr(filename.size() - 3) != ".fy") {
        runtimeError("File must have a .fy extension");
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

void printValue(const std::shared_ptr<Value> value, bool error) {
    Style style{};
    switch(value->getType()) {
        case ValueType::Integer: {
            int int_value = value->get<int>();
            if (error) {
                std::cout << style.red << int_value << style.reset;
            } else {
                std::cout << style.light_blue << int_value << style.reset;
            }
            return;
        }
        case ValueType::Float: {
            double float_value = value->get<double>();
            if (float_value == static_cast<int>(float_value)) {
                if (error) {
                    std::cout << style.red << float_value << ".0" << style.reset;
                } else {
                    std::cout << style.light_blue << float_value << ".0" << style.reset;
                }
            } else {
                if (error) {
                    std::cout << style.red << float_value << style.reset;
                } else {
                    std::cout << style.light_blue << float_value << style.reset;
                }
            }
            return;
        }
        case ValueType::Boolean: {
            bool bool_value = value->get<bool>();
            if (error) {
                std::cout << style.red << std::boolalpha << bool_value << style.reset;
            } else {
                std::cout << style.purple << std::boolalpha << bool_value << style.reset;
            }
            return;
        }
        case ValueType::String: {
            std::string string_value = value->get<std::string>();
            if (error) {
                std::cout << style.red << string_value << style.reset;
            } else {
                std::cout << style.green << "'" << string_value << "'" << style.reset;
            }
            return;
        }
        case ValueType::List: {
            std::shared_ptr<List> list_value = value->get<std::shared_ptr<List>>();
            if (error) {
                std::cout << style.red;
            }
            std::cout << "[";
            bool first = true;
            for (int i = 0; i < list_value->size(); i++) {
                auto item = list_value->at(i);
                if (!first) {
                    std::cout << ", ";
                } else {
                    first = false;
                }
                printValue(item, error);
            }
            if (error) {
                std::cout << style.red << "]" << style.reset;
            } else {
                std::cout << "]";
            }
            return;
        }
        case ValueType::Dictionary: {
            auto dict_value = value->get<std::shared_ptr<Dictionary>>();
            if (error) {
                std::cout << style.red;
            }
            std::cout << "{";
            bool first = true;
            for (const auto pair : *dict_value) {
                if (!first) {
                    std::cout << ", ";
                } else {
                    first = false;
                }
                printValue(pair.first, error);
                if (error) {
                    std::cout << style.red;
                }
                std::cout << ":";
                printValue(pair.second, error);
            }
            if (error) {
                std::cout << style.red << "}" << style.reset;
            } else {
                std::cout << "}";
            }
            return;
        }
        case ValueType::None: {
            if (error) {
                std::cout << style.red << "Null" << style.reset;
            } else {
                std::cout << style.blue << "Null" << style.reset;
            }
            return;
        }
        case ValueType::Function: {
            auto node = value->get<std::shared_ptr<ASTNode>>();
            auto func_node = std::static_pointer_cast<FuncNode>(node);
            if (error) {
                std::cout << style.red << "Function:" << *func_node->func_name << style.reset;
            } else {
                std::cout << style.blue << "Function:" << *func_node->func_name << style.reset;
            }
            return;
        }
        case ValueType::BuiltInFunction: {
            if (error) {
                std::cout << style.red << "Type:BuiltInFunction" << style.reset;
            } else {
                std::cout << style.blue << "Type:BuiltInFunction" << style.reset;
            }
            return;
        }
        case ValueType::Class: {
            auto node = value->get<std::shared_ptr<Class>>();
            if (error) {
                std::cout << style.red << "Class:" << node->getName() << style.reset;
            } else {
                std::cout << style.blue << "Class:" << node->getName() << style.reset;
            }
            return;
        }
        case ValueType::Instance: {
            auto node = value->get<std::shared_ptr<Instance>>();
            if (error) {
                std::cout << style.red << node->getClassName() << ":Instance" << style.reset;
            } else {
                std::cout << style.blue << node->getClassName() << ":Instance" << style.reset;
            }
            return;
        }
        case ValueType::Type: {
            if (error) {
                std::cout << style.red << getTypeStr(value->get<ValueType>()) << style.reset;
            } else {
                std::cout << style.blue << getTypeStr(value->get<ValueType>()) << style.reset;
            }
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
            first_num = first->get<bool>() ? 1 : 0;
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
            second_num = second->get<bool>() ? 1 : 0;
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

Environment buildStartingEnvironment() {
    Environment env;
    env.addScope();

    env.addFunction("abs", std::make_shared<Value>(std::make_shared<BuiltInFunction>(absoluteValue)));
    env.addFunction("all", std::make_shared<Value>(std::make_shared<BuiltInFunction>(all)));
    env.addFunction("any", std::make_shared<Value>(std::make_shared<BuiltInFunction>(any)));
    env.addFunction("appendFile", std::make_shared<Value>(std::make_shared<BuiltInFunction>(appendFile)));
    env.addFunction("bool", std::make_shared<Value>(std::make_shared<BuiltInFunction>(boolConverter)));
    env.addFunction("callable", std::make_shared<Value>(std::make_shared<BuiltInFunction>(callable)));
    env.addFunction("dict", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictConverter)));
    env.addFunction("divMod", std::make_shared<Value>(std::make_shared<BuiltInFunction>(divMod)));
    env.addFunction("enumerate", std::make_shared<Value>(std::make_shared<BuiltInFunction>(enumerate)));
    env.addFunction("float", std::make_shared<Value>(std::make_shared<BuiltInFunction>(floatConverter)));
    env.addFunction("globals", std::make_shared<Value>(std::make_shared<BuiltInFunction>(globals)));
    env.addFunction("input", std::make_shared<Value>(std::make_shared<BuiltInFunction>(input)));
    env.addFunction("int", std::make_shared<Value>(std::make_shared<BuiltInFunction>(intConverter)));
    env.addFunction("length", std::make_shared<Value>(std::make_shared<BuiltInFunction>(length)));
    env.addFunction("list", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listConverter)));
    env.addFunction("locals", std::make_shared<Value>(std::make_shared<BuiltInFunction>(locals)));
    env.addFunction("map", std::make_shared<Value>(std::make_shared<BuiltInFunction>(map)));
    env.addFunction("max", std::make_shared<Value>(std::make_shared<BuiltInFunction>(max)));
    env.addFunction("min", std::make_shared<Value>(std::make_shared<BuiltInFunction>(min)));
    env.addFunction("print", std::make_shared<Value>(std::make_shared<BuiltInFunction>(print)));
    env.addFunction("randChoice", std::make_shared<Value>(std::make_shared<BuiltInFunction>(randChoice)));
    env.addFunction("randInt", std::make_shared<Value>(std::make_shared<BuiltInFunction>(randInt)));
    env.addFunction("range", std::make_shared<Value>(std::make_shared<BuiltInFunction>(range)));
    env.addFunction("readFile", std::make_shared<Value>(std::make_shared<BuiltInFunction>(readFile)));
    env.addFunction("reversed", std::make_shared<Value>(std::make_shared<BuiltInFunction>(reversed)));
    env.addFunction("round", std::make_shared<Value>(std::make_shared<BuiltInFunction>(roundVal)));
    env.addFunction("str", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringConverter)));
    env.addFunction("sum", std::make_shared<Value>(std::make_shared<BuiltInFunction>(sum)));
    env.addFunction("time", std::make_shared<Value>(std::make_shared<BuiltInFunction>(currentTime)));
    env.addFunction("type", std::make_shared<Value>(std::make_shared<BuiltInFunction>(getType)));
    env.addFunction("writeFile", std::make_shared<Value>(std::make_shared<BuiltInFunction>(writeFile)));
    env.addFunction("zip", std::make_shared<Value>(std::make_shared<BuiltInFunction>(zip)));

    // ValueType::Float Members
    env.addMember(ValueType::Float, "isInt", std::make_shared<Value>(std::make_shared<BuiltInFunction>(floatIsInt)));

    // ValueType::List Members
    env.addMember(ValueType::List, "append", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listAppend)));
    env.addMember(ValueType::List, "clear", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listClear)));
    env.addMember(ValueType::List, "copy", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listCopy)));
    env.addMember(ValueType::List, "index", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listIndex)));
    env.addMember(ValueType::List, "insert", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listInsert)));
    env.addMember(ValueType::List, "pop", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listPop)));
    env.addMember(ValueType::List, "remove", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listRemove)));
    env.addMember(ValueType::List, "size", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listSize)));

    // ValueType::Dictionary Members
    env.addMember(ValueType::Dictionary, "clear", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictClear)));
    env.addMember(ValueType::Dictionary, "copy", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictCopy)));
    env.addMember(ValueType::Dictionary, "get", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictGet)));
    env.addMember(ValueType::Dictionary, "items", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictItems)));
    env.addMember(ValueType::Dictionary, "keys", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictKeys)));
    env.addMember(ValueType::Dictionary, "pop", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictPop)));
    env.addMember(ValueType::Dictionary, "setDefault", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictSetDefault)));
    env.addMember(ValueType::Dictionary, "size", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictSize)));
    env.addMember(ValueType::Dictionary, "update", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictUpdate)));
    env.addMember(ValueType::Dictionary, "values", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictValues)));

    // ValueType::String Members
    env.addMember(ValueType::String, "capitalize", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringCapitalize)));
    env.addMember(ValueType::String, "endsWith", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringEndsWith)));
    env.addMember(ValueType::String, "find", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringFind)));
    env.addMember(ValueType::String, "isAlpha", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringIsAlpha)));
    env.addMember(ValueType::String, "isAlphaNum", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringIsAlphaNum)));
    env.addMember(ValueType::String, "isDigit", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringIsDigit)));
    env.addMember(ValueType::String, "isSpace", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringIsSpace)));
    env.addMember(ValueType::String, "isWhitespace", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringIsWhitespace)));
    env.addMember(ValueType::String, "join", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringJoin)));
    env.addMember(ValueType::String, "length", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringLength)));
    env.addMember(ValueType::String, "lower", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringLower)));
    env.addMember(ValueType::String, "replace", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringReplace)));
    env.addMember(ValueType::String, "split", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringSplit)));
    env.addMember(ValueType::String, "strip", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringStrip)));
    env.addMember(ValueType::String, "toJson", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringToJson)));
    env.addMember(ValueType::String, "upper", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringUpper)));

    // ValueType::Instance Members
    env.addMember(ValueType::Instance, "delAttr", std::make_shared<Value>(std::make_shared<BuiltInFunction>(instanceDel)));
    env.addMember(ValueType::Instance, "getAttr", std::make_shared<Value>(std::make_shared<BuiltInFunction>(instanceGet)));
    env.addMember(ValueType::Instance, "hasAttr", std::make_shared<Value>(std::make_shared<BuiltInFunction>(instanceHas)));
    env.addMember(ValueType::Instance, "setAttr", std::make_shared<Value>(std::make_shared<BuiltInFunction>(instanceSet)));

    return env;
}


///  MEMBER FUNCTIONS  ///


BuiltInFunctionReturn absoluteValue(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("abs() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    switch (args[0]->getType()) {
        case ValueType::Integer:
            return std::make_shared<Value>(std::abs(args[0]->get<int>()));
        case ValueType::Float:
            return std::make_shared<Value>(std::abs(args[0]->get<double>()));
        default:
            throw std::runtime_error("abs() expected an argument of Type:Integer or Type:Float but got " + getTypeStr(args[0]->getType()));
    }
    return std::nullopt;
}

BuiltInFunctionReturn all(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("all() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::List) {
        throw std::runtime_error("all() expected an argument of Type:List but got " + getTypeStr(args[0]->getType()));
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    for (int i = 0; i < list->size(); i++) {
        auto item = list->at(i);
        auto result = boolConverter(std::vector<std::shared_ptr<Value>>{item}, env);
        if (result.has_value()) {
            bool bool_result = result.value()->get<bool>();
            if (!bool_result) {
                return std::make_shared<Value>(false);
            }
        }
    }
    return std::make_shared<Value>(true);
}

BuiltInFunctionReturn any(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("any() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::List) {
        throw std::runtime_error("any() expected an argument of Type:List but got " + getTypeStr(args[0]->getType()));
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    for (int i = 0; i < list->size(); i++) {
        auto item = list->at(i);
        auto result = boolConverter(std::vector<std::shared_ptr<Value>>{item}, env);
        if (result.has_value()) {
            bool bool_result = result.value()->get<bool>();
            if (bool_result) {
                return std::make_shared<Value>(true);
            }
        }
    }
    return std::make_shared<Value>(false);
}

BuiltInFunctionReturn appendFile(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("appendFile() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::String) {
        throw std::runtime_error("appendFile() expected an argument 1 of Type:String but got " + getTypeStr(args[0]->getType()));
    }

    if (args[1]->getType() != ValueType::String) {
        throw std::runtime_error("appendFile() expected an argument 2 of Type:String but got " + getTypeStr(args[1]->getType()));
    }

    std::shared_ptr<Value> filename = args[0];
    std::string contents_to_add = args[1]->get<std::string>();

    std::string orig_contents = "";
    try {
        auto contents = readFile(std::vector<std::shared_ptr<Value>>{filename}, env);
        if (contents) {
            orig_contents = contents.value()->get<std::string>();
        }
    }
    catch (const std::exception& e) {
    }

    auto new_contents = std::make_shared<Value>(orig_contents + contents_to_add);
    writeFile(std::vector<std::shared_ptr<Value>>{filename, new_contents}, env);
    return std::make_shared<Value>();
}

BuiltInFunctionReturn boolConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("bool() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
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
            throw std::runtime_error("Unsupported type for bool conversion: " + getTypeStr(arg->getType()));
    }
}

BuiltInFunctionReturn callable(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("callable() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto type = args[0]->getType();
    switch (type) {
        case ValueType::Function:
            return std::make_shared<Value>(true);
        case ValueType::BuiltInFunction:
            return std::make_shared<Value>(true);
        case ValueType::Class:
            return std::make_shared<Value>(true);
        default:
            return std::make_shared<Value>(false);
    }
}

BuiltInFunctionReturn currentTime(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    using namespace std::chrono;

    // Get the current time since epoch in milliseconds
    auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    // Return it as an int
    return std::make_shared<Value>(static_cast<int>(now));
}

BuiltInFunctionReturn dictConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    auto dict = std::make_shared<Dictionary>();

    if (args.size() != 1 && args.size() != 0) {
        throw std::runtime_error("dict() takes 0-1 argument" + std::to_string(args.size()) + " were given");
    }

    if (args.size() == 0) {
        return std::make_shared<Value>(dict);
    }

    auto arg = args[0];
    switch (arg->getType()) {
        case ValueType::Dictionary: {
            auto orig_dict = arg->get<std::shared_ptr<Dictionary>>();
            for (const auto& pair : *orig_dict) {
                dict->insert(pair);
            }
            return std::make_shared<Value>(dict); // Already a dictionary
        }
        case ValueType::List: {
            auto list = arg->get<std::shared_ptr<List>>();
            for (int i = 0; i < list->size(); i++) {
                auto element = list->at(i);
                if (element->getType() == ValueType::List) {
                    auto k_v = element->get<std::shared_ptr<List>>();
                    if (k_v->size() == 2) {
                        dict->insert(std::make_pair(k_v->at(0), k_v->at(1)));
                    } else {
                        throw std::runtime_error("Dictionary update sequence element #" + std::to_string(i + 1) + " has length "\
 + std::to_string(k_v->size()) + "; 2 is required");
                    }
                } else {
                    throw std::runtime_error("Dictionary conversion requires the list to contain "\
"lists holding exactly 2 elements each.");
                }
            }
        }
        default: {
            break;
        }
    }
    return std::make_shared<Value>(dict);
}

BuiltInFunctionReturn divMod(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("divMod() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::Integer) {
        throw std::runtime_error("divMod() expected an argument 1 of Type:Integer but got " + getTypeStr(args[0]->getType()));
    }
    if (args[1]->getType() != ValueType::Integer) {
        throw std::runtime_error("divMod() expected an argument 2 of Type:Integer but got " + getTypeStr(args[1]->getType()));
    }

    int dividend = args[0]->get<int>();
    int divisor = args[1]->get<int>();

    int result = dividend / divisor;
    int remainder = dividend % divisor;

    List list;
    list.push_back(std::make_shared<Value>(result));
    list.push_back(std::make_shared<Value>(remainder));
    return std::make_shared<Value>(std::make_shared<List>(list));
}

BuiltInFunctionReturn enumerate(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() < 1 || args.size() > 2) {
        throw std::runtime_error("enumerate() takes 1-2 arguments. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::List) {
        throw std::runtime_error("enumerate() expected an argument 1 of Type:List but got " + getTypeStr(args[0]->getType()));
    }
    int start = 0;
    if (args.size() == 2) {
        if (args[1]->getType() != ValueType::Integer) {
            throw std::runtime_error("enumerate() expected an argument 2 of Type:Integer but got " + getTypeStr(args[1]->getType()));
        }
        start = args[1]->get<int>();
    }

    auto list = args[0]->get<std::shared_ptr<List>>();

    auto result = std::make_shared<List>();
    for (int i = 0; i < list->size(); i++) {
        auto group = std::make_shared<List>();
        group->push_back(std::make_shared<Value>(i + start));
        group->push_back(list->at(i));
        result->push_back(std::make_shared<Value>(group));
    }
    return std::make_shared<Value>(result);
}

BuiltInFunctionReturn floatConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("float() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
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
                throw std::runtime_error("Cannot convert Type:String to Type:Float");
            } catch (const std::out_of_range&) {
                throw std::runtime_error("String value out of range for float conversion");
            }
        }
        case ValueType::Boolean:
            return std::make_shared<Value>(arg->get<bool>() ? 1.0 : 0.0);
        default:
            throw std::runtime_error("Unsupported type for float conversion: " + getTypeStr(arg->getType()));
    }
}

BuiltInFunctionReturn getType(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("type() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    return std::make_shared<Value>(args[0]->getType());
}

BuiltInFunctionReturn globals(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 0) {
        throw std::runtime_error("globals() takes 0 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto global_scope = env.copyScopes().at(0);
    Dictionary dict;
    for (const auto& pair : global_scope.getPairs()) {
        dict[std::make_shared<Value>(pair.first)] = pair.second;
    }
    return std::make_shared<Value>(std::make_shared<Dictionary>(dict));
}

BuiltInFunctionReturn input(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() > 1) {
        throw std::runtime_error("input() takes 0-1 arguments. " + std::to_string(args.size()) + " were given");
    }

    if (args.size() == 1) {
        if (args[0]->getType() != ValueType::String) {
            printValue(args[0]);
        }
        else {
            std::string s = args[0]->get<std::string>();
            std::cout << s;
        }
    }
    std::string in;
    std::getline(std::cin, in);
    return std::make_shared<Value>(in);
}

BuiltInFunctionReturn intConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("int() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
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
                throw std::runtime_error("Cannot convert Type:String to Type:Integer");
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
            throw std::runtime_error("Unsupported type for int conversion: " + getTypeStr(arg->getType()));
    }
}

BuiltInFunctionReturn length(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("length() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto value = args[0];
    ValueType type = value->getType();
    if (type == ValueType::String) {
        return std::make_shared<Value>(static_cast<int>(value->get<std::string>().length()));
    } else if (type == ValueType::List) {
        return std::make_shared<Value>(static_cast<int>(value->get<std::shared_ptr<List>>()->size()));
    } else if (type == ValueType::Dictionary) {
        return std::make_shared<Value>(static_cast<int>(value->get<std::shared_ptr<Dictionary>>()->size()));
    } else {
        throw std::runtime_error("Object of " + getTypeStr(value->getType()) + " has no length");
    }
    return std::nullopt;
}

BuiltInFunctionReturn listConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    auto list = std::make_shared<List>();

    if (args.size() == 0) {
        return std::make_shared<Value>(std::make_shared<List>());
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
        case ValueType::List: {
            auto orig_l = arg->get<std::shared_ptr<List>>();
            for (int i = 0; i < orig_l->size(); i++) {
                list->push_back(orig_l->at(i));
            }
            return std::make_shared<Value>(list); // Already a list
        }
        case ValueType::Dictionary: {
            auto dict = arg->get<std::shared_ptr<Dictionary>>();
            std::vector<std::shared_ptr<Value>> values;
            values.push_back(std::make_shared<Value>(dict));
            return dictKeys(values, env);
        }
        default: {
            for (const auto& element : args) {
                list->push_back(element);
            }
            break;
        }
    }
    return std::make_shared<Value>(list);
}

BuiltInFunctionReturn locals(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 0) {
        throw std::runtime_error("locals() takes 0 arguments. " + std::to_string(args.size()) + " were given");
    }

    Dictionary dict;
    for (const auto& pair : env.getScope().getPairs()) {
        dict[std::make_shared<Value>(pair.first)] = pair.second;
    }
    return std::make_shared<Value>(std::make_shared<Dictionary>(dict));
}

BuiltInFunctionReturn map(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("map() requires exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto func = args[0];
    auto list_value = args[1];

    if (func->getType() != ValueType::Function && func->getType() != ValueType::BuiltInFunction) {
        throw std::runtime_error("map() expected an argument 1 of Type:Function or Type:BuiltInFunction but got " + getTypeStr(func->getType()));
    }

    if (list_value->getType() != ValueType::List) {
        throw std::runtime_error("map() expected an argument 2 of Type:List but got " + getTypeStr(list_value->getType()));
    }

    auto list = list_value->get<std::shared_ptr<List>>();
    std::shared_ptr<List> result_list = std::make_shared<List>();

    for (int i = 0; i < list->size(); i++) {
        const auto& element = list->at(i);
        // Prepare the argument list for the function call
        std::vector<std::shared_ptr<Value>> func_args = { element };

        // Check if the function is a built-in function
        if (func->getType() == ValueType::BuiltInFunction) {
            auto built_in_func = func->get<std::shared_ptr<BuiltInFunction>>();
            auto result = (*built_in_func)(func_args, env);
            if (result) {
                result_list->push_back(result.value());
            }
        } else if (func->getType() == ValueType::Function) {
            auto func_node = std::dynamic_pointer_cast<FuncNode>(func->get<std::shared_ptr<ASTNode>>());

            if (func_node) {
                auto result = func_node->callFunc(func_args, std::map<std::string, std::shared_ptr<Value>>{}, env);
                if (result) {
                    result_list->push_back(result.value());
                }
            } else {
                throw std::runtime_error("map() argument 1 must be a callable function");
            }
        }
    }

    return std::make_shared<Value>(result_list);
}

BuiltInFunctionReturn max(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() == 0) {
        throw std::runtime_error("max() takes 1 or more arguments. 0 were given");
    }

    std::shared_ptr<List> list;
    if (args.size() == 1) {
        if (args[0]->getType() != ValueType::List) {
            throw std::runtime_error("max() expected an argument of Type:List but got " + getTypeStr(args[0]->getType()));
        }
        list = args[0]->get<std::shared_ptr<List>>();
    } else {
        List arg_list;
        for (const auto& arg : args) {
            arg_list.push_back(arg);
        }
        list = std::make_shared<List>(arg_list);
    }
        
    if (list->empty()) {
        throw std::runtime_error("max() argument is an empty sequence");
    }
    auto first_type = list->at(0)->getType();

    // Ensure all elements are comparable
    for (const auto& item : list->getElements()) {
        auto item_type = item->getType();
        if (!(item_type == first_type || 
              (item_type == ValueType::Integer && first_type == ValueType::Float) || 
              (item_type == ValueType::Float && first_type == ValueType::Integer))) {
            throw std::runtime_error("max() '>' comparison is not supported between " + getTypeStr(first_type) +
                                     " and " + getTypeStr(item_type));
        }
    }

    int max_index = 0;
    for (int i = 1; i < list->size(); i++) {
        auto current = list->at(i);
        auto max_value = list->at(max_index);

        if (first_type == ValueType::Integer || first_type == ValueType::Float) {
            // Convert to double for comparison if mixing Integer and Float
            double current_value = (current->getType() == ValueType::Integer) 
                                   ? static_cast<double>(current->get<int>()) 
                                   : current->get<double>();

            double max_value_num = (max_value->getType() == ValueType::Integer) 
                                   ? static_cast<double>(max_value->get<int>()) 
                                   : max_value->get<double>();

            if (current_value > max_value_num) {
                max_index = i;
            }
        } else if (first_type == ValueType::String) {
            if (current->get<std::string>() > max_value->get<std::string>()) {
                max_index = i;
            }
        } else if (first_type == ValueType::List) {
            int current_size = current->get<std::shared_ptr<List>>()->size();
            int max_size = max_value->get<std::shared_ptr<List>>()->size();
            if (current_size > max_size) {
                max_index = i;
            }
        } else if (first_type == ValueType::Dictionary) {
            int current_size = current->get<std::shared_ptr<Dictionary>>()->size();
            int max_size = max_value->get<std::shared_ptr<Dictionary>>()->size();
            if (current_size > max_size) {
                max_index = i;
            }
        }
    }

    return list->at(max_index);
}

BuiltInFunctionReturn min(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() == 0) {
        throw std::runtime_error("min() takes 1 or more arguments. 0 were given");
    }

    std::shared_ptr<List> list;
    if (args.size() == 1) {
        if (args[0]->getType() != ValueType::List) {
            throw std::runtime_error("min() expected an argument of Type:List but got " + getTypeStr(args[0]->getType()));
        }
        list = args[0]->get<std::shared_ptr<List>>();
    } else {
        List arg_list;
        for (const auto& arg : args) {
            arg_list.push_back(arg);
        }
        list = std::make_shared<List>(arg_list);
    }
        
    if (list->empty()) {
        throw std::runtime_error("min() argument is an empty sequence");
    }
    auto first_type = list->at(0)->getType();

    // Ensure all elements are comparable
    for (const auto& item : list->getElements()) {
        auto item_type = item->getType();
        if (!(item_type == first_type || 
              (item_type == ValueType::Integer && first_type == ValueType::Float) || 
              (item_type == ValueType::Float && first_type == ValueType::Integer))) {
            throw std::runtime_error("min() '>' comparison is not supported between " + getTypeStr(first_type) +
                                     " and " + getTypeStr(item_type));
        }
    }

    int min_index = 0;
    for (int i = 1; i < list->size(); i++) {
        auto current = list->at(i);
        auto max_value = list->at(min_index);

        if (first_type == ValueType::Integer || first_type == ValueType::Float) {
            // Convert to double for comparison if mixing Integer and Float
            double current_value = (current->getType() == ValueType::Integer) 
                                   ? static_cast<double>(current->get<int>()) 
                                   : current->get<double>();

            double max_value_num = (max_value->getType() == ValueType::Integer) 
                                   ? static_cast<double>(max_value->get<int>()) 
                                   : max_value->get<double>();

            if (current_value < max_value_num) {
                min_index = i;
            }
        } else if (first_type == ValueType::String) {
            if (current->get<std::string>() < max_value->get<std::string>()) {
                min_index = i;
            }
        } else if (first_type == ValueType::List) {
            int current_size = current->get<std::shared_ptr<List>>()->size();
            int max_size = max_value->get<std::shared_ptr<List>>()->size();
            if (current_size < max_size) {
                min_index = i;
            }
        } else if (first_type == ValueType::Dictionary) {
            int current_size = current->get<std::shared_ptr<Dictionary>>()->size();
            int max_size = max_value->get<std::shared_ptr<Dictionary>>()->size();
            if (current_size < max_size) {
                min_index = i;
            }
        }
    }

    return list->at(min_index);
}

BuiltInFunctionReturn print(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    for (const auto& arg : args) {
        printValue(arg);
        std::cout << " ";
    }
    std::cout << std::endl;
    return std::nullopt;
}

BuiltInFunctionReturn randChoice(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("randChoice() takes exactly 1 arguments. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::List) {
        throw std::runtime_error("randChoice() expected an argument 1 of Type:List but got " + getTypeStr(args[0]->getType()));
    }

    auto list = args[0]->get<std::shared_ptr<List>>();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, list->size() - 1);

    int index = dist(gen);
    return list->at(index);
}

BuiltInFunctionReturn randInt(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("randInt() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::Integer) {
        throw std::runtime_error("randInt() expected an argument 1 of Type:Integer but got " + getTypeStr(args[0]->getType()));
    }
    if (args[1]->getType() != ValueType::Integer) {
        throw std::runtime_error("randInt() expected an argument 2 of Type:Integer but got " + getTypeStr(args[1]->getType()));
    }

    int min = args[0]->get<int>();
    int max = args[1]->get<int>();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(min, max);

    return std::make_shared<Value>(dist(gen));
}

BuiltInFunctionReturn range(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() < 1 || args.size() > 3) {
        throw std::runtime_error("range() takes 1-3 arguments. " + std::to_string(args.size()) + " were given");
    }

    for (int i = 0; i < args.size(); i++) {
        if (args[i]->getType() != ValueType::Integer) {
            throw std::runtime_error("range() expected an argument " + std::to_string(i) + " of Type:Integer but got " + getTypeStr(args[i]->getType()));
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
            throw std::runtime_error("range() does not allow argument 3 to be zero");
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

BuiltInFunctionReturn readFile(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("read() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::String) {
        throw std::runtime_error("read() expected an argument of Type:String but got " + getTypeStr(args[0]->getType()));
    }

    std::string new_path;

    std::string file_path = args[0]->get<std::string>();
    if (!std::filesystem::path(file_path).is_absolute()) {
        std::string path = currentExecutionContext();
        new_path = path.substr(0, path.find_last_of('/')) + "/" + file_path;
    } else {
        new_path = file_path;
    }

    std::ifstream file(new_path);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + new_path);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return std::make_shared<Value>(buffer.str());
}

BuiltInFunctionReturn reversed(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("reversed() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::List) {
        throw std::runtime_error("reversed() expected an argument of Type:List but got " + getTypeStr(args[0]->getType()));
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    List new_list;

    for (auto item : list->getElements()) {
        new_list.insert(0, item);
    }

    return std::make_shared<Value>(std::make_shared<List>(new_list));
}

BuiltInFunctionReturn roundVal(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() < 1 || args.size() > 2) {
        throw std::runtime_error("round() takes 1-2 arguments. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::Integer && args[0]->getType() != ValueType::Float) {
        throw std::runtime_error("round() expected an argument 1 of Type:Integer or Type:Float but got " + getTypeStr(args[0]->getType()));
    }

    int precision = 0; // Default precision
    if (args.size() == 2) {
        if (args[1]->getType() != ValueType::Integer) {
            throw std::runtime_error("round() expected an argument 2 of Type:Integer but got " + getTypeStr(args[1]->getType()));
        }
        precision = args[1]->get<int>();
    }

    if (args[0]->getType() == ValueType::Integer) {
        int num = args[0]->get<int>();
        if (precision > 0) {
            throw std::runtime_error("round() cannot apply precision to an integer");
        }
        return std::make_shared<Value>(num); // Integers do not require rounding
    }

    double num = args[0]->get<double>();
    double factor = std::pow(10.0, precision);
    num = std::round(num * factor) / factor;

    return std::make_shared<Value>(num);
}

std::string toString(double value){
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

BuiltInFunctionReturn stringConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("string() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto arg = args[0];

    switch (arg->getType()) {
        case ValueType::String:
            return arg; // Already a string
        case ValueType::Integer:
            return std::make_shared<Value>(std::to_string(arg->get<int>()));
        case ValueType::Float:
            return std::make_shared<Value>(std::to_string(arg->get<double>()));
        case ValueType::Boolean: {
            auto result = std::make_shared<Value>(std::string(arg->get<bool>() ? "true" : "false"));
            return result;
        }
        case ValueType::List: {
            std::shared_ptr<List> list = arg->get<std::shared_ptr<List>>();
            std::string result = "[";
            for (size_t i = 0; i < list->size(); ++i) {
                auto str_return = stringConverter({list->at(i)}, env).value();
                if (list->at(i)->getType() == ValueType::String) {
                    result += '"' + str_return->get<std::string>() + '"';
                } else {
                    result += str_return->get<std::string>();
                }
                if (i < list->size() - 1) {
                    result += ", ";
                }
            }
            result += "]";
            return std::make_shared<Value>(result);
        }
        case ValueType::Dictionary: {
            auto dict = arg->get<std::shared_ptr<Dictionary>>();
            std::string result = "{";
            bool first = true;
            for (const auto& pair : *dict) {
                if (!first) {
                    result += ", ";
                }
                first = false;
                auto key_str = stringConverter({pair.first}, env).value();
                auto value_str = stringConverter({pair.second}, env).value();
                std::string key_representation;
                if (pair.first->getType() == ValueType::String) {
                    key_representation = '"' + key_str->get<std::string>() + '"';
                } else {
                    key_representation = key_str->get<std::string>();
                }

                std::string value_representation;
                if (pair.second->getType() == ValueType::String) {
                    value_representation = '"' + value_str->get<std::string>() + '"';
                } else {
                    value_representation = value_str->get<std::string>();
                }

                result += key_representation + ": " + value_representation;
            }
            result += "}";
            return std::make_shared<Value>(result);
        }
        default:
            throw std::runtime_error("Unsupported type for string conversion");
    }
}

BuiltInFunctionReturn sum(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("sum() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::List) {
        throw std::runtime_error("sum() expected an argument of Type:List but got " + getTypeStr(args[0]->getType()));
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    bool not_ints = false;
    double summation = 0.0;
    for (auto num : list->getElements()) {
        if (num->getType() == ValueType::Integer) {
            double cast = static_cast<double>(num->get<int>());
            summation += cast;
        } else if (num->getType() == ValueType::Float) {
            not_ints = true;
            summation += num->get<double>();
        } else {
            throw std::runtime_error("Unsupported operation: Type:Float '+' " + getTypeStr(num->getType()));
        }
    }

    if (not_ints) {
        return std::make_shared<Value>(summation);
    } else {
        return std::make_shared<Value>(static_cast<int>(summation));
    }
}

BuiltInFunctionReturn writeFile(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("write() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    if (args[0]->getType() != ValueType::String) {
        throw std::runtime_error("write() expected an argument 1 of Type:String but got " + getTypeStr(args[0]->getType()));
    }
    if (args[1]->getType() != ValueType::String) {
        throw std::runtime_error("write() expected an argument 2 of Type:String but got " + getTypeStr(args[1]->getType()));
    }

    std::string new_path;
    std::string file_path = args[0]->get<std::string>();
    std::string content = args[1]->get<std::string>();

    if (!std::filesystem::path(file_path).is_absolute()) {
        std::string path = currentExecutionContext(); // Fetch base execution context
        new_path = path.substr(0, path.find_last_of('/')) + "/" + file_path;
    } else {
        new_path = file_path;
    }

    std::ofstream file(new_path, std::ios::out);
    if (!file) {
        throw std::runtime_error("Failed to open file for writing: " + new_path);
    }

    file << content;
    file.close();

    return std::make_shared<Value>();
}

BuiltInFunctionReturn zip(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() < 2) {
        throw std::runtime_error("input() takes 2 or more arguments. " + std::to_string(args.size()) + " were given");
    }

    int min_size = args[0]->get<std::shared_ptr<List>>()->size();

    int i = 0;
    for (auto arg : args) {
        if (arg->getType() != ValueType::List) {
            throw std::runtime_error("zip() expected an argument " + std::to_string(i) + " of Type:List but got " + getTypeStr(arg->getType()));
        }
        int size = arg->get<std::shared_ptr<List>>()->size();
        if (size < min_size) {
            min_size = size;
        }
        i += 1;
    }

    std::shared_ptr<List> result = std::make_shared<List>();
    for (int i = 0; i < min_size; i++) {
        std::shared_ptr<List> group = std::make_shared<List>();
        for (auto arg : args) {
            group->push_back(arg->get<std::shared_ptr<List>>()->at(i));
        }
        result->push_back(std::make_shared<Value>(group));
    }
    return std::make_shared<Value>(result);
}


///  TYPE MEMBER FUNCTIONS  ///


BuiltInFunctionReturn floatIsInt(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("isInt() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    double num = args[0]->get<double>();
    if (num == static_cast<int>(num)) {
        return std::make_shared<Value>(true);
    } else {
        return std::make_shared<Value>(false);
    }
}


BuiltInFunctionReturn listAppend(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("append() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    if (args[1]->getType() == ValueType::List) {
        auto orig = args[1]->get<std::shared_ptr<List>>();
        auto copy = std::make_shared<List>(*orig);
        list->push_back(std::make_shared<Value>(copy));
    }
    else if (args[1]->getType() == ValueType::Dictionary) {
        auto orig = args[1]->get<std::shared_ptr<Dictionary>>();
        auto copy = std::make_shared<Dictionary>(*orig);
        list->push_back(std::make_shared<Value>(copy));
    }
    else {
        list->push_back(args[1]);
    }
    return std::make_shared<Value>();
}

BuiltInFunctionReturn listClear(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("clear() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    list->clear();
    return std::make_shared<Value>();
}

BuiltInFunctionReturn listCopy(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("copy() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    return std::make_shared<Value>(std::make_shared<List>(list->getElements()));
}

BuiltInFunctionReturn listIndex(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() < 2 || args.size() > 4) {
        throw std::runtime_error("index() takes 2-4 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    auto value = args[1];
    auto start = 0;
    auto end = list->size();
    if (args.size() >= 3) {
        if (args[2]->getType() != ValueType::Integer) {
            throw std::runtime_error("index() expected an argument 2 of Type:Integer but got " + getTypeStr(args[2]->getType()));
        }
        start = args[2]->get<int>();
    }
    if (args.size() == 4) {
        if (args[3]->getType() != ValueType::Integer) {
            throw std::runtime_error("index() expected an argument 3 of Type:Integer but got " + getTypeStr(args[3]->getType()));
        }
        end = args[3]->get<int>();
    }

    if (start < 0) {
        start = list->size() - -start;
        if (start < 0) {
            throw std::runtime_error("Invalid start index");
        }
    }
    if (end < 0) {
        end = list->size() - -end;
        if (end < 0) {
            throw std::runtime_error("Invalid end index");
        }
    }
    if (start > end) {
        throw std::runtime_error("index() start index was greater than end index");
    }

    int index = list->index(value, start, end);
    return std::make_shared<Value>(index);
}

BuiltInFunctionReturn listInsert(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 3) {
        throw std::runtime_error("insert() takes exactly 3 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    if (args[1]->getType() != ValueType::Integer) {
        throw std::runtime_error("insert() expected an argument 1 of Type:Integer but got " + getTypeStr(args[1]->getType()));
    }
    auto index = args[1]->get<int>();
    auto value = args[2];

    list->insert(index, value);
    return std::make_shared<Value>();
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
            throw std::runtime_error("pop() expected an argument of Type:Integer but got " + getValueStr(args[1]));
        }
        index = args[1]->get<int>();
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    int size = list->size();

    if (index >= size || index < size * -1) {
        throw std::runtime_error("pop() index out of range");
    } else {
        return list->pop(index);
    }
}

BuiltInFunctionReturn listRemove(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("remove() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    auto value = args[1];

    
    list->erase(value);
    return std::make_shared<Value>();
}

BuiltInFunctionReturn listSize(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("size() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto list = args[0]->get<std::shared_ptr<List>>();
    int size = list->size();
    return std::make_shared<Value>(size);
    return std::nullopt;
}


BuiltInFunctionReturn dictClear(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("clear() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    dict->clear();
    return std::make_shared<Value>();
}

BuiltInFunctionReturn dictCopy(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("clear() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    Dictionary copy{*dict};
    return std::make_shared<Value>(std::make_shared<Dictionary>(copy));
}

BuiltInFunctionReturn dictValues(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("values() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    auto result = std::make_shared<List>();

    for (const auto& pair : *dict) {
        result->push_back(pair.second);
    }

    return std::make_shared<Value>(result);
}

BuiltInFunctionReturn dictGet(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() < 2 || args.size() > 3) {
        throw std::runtime_error("get() takes 2-3 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    auto key = args[1];
    std::shared_ptr<Value> default_return;
    if (args.size() == 3) {
        default_return = args[2];
    }

    auto it = dict->find(key);
    if (it != dict->end()) {
        return it->second;
    } else {
        if (default_return) {
            return default_return;
        } else {
            return std::make_shared<Value>();
        }
    }
}

BuiltInFunctionReturn dictItems(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("items() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    auto result = std::make_shared<List>();

    for (const auto& pair : *dict) {
        auto key_value_pair = std::make_shared<List>();
        key_value_pair->push_back(pair.first);
        key_value_pair->push_back(pair.second);
        result->push_back(std::make_shared<Value>(key_value_pair));
    }

    return std::make_shared<Value>(result);
}

BuiltInFunctionReturn dictKeys(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("keys() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    auto result = std::make_shared<List>();

    for (const auto& pair : *dict) {
        result->push_back(pair.first);
    }

    return std::make_shared<Value>(result);
}

BuiltInFunctionReturn dictPop(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() < 2 || args.size() > 3) {
        throw std::runtime_error("pop() takes 2-3 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    auto key = args[1];

    auto it = dict->find(key);
    if (it != dict->end()) {
        auto value = it->second;
        dict->erase(it);
        return value;
    } else {
        if (args.size() == 3) {
            return args[2];
        } else {
            throw std::runtime_error("Key not found in dictionary");
        }
    }
}

BuiltInFunctionReturn dictSetDefault(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() < 2 || args.size() > 3) {
        throw std::runtime_error("setDefault() takes 2-3 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    auto key = args[1];
    std::shared_ptr<Value> default_val = std::make_shared<Value>();
    if (args.size() == 3) {
        default_val = args[2];
    }

    auto it = dict->find(key);
    if (it != dict->end()) {
        return it->second;
    } else {
        (*dict)[key] = default_val;
        return default_val;
    }
}

BuiltInFunctionReturn dictSize(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("size() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    int size = dict->size();
    return std::make_shared<Value>(size);
}

BuiltInFunctionReturn dictUpdate(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("update() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto dict = args[0]->get<std::shared_ptr<Dictionary>>();
    std::shared_ptr<Dictionary> other_dict;
    if (args[1]->getType() != ValueType::Dictionary) {
        throw std::runtime_error("update() expected an argument of Type:Dictionary but got " + getTypeStr(args[1]->getType()));
    }
    other_dict = args[1]->get<std::shared_ptr<Dictionary>>();

    for (const auto& pair : *other_dict) {
        (*dict)[pair.first] = pair.second;
    }

    return std::make_shared<Value>();
}


BuiltInFunctionReturn stringCapitalize(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("capitalize() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    if (string.empty()) {
        return args[0];
    }
    std::transform(string.begin(), string.begin() + 1, string.begin(),
                    [](unsigned char c){ return std::toupper(c); });
    if (string.length() == 1) {
        return std::make_shared<Value>(string);
    }
    std::transform(string.begin() + 1, string.end(), string.begin() + 1,
                    [](unsigned char c){ return std::tolower(c); });
    return std::make_shared<Value>(string);
}

BuiltInFunctionReturn stringEndsWith(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("endsWith() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    if (args[1]->getType() != ValueType::String) {
        throw std::runtime_error("endsWith() expects an argument 1 of Type:String but got " + getTypeStr(args[1]->getType()));
    }
    std::string substr = args[1]->get<std::string>();
    if (string.length() < substr.length()) {
        return std::make_shared<Value>(false);
    }
    int string_start_index = string.length() - substr.length();

    for (int i = 0; i < substr.length(); i++) {
        if (string.at(string_start_index + i) != substr.at(i)) {
            return std::make_shared<Value>(false);
        }
    }
    return std::make_shared<Value>(true);
}

BuiltInFunctionReturn stringFind(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("find() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    if (args[1]->getType() != ValueType::String) {
        throw std::runtime_error("find() expected an argument 1 of Type:String but got " + getTypeStr(args[1]->getType()));
    }
    std::string substr = args[1]->get<std::string>();

    if (substr.empty()) {
        return std::make_shared<Value>(0);
    }

    int index = string.find(substr);
    if (index == std::string::npos) {
        return std::make_shared<Value>(-1);
    }
    return std::make_shared<Value>(index);
}

BuiltInFunctionReturn stringIsAlpha(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("isAlpha() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    bool result = std::all_of(string.begin(), string.end(), [](unsigned char c){ return isalpha(c); });
    return std::make_shared<Value>(result);
}

BuiltInFunctionReturn stringIsAlphaNum(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("isAlphaNum() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    bool result = std::all_of(string.begin(), string.end(), [](unsigned char c){ return isalnum(c); });
    return std::make_shared<Value>(result);
}

BuiltInFunctionReturn stringIsDigit(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("isDigit() requires exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    if (string.size() == 0) {
        return std::make_shared<Value>(false);
    }

    for (char c : string) {
        if (!std::isdigit(c)) {
            return std::make_shared<Value>(false);
        }
    }
    return std::make_shared<Value>(true);
}

BuiltInFunctionReturn stringIsSpace(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("isSpace() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    bool result = std::all_of(string.begin(), string.end(), [](unsigned char c){ return isspace(c); });
    return std::make_shared<Value>(result);
}

BuiltInFunctionReturn stringIsWhitespace(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("isWhitespace() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    bool result = std::all_of(string.begin(), string.end(), [](unsigned char c){ return iswspace(c); });
    return std::make_shared<Value>(result);
}

BuiltInFunctionReturn stringJoin(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("join() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    std::string joiner = args[0]->get<std::string>();

    if (args[1]->getType() != ValueType::List) {
        throw std::runtime_error("join() expected an argument of Type:List but got " + getTypeStr(args[1]->getType()));
    }
    auto segments = args[1]->get<std::shared_ptr<List>>();
    std::string combined = "";
    for (int i = 0; i < segments->size(); i++) {
        if (i != 0) {
            combined += joiner;
        }
        if (segments->at(i)->getType() != ValueType::String) {
            throw std::runtime_error("join() expected a list of string elements, but got " + getValueStr(segments->at(i)));
        }
        combined += segments->at(i)->get<std::string>();
    }

    return std::make_shared<Value>(combined);
}

BuiltInFunctionReturn stringLength(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("length() requires exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    int length = string.length();
    return std::make_shared<Value>(length);
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

BuiltInFunctionReturn stringReplace(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 3) {
        throw std::runtime_error("replace() takes exactly 3 arguments. " + std::to_string(args.size()) + " were given");
    }

    // Get the string to modify
    std::string str = args[0]->get<std::string>();

    // Get the substring to replace
    if (args[1]->getType() != ValueType::String) {
        throw std::runtime_error("replace() expected an argument 1 of Type:String but got " + getTypeStr(args[1]->getType()));
    }
    std::string to_replace = args[1]->get<std::string>();

    // Get the replacement substring
    if (args[2]->getType() != ValueType::String) {
        throw std::runtime_error("replace() expected an argument 2 of Type:String but got " + getTypeStr(args[2]->getType()));
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

BuiltInFunctionReturn stringSplit(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() > 2) {
        throw std::runtime_error("split() takes 1-2 arguments. " + std::to_string(args.size()) + " were given");
    }

    std::string str = args[0]->get<std::string>();

    std::string delimiter = " "; // Default delimiter is space
    if (args.size() == 2) {
        if (args[1]->getType() == ValueType::String) {
            delimiter = args[1]->get<std::string>();
        } else {
            throw std::runtime_error("split() expected an argument 1 of Type:String but got " + getTypeStr(args[1]->getType()));
        }
    }

    List result;
    size_t pos = 0;
    std::string token;
    while ((pos = str.find(delimiter)) != std::string::npos) {
        token = str.substr(0, pos);
        if (token != "") {
            result.push_back(std::make_shared<Value>(token));
        }
        str.erase(0, pos + delimiter.length());
    }
    if (str != "") {
        result.push_back(std::make_shared<Value>(str)); // Add the last token
    }

    return std::make_shared<Value>(std::make_shared<List>(result));
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
            throw std::runtime_error("strip() expected an argument of Type:String but got " + getTypeStr(args[1]->getType()));
        }
    }

    return std::make_shared<Value>(ltrim(rtrim(args[0]->get<std::string>(), strip_chars), strip_chars));
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

BuiltInFunctionReturn stringToJson(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 1) {
        throw std::runtime_error("toJson() takes exactly 1 argument. " + std::to_string(args.size()) + " were given");
    }

    std::string string = args[0]->get<std::string>();
    std::vector<Token> tokens = Lexer{string}.tokenize();
    if (tokens.size() < 3) {
        throw std::runtime_error("Invalid string syntax for dictionary conversion");
    }
    tokens.insert(tokens.end() - 1, Token{TokenType::_Semi, 0, 0});
    Parser parser{tokens};
    std::vector<std::shared_ptr<ASTNode>> statements;
    try {
        statements = parser.parse();
    }
    catch (const ErrorException& e) {
        throw ErrorException(e.value);
    }
    catch (const std::exception& e) {
        std::string message = e.what();
        int index = message.find_last_of('\n');
        std::string problem = message.substr(index);
        runtimeError(problem);
    }
    if (auto dict_node = std::dynamic_pointer_cast<DictionaryNode>(statements[0])) {
        if (statements.size() != 1) {
            throw std::runtime_error("Invalid string syntax for dictionary conversion");
        }
        Environment env = buildStartingEnvironment();
        std::optional<std::shared_ptr<Value>> dictionary;
        return dict_node->evaluate(env);
    } else {
        throw std::runtime_error("Invalid string syntax for dictionary conversion");
        return nullptr;
    }
}


BuiltInFunctionReturn instanceDel(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("delAttr() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto inst = args[0]->get<std::shared_ptr<Instance>>();
    auto name_val = args[1];
    if (name_val->getType() != ValueType::String) {
        throw std::runtime_error("delAttr() expected an argument of Type:String but got " + getTypeStr(name_val->getType()));
    }

    auto name = name_val->get<std::string>();
    if (inst->getEnvironment().hasMember(name)) {
        inst->getEnvironment().delMember(name);
    }
    return std::make_shared<Value>();
}

BuiltInFunctionReturn instanceGet(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("getAttr() takes exactly 2 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto inst = args[0]->get<std::shared_ptr<Instance>>();
    auto name_val = args[1];
    if (name_val->getType() != ValueType::String) {
        throw std::runtime_error("getAttr() expected an argument of Type:String but got " + getTypeStr(name_val->getType()));
    }

    auto name = name_val->get<std::string>();
    return inst->getEnvironment().getMember(name);
}

BuiltInFunctionReturn instanceHas(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 2) {
        throw std::runtime_error("hasAttr() takes exactly 2 argument. " + std::to_string(args.size()) + " were given");
    }

    auto inst = args[0]->get<std::shared_ptr<Instance>>();
    auto name = args[1];
    if (name->getType() != ValueType::String) {
        throw std::runtime_error("hasAttr() expected an argument of Type:String but got " + getTypeStr(name->getType()));
    }

    bool has = inst->getEnvironment().hasMember(name->get<std::string>());

    return std::make_shared<Value>(has);
}

BuiltInFunctionReturn instanceSet(const std::vector<std::shared_ptr<Value>>& args, Environment& env) {
    if (args.size() != 3) {
        throw std::runtime_error("setAttr() takes exactly 3 arguments. " + std::to_string(args.size()) + " were given");
    }

    auto inst = args[0]->get<std::shared_ptr<Instance>>();
    auto name_val = args[1];
    auto value = args[2];
    if (name_val->getType() != ValueType::String) {
        throw std::runtime_error("setAttr() expected an argument of Type:String but got " + getTypeStr(name_val->getType()));
    }
    
    auto name = name_val->get<std::string>();
    inst->getEnvironment().addMember(name, value);
    return std::make_shared<Value>();
}