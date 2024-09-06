#include "parser.h"
#include "pch.h"
#include "global_context.h"
#include "library.h"
#include <iostream>
#include <fstream>

// Implement the ValueCompare function
bool ValueCompare::operator()(const std::shared_ptr<Value>& lhs, const std::shared_ptr<Value>& rhs) const {
    if (!lhs || !rhs) {
        return lhs < rhs;
    }

    const auto& lhs_variant = static_cast<const VariantType&>(*lhs);
    const auto& rhs_variant = static_cast<const VariantType&>(*rhs);

    return std::visit([&](auto&& lhsVal, auto&& rhsVal) -> bool {
        using T1 = std::decay_t<decltype(lhsVal)>;
        using T2 = std::decay_t<decltype(rhsVal)>;

        if constexpr (std::is_same_v<T1, T2>) {
            return lhsVal < rhsVal;
        } else {
            return lhs_variant.index() < rhs_variant.index();
        }
    }, lhs_variant, rhs_variant);
}

std::unordered_map<TokenType, ValueType> token_value_map{
    {TokenType::_IntType, ValueType::Integer},
    {TokenType::_FloatType, ValueType::Float},
    {TokenType::_BoolType, ValueType::Boolean},
    {TokenType::_StrType, ValueType::String},
    {TokenType::_ListType, ValueType::List},
    {TokenType::_FuncType, ValueType::Function},
    {TokenType::_BuiltInType, ValueType::BuiltInFunction},
    {TokenType::_NullType, ValueType::Null}
};

std::string getLine(const std::string& filename, int line) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string currentLine;
    int currentLineNum = 1;

    while (std::getline(file, currentLine)) {
        if (currentLineNum == line) {
            return currentLine;
        }
        currentLineNum++;
    }

    if (currentLineNum < line) {
        throw std::runtime_error("Line number out of range in file: " + filename);
    }
    return "";
}

void handleError(const std::string& message, int line, int column, std::string prefix) {
    std::string filename = GlobalContext::instance().getFilename();
    Style style{};
    std::string error = std::format("{}{}:{} File {}{}{} at {}{}line {} column {}{}:\n",
                                    style.red, prefix, style.reset, style.green, filename, style.reset,
                                    style.purple, style.underline, line, column, style.reset);
    error += std::format("        {}\n", getLine(filename, line));
    std::string spaces = "        ";
    for (int i = 0; i < column - 1; i++) {
        spaces += " ";
    }
    spaces += style.orange + "^\n";
    error += spaces;
    error += std::format("{}.{}", message, style.reset);
    throw std::runtime_error(error);
}

void parsingError(const std::string& message, int line, int column) {
    handleError(message, line, column, "Syntax Error");
}

void runtimeError(const std::string& message, int line, int column) {
    handleError(message, line, column, "Runtime Error");
}

void runtimeError(const std::string& message, std::shared_ptr<ASTNode> node) {
    runtimeError(message, node->line, node->column);
}

// << overload for the ValueType
std::ostream& operator<<(std::ostream& os, const ValueType& v_type) {
    os << getTypeStr(v_type);
    return os;
}

// << overload for the List type
std::ostream& operator<<(std::ostream& os, const List& list) {
    os << "[";
    for (size_t i = 0; i < list.size(); ++i) {
        if (list[i]) {
            // Use std::visit to handle the variant inside the shared_ptr<Value>
            std::visit([&os](const auto& v) {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, bool>) {
                    os << std::boolalpha << v;  // Print bool values as true/false
                } else if constexpr (std::is_same_v<T, std::string>) {
                    os << "'" << v << "'";
                } else if constexpr (std::is_same_v<T, std::shared_ptr<List>>) {
                    if (v) {
                        os << *v;  // Recursively print the inner list
                    } else {
                        os << "null";
                    }
                }
                else {
                    os << v;
                }
            }, *list[i]);
        } else {
            os << "null";
        }

        if (i < list.size() - 1) {
            os << ", ";
        }
    }
    os << "]";
    return os;
}


void printValue(const std::shared_ptr<Value> value, Environment& env) {
    Style style{};
    if (std::holds_alternative<int>(*value)) {
        auto int_value = std::get<int>(*value);
        std::cout << style.light_blue << int_value << style.reset;
    } else if (std::holds_alternative<double>(*value)) {
        auto double_value = std::get<double>(*value);
        if (double_value == static_cast<int>(double_value)) {
            std::cout << style.light_blue << double_value << ".0" << style.reset;
        } else {
            std::cout << style.light_blue << double_value << style.reset;
        }
    } else if (std::holds_alternative<bool>(*value)) {
        auto bool_value = std::get<bool>(*value);
        std::cout << style.purple << std::boolalpha << bool_value << style.reset;
    } else if (std::holds_alternative<std::string>(*value)) {
        auto string_value = std::get<std::string>(*value);
        std::cout << style.green << "'" << string_value << "'" << style.reset;
    } else if (std::holds_alternative<std::shared_ptr<List>>(*value)) {
        auto list_value = std::get<std::shared_ptr<List>>(*value);
        std::cout << "[";
        bool first = true;
        for (const auto item : *list_value) {
            if (!first) {
                std::cout << ", ";
            } else {
                first = false;
            }
            printValue(item, env);
        }
        std::cout << "]";
    } else if (std::holds_alternative<std::shared_ptr<Dictionary>>(*value)) {
        auto dict_value = std::get<std::shared_ptr<Dictionary>>(*value);
        std::cout << "{";
        bool first = true;
        for (const auto pair : *dict_value) {
            if (!first) {
                std::cout << ", ";
            } else {
                first = false;
            }
            printValue(pair.first, env);
            std::cout << ":";
            printValue(pair.second, env);
        }
        std::cout << "}";
    } else if (std::holds_alternative<std::shared_ptr<ASTNode>>(*value)) {
        auto func_value = std::get<std::shared_ptr<ASTNode>>(*value);
        if (auto func = dynamic_cast<FuncNode*>(func_value.get())) {
            if (func->func_name) {
                std::cout << "Function->" << *func->func_name;
            } else {
                throw std::runtime_error("Unable to find function name.");
            }
        } else {
            runtimeError("Tried to print value that was an unknown ASTNode.", func_value);
        }
    } else if (std::holds_alternative<std::shared_ptr<BuiltInFunction>>(*value)) {
        std::cout << "Built-in Function->" << env.getName(value);
    } else if (std::holds_alternative<ValueType>(*value)) {
        auto v_type = std::get<ValueType>(*value);
        std::string type_str = getTypeStr(v_type);
        if (type_str == "Type:Null") {
            std::cout << "Null";
        }
        else {
            std::cout << type_str;
        }
    }
    else {
        throw std::runtime_error("Received invalid value type to print.");
    }
}


std::optional<std::shared_ptr<Value>> AtomNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate atom" << std::endl;
    if (isInteger()) {
        return std::make_shared<Value>(getInteger());
    } else if (isFloat()) {
        return std::make_shared<Value>(getFloat());
    } else if (isBool()) {
        return std::make_shared<Value>(getBool());
    } else if (isString()) {
        return std::make_shared<Value>(getString());
    } else {
        parsingError("Unable to evaluate value.", line, column);
        return std::nullopt;
    }
}

std::variant<int, double, bool, std::string> AtomNode::getValue() const {
    return value;
}

bool AtomNode::isInteger() const {
    return std::holds_alternative<int>(value);
}

bool AtomNode::isFloat() const {
    return std::holds_alternative<double>(value);
}

bool AtomNode::isBool() const {
    return std::holds_alternative<bool>(value);
}

bool AtomNode::isString() const {
    return std::holds_alternative<std::string>(value);
}

int AtomNode::getInteger() const {
    return std::get<int>(value);
}

double AtomNode::getFloat() const {
    return std::get<double>(value);
}

bool AtomNode::getBool() const {
    return std::get<bool>(value);
}

std::string AtomNode::getString() const {
    return std::get<std::string>(value);
}


std::optional<std::shared_ptr<Value>> IdentifierNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate identifier" << std::endl;
    if (env.has(value)) {
        return env.get(value);
    } else if (env.hasFunction(value)) {
        return env.getFunction(value);
    } else {
        runtimeError(value + " is not defined", line, column);
    }
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> IdentifierNode::evaluate(Environment& env, std::shared_ptr<ValueType> member_type) {
    if (debug) std::cout << "evaluate identifier" << std::endl;
    if (env.hasMember(member_type, value)) {
        return env.getMember(member_type, value);
    } else {
        runtimeError(value + " is not defined", line, column);
    }
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> ListNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate list" << std::endl;
    List evaluated_list;

    for (const auto element : list) {
        if (element) {
            // Evaluate the ASTNode and add the result to the evaluated list
            auto result = element->evaluate(env);
            if (result) {
                evaluated_list.push_back(result.value());
            } else {
                runtimeError("List element was unable to be evaluated", line, column);
            }
        } else {
            runtimeError("List contained a nullptr pointing to an ASTNode", line, column); // Null ASTNode pointer
        }
    }

    return std::make_shared<Value>(std::make_shared<List>(evaluated_list));
}

std::optional<std::shared_ptr<Value>> DictionaryNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate dict" << std::endl;
    std::shared_ptr<Dictionary> evaluated_dict = std::make_shared<Dictionary>();

    for (const auto& pair : dictionary) {
        auto key = pair.first->evaluate(env);
        auto value = pair.second->evaluate(env);
        if (key && value) {
            evaluated_dict->insert({key.value(), value.value()});
        } else {
            runtimeError("Dictionary key or value was unable to be evaluated", line, column);
        }
    }

    return std::make_shared<Value>(evaluated_dict);
}

std::optional<std::shared_ptr<Value>> IndexNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate index" << std::endl;
    if (container == nullptr) {
        runtimeError("Null object is not subscriptable", line, column);
    }

    auto eval = container->evaluate(env);
    if (eval) {
        if (std::holds_alternative<std::string>(*eval.value())) {
            auto string_val = std::make_shared<std::string>(std::get<std::string>(*eval.value()));
            return getIndex(env, string_val);
        }
        else if (std::holds_alternative<std::shared_ptr<List>>(*eval.value())) {
            auto list_val = std::get<std::shared_ptr<List>>(*eval.value());
            return getIndex(env, list_val);
        } else if (std::holds_alternative<std::shared_ptr<Dictionary>>(*eval.value())) {
            auto dict_val = std::get<std::shared_ptr<Dictionary>>(*eval.value());
            return getIndex(env, dict_val);
        }
        else {
            runtimeError("Index node container was an unexpected type", line, column);
        }
    }
    return std::nullopt;
}

std::variant<char, std::shared_ptr<Value>> getAtIndex(std::variant<std::shared_ptr<std::string>,
                                                                    std::shared_ptr<List>,
                                                                    std::shared_ptr<Dictionary>> listr,
                                                        int index, int line, int column) {
    if (std::holds_alternative<std::shared_ptr<std::string>>(listr)) {
        auto string = std::get<std::shared_ptr<std::string>>(listr);
        if (index >= 0 && index < string->length()) {
            return string->at(index);
        } else if (index >= string->length() * -1 && index < 0) {
            return string->at(string->length() - -index);
        } else {
            runtimeError("String index out of range", line, column);
        }
    } else if (std::holds_alternative<std::shared_ptr<List>>(listr)) {
        auto list = std::get<std::shared_ptr<List>>(listr);
        if (index >= 0 && index < list->size()) {
            return list->at(index);
        } else if (index >= list->size() * -1 && index < 0) {
            return list->at(list->size() - -index);
        } else {
            runtimeError("List index out of range", line, column);
        }
    } else {
        runtimeError("getAtIndex did not receive a string or list", line, column);
    }
    return nullptr;
}

std::optional<std::shared_ptr<Value>> IndexNode::getIndex(Environment& env,
                                                    std::variant<std::shared_ptr<std::string>,
                                                                std::shared_ptr<List>,
                                                                std::shared_ptr<Dictionary>> distr) {
    if (end_index) {
        if (std::holds_alternative<std::shared_ptr<Dictionary>>(distr)) {
            runtimeError("Dictionary is not subscriptable", line, column);
            return std::nullopt;
        }
        auto start_result = start_index->evaluate(env);
        auto end_result = end_index->evaluate(env);
        if (start_result && end_result) {
            if (std::holds_alternative<int>(*start_result.value())) {
                int start_val = std::get<int>(*start_result.value());
                if (std::holds_alternative<int>(*end_result.value())) {
                    int end_val = std::get<int>(*end_result.value());
                    if (start_val <= end_val) {
                        if (std::holds_alternative<std::shared_ptr<std::string>>(distr)) {
                            if (start_val < 0 && end_val >= 0) {
                                return std::make_shared<Value>("");
                            }

                            std::string sub_str;
                            for (int i = start_val; i < end_val; i++) {
                                char c = std::get<char>(getAtIndex(std::get<std::shared_ptr<std::string>>(distr), i, line, column));
                                sub_str += c;
                            }
                            return std::make_shared<Value>(sub_str);
                        } else if (std::holds_alternative<std::shared_ptr<List>>(distr)){
                            std::shared_ptr<List> list = std::make_shared<List>();
                            if (start_val < 0 && end_val >= 0) {
                                return std::make_shared<Value>(list);
                            }
                            
                            for (int i = start_val; i < end_val; i++) {
                                auto list_val = std::get<std::shared_ptr<Value>>(getAtIndex(std::get<std::shared_ptr<std::string>>(distr), i, line, column));
                                list->push_back(list_val);
                            }
                            return std::make_shared<Value>(list);
                        } else {
                            runtimeError("Invalid type for getting index", line, column);
                        }
                    } else {
                        runtimeError("Invalid format of slice indeces. The start index is larger than the end", line, column);
                    }
                } else {
                    runtimeError("The ending index value was not an int", line, column);
                }
            } else {
                runtimeError("The starting index value was not an int", line, column);
            }
        } else {
            runtimeError("Failed to evaluate start_index or end_index", line, column);
        }
    } else {
        if (std::holds_alternative<std::shared_ptr<Dictionary>>(distr)) {
            // It's a dictionary, not string or list
            auto dict = std::get<std::shared_ptr<Dictionary>>(distr);
            auto key = start_index->evaluate(env);
            if (key) {
                auto it = dict->find(key.value());
                if (it != dict->end()) {
                    return it->second;
                } else {
                    runtimeError("Unable to find key " + getValueStr(key.value()) + " in dictionary", line, column);
                }
            } else {
                runtimeError("Failed to evaluate key", line, column);
            }
        } else {
            auto result = start_index->evaluate(env);
            if (result) {
                if (std::holds_alternative<int>(*result.value())) {
                    int int_val = std::get<int>(*result.value());
                    if (std::holds_alternative<std::shared_ptr<std::string>>(distr)) {
                        auto get_char = getAtIndex(distr, int_val, line, column);
                        if (std::holds_alternative<char>(get_char)) {
                            auto c = std::get<char>(get_char);
                            std::string str = std::string(1, c);
                            return std::make_shared<Value>(str);
                        }
                    } else {
                        auto value = std::get<std::shared_ptr<Value>>(getAtIndex(distr, int_val, line, column));
                        return value;
                    }
                } else {
                    runtimeError("The index was not given an int", start_index);
                }
            } else {
                runtimeError("Failed to evaluate start_index", start_index);
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}

void setAtIndex(std::variant<std::shared_ptr<List>, std::shared_ptr<Dictionary>> env_dist, std::shared_ptr<Value> index_key, std::shared_ptr<Value> value) {
    if (std::holds_alternative<std::shared_ptr<List>>(env_dist)) {
        auto env_list = std::get<std::shared_ptr<List>>(env_dist);
        int list_index = std::get<int>(*index_key);
        if (list_index >= 0 && list_index <= env_list->size()) {
            if (std::holds_alternative<std::string>(*value)) {
                // Assigning a string
                auto str_val = std::get<std::string>(*value);
                std::string s = "";
                int i = 0;
                for (auto c : str_val) {
                    s += c;
                    auto char_str = std::make_shared<Value>(s);
                    if (list_index + i == env_list->size()) {
                        env_list->push_back(char_str);
                    } else {
                        env_list->insert(env_list->begin() + list_index + i, char_str);
                    }
                    s = "";
                    i++;
                }
            } else if (std::holds_alternative<std::shared_ptr<List>>(*value)) {
                auto list_val = std::get<std::shared_ptr<List>>(*value);
                // Assigning a list
                for (int i = list_val->size() - 1; i >= 0; i--) {
                    if (list_index == env_list->size()) {
                        env_list->push_back(list_val->at(i));
                    } else {
                        env_list->insert(env_list->begin() + list_index, list_val->at(i));
                    }
                }
            }
        }
    } else {
        auto env_dict = std::get<std::shared_ptr<Dictionary>>(env_dist);
        auto it = env_dict->find(index_key);
        if (it != env_dict->end()) {
            it->second = value;
        } else {
            env_dict->insert({index_key, value});
        }
    }
}

void IndexNode::assignIndex(Environment& env, std::shared_ptr<Value> value) {
    auto eval = container->evaluate(env);
    std::shared_ptr<Value> env_val;
    if (eval) {
        env_val = eval.value();
    } else {
        runtimeError("Index assignment unable to evaluate the container", line, column);
        return;
    }
    if (std::holds_alternative<std::shared_ptr<List>>(*env_val)) {
        std::shared_ptr<List> env_list = std::get<std::shared_ptr<List>>(*env_val);

        if (end_index == nullptr) {
            // A single index assignment
            auto index_eval = start_index->evaluate(env);
            if (index_eval) {
                if (std::holds_alternative<int>(*index_eval.value())) {
                    int index = std::get<int>(*index_eval.value());
                    if (index >= 0 && index < env_list->size()) {
                        env_list->at(index) = value;
                    } else if (index < 0 && index >= env_list->size() * -1) {
                        env_list->at(env_list->size() - index);
                    } else {
                        runtimeError("Index assignment out of range", line, column);
                    }
                } else {
                    runtimeError("Index assignment requires int", line, column);
                }
            } else {
                runtimeError("Failed to evaluate assignment index", line, column);
            }
        } else {
            // List slice index assignment
            auto start_eval = start_index->evaluate(env);
            auto end_eval = end_index->evaluate(env);
            if (start_eval && end_eval) {
                if (std::holds_alternative<int>(*start_eval.value())) {
                    int start_val = std::get<int>(*start_eval.value());
                    if (std::holds_alternative<int>(*end_eval.value())) {
                        int end_val = std::get<int>(*end_eval.value());
                        int slice_size = std::abs(end_val - start_val);
                        // Cut out the slice section of the original list
                        for (int i = 0; i < slice_size; i++) {
                            if (env_list->begin() + start_val == env_list->end()) {
                                break;
                                // runtimeError("Assignment index went out of range.");
                            }
                            env_list->erase(env_list->begin() + start_val);
                        }
                        setAtIndex(env_list, start_eval.value(), value);
                    } else {
                        runtimeError("Assignment index end value is not an int", line, column);
                    }
                } else {
                    runtimeError("Assignment index start value is not an int", line, column);
                }
            } else {
                runtimeError("Assignment index was unable to evaluate", line, column);
            }
        }
    }
    else if (std::holds_alternative<std::shared_ptr<Dictionary>>(*env_val)) {
        std::shared_ptr<Dictionary> env_dict = std::get<std::shared_ptr<Dictionary>>(*env_val);

        if (end_index == nullptr) {
            auto key_eval = start_index->evaluate(env);
            if (key_eval) {
                setAtIndex(env_dict, key_eval.value(), value);
            } else {
                runtimeError("Failed to evaluate assignment key", line, column);
            }
        } else {
            runtimeError("Dictionary is not subscriptable", line, column);
        }
    }
    else {
        runtimeError(getValueStr(env_val) + " object does not support item assignment", line, column);
        return;
    }
}


template <typename T1, typename T2>
std::optional<std::shared_ptr<Value>> doArithmetic(const T1 lhs, const T2 rhs, const TokenType op, int line, int column) {
    // Helper function to give the bool value of each type
    auto checkTruthy = [](const Value& value) -> bool {
        return std::visit([](const auto& v) -> bool {
            using T = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<T, bool>) {
                return v;
            } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
                return v != 0;
            } else if constexpr (std::is_same_v<T, std::string>) {
                return !v.empty();
            } else if constexpr (std::is_same_v<T, std::shared_ptr<List>>) {
                return !v->empty();
            } else if constexpr (std::is_same_v<T, std::shared_ptr<Dictionary>>) {
                return !v->empty();
            } else if constexpr (std::is_same_v<T, ValueType>) {
                return v != ValueType::Null;
            }
            return false;
        }, value);
    };

    // Deep comparison function for lists
    auto deepCompareLists = [&](const std::shared_ptr<List>& lhs_list, const std::shared_ptr<List>& rhs_list) -> bool {
        if (lhs_list->size() != rhs_list->size()) {
            return false;
        }
        for (size_t i = 0; i < lhs_list->size(); ++i) {
            auto lhs_element = lhs_list->at(i);
            auto rhs_element = rhs_list->at(i);
            auto result = std::visit([&](auto lhs_val) -> bool {
                return std::visit([&](auto rhs_val) -> bool {
                    auto comp_result = doArithmetic(lhs_val, rhs_val, TokenType::_Compare, line, column);
                    if (comp_result && std::holds_alternative<bool>(*comp_result.value())) {
                        return std::get<bool>(*comp_result.value());
                    }
                    return false;
                }, *rhs_element);
            }, *lhs_element);

            if (!result) {
                return false;
            }
        }
        return true;
    };

    auto deepCompareDictionaries = [&](const std::shared_ptr<Dictionary>& lhs_dict,
                                    const std::shared_ptr<Dictionary>& rhs_dict) -> bool {
        if (lhs_dict->size() != rhs_dict->size()) {
            return false;
        }

        for (const auto& lhs_pair : *lhs_dict) {
            auto lhs_key = lhs_pair.first;
            auto lhs_value = lhs_pair.second;

            auto rhs_iter = rhs_dict->find(lhs_key);
            if (rhs_iter == rhs_dict->end()) {
                // Key from lhs_dict not found in rhs_dict
                return false;
            }

            auto rhs_value = rhs_iter->second;

            // Compare values
            auto result = std::visit([&](auto lhs_val) -> bool {
                return std::visit([&](auto rhs_val) -> bool {
                    auto comp_result = doArithmetic(lhs_val, rhs_val, TokenType::_Compare, line, column);
                    if (comp_result && std::holds_alternative<bool>(*comp_result.value())) {
                        return std::get<bool>(*comp_result.value());
                    }
                    return false;
                }, *rhs_value);
            }, *lhs_value);

            if (!result) {
                return false;
            }
        }

        return true;
    };

    // Handle AND OR for all type cases
    if (op == TokenType::_And) {
        if (checkTruthy(lhs)) {
            if (checkTruthy(rhs)) {
                return std::make_shared<Value>(rhs);
            } else {
                return std::make_shared<Value>(false);
            }
        } else {
            return std::make_shared<Value>(false);
        }
    }
    else if (op == TokenType::_Or) {
        if (checkTruthy(lhs)) {
            return std::make_shared<Value>(lhs);
        } else if (checkTruthy(rhs)) {
            return std::make_shared<Value>(rhs);
        } else {
            return std::make_shared<Value>(false);
        }
    }
    else if ((op == TokenType::_Compare || op == TokenType::_NotEqual)) {
        if ((std::is_same_v<T1, ValueType> || std::is_same_v<T2, ValueType>) && !std::is_same_v<T1, T2>) {
            return std::make_shared<Value>(false);
        }
    }
    
    if constexpr (std::is_same_v<T1, std::shared_ptr<List>> && std::is_same_v<T2, std::shared_ptr<List>>) {
        // BOTH ARE LISTS
        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) {
            std::shared_ptr<List> result = lhs;  // Start with a copy of the first list
            result->insert(result->end(), rhs->begin(), rhs->end());  // Append the second list
            return std::make_shared<Value>(result);
        }
        else if (op == TokenType::_Compare) return std::make_shared<Value>(deepCompareLists(lhs, rhs));
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(!deepCompareLists(lhs, rhs));
    }
    else if constexpr (std::is_same_v<T1, std::shared_ptr<Dictionary>> && std::is_same_v<T2, std::shared_ptr<Dictionary>>) {
        // BOTH ARE DICTIONARIES
        if (op == TokenType::_Compare) return std::make_shared<Value>(deepCompareDictionaries(lhs, rhs));
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(!deepCompareDictionaries(lhs, rhs));
    }
    else if constexpr (std::is_same_v<T1, std::string> && std::is_same_v<T2, std::string>) {
        // BOTH STRINGS
        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) return std::make_shared<Value>(lhs + rhs);
        else if (op == TokenType::_Compare) return std::make_shared<Value>(lhs == rhs);
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(lhs != rhs);
    }
    else if constexpr ((std::is_same_v<T1, std::string> && std::is_same_v<T2, int>) || (std::is_same_v<T1, int> && std::is_same_v<T2, std::string>)) {
        // A STRING AND INT
        if (op == TokenType::_Multiply || op == TokenType::_MultiplyEquals) {
            std::string new_str = "";
            if constexpr (std::is_same_v<T1, std::string>) {
                for (int i = 0; i < rhs; i++) {
                    new_str += lhs;
                }
            } else {
                for (int i = 0; i < lhs; i++) {
                    new_str += rhs;
                }
            }
            return std::make_shared<Value>(new_str);
        }
    }
    else if constexpr ((std::is_same_v<T1, int> || std::is_same_v<T1, double>) &&
                       (std::is_same_v<T2, int> || std::is_same_v<T2, double>)) {
        // Explicitly handle mixed types
        double lhs_double = static_cast<double>(lhs);
        double rhs_double = static_cast<double>(rhs);
        std::shared_ptr<Value> value = std::make_shared<Value>();

        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) value = std::make_shared<Value>(lhs_double + rhs_double);
        else if (op == TokenType::_Minus || op == TokenType::_MinusEquals) value = std::make_shared<Value>(lhs_double - rhs_double);
        else if (op == TokenType::_Multiply || op == TokenType::_MultiplyEquals) value = std::make_shared<Value>(lhs_double * rhs_double);
        else if (op == TokenType::_Divide || op == TokenType::_DivideEquals) {
            if (rhs_double == 0) runtimeError("Attempted division by 0", line, column);
            return std::make_shared<Value>(lhs_double / rhs_double);
        }
        else if (op == TokenType::_FloorDiv) {
            if (rhs_double == 0) runtimeError("Attempted division by 0", line, column);
            value = std::make_shared<Value>(static_cast<int>(lhs_double / rhs_double));
        }
        else if (op == TokenType::_Mod) {
            if (rhs_double == 0) runtimeError("Attempted division by 0", line, column);
            if constexpr (std::is_same_v<T1, double> || std::is_same_v<T2, double>) {
                runtimeError("The modulus '%' can only be performed on ints", line, column);
            } else {
                value = std::make_shared<Value>(static_cast<int>(lhs) % static_cast<int>(rhs));
            }
        }
        else if (op == TokenType::_Caret || op == TokenType::_DoubleMultiply) value = std::make_shared<Value>(std::pow(lhs_double, rhs_double));
        else if (op == TokenType::_Compare) return std::make_shared<Value>(lhs_double == rhs_double);
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(lhs_double != rhs_double);
        else if (op == TokenType::_GreaterThan) return std::make_shared<Value>(lhs_double > rhs_double);
        else if (op == TokenType::_GreaterEquals) return std::make_shared<Value>(lhs_double >= rhs_double);
        else if (op == TokenType::_LessThan) return std::make_shared<Value>(lhs_double < rhs_double);
        else if (op == TokenType::_LessEquals) return std::make_shared<Value>(lhs_double <= rhs_double);

        if (std::is_same_v<T1, int> && std::is_same_v<T2, int>) {
            // Make sure that, if it starts an int and doesn't need to become double, it stays int.
            if (!value) {
                runtimeError("Reached end of arithmetic without value", line, column);
            }
            if (std::holds_alternative<double>(*value)) {
                auto double_value = std::get<double>(*value);
                if (double_value == static_cast<int>(double_value)) {
                    return std::make_shared<Value>(static_cast<int>(double_value));
                } else {
                    return value;
                }
            } else {
                return value;
            }
        } else {
            return value;
        }
    }
    else if constexpr (std::is_same_v<T1, T2>) {
        if (op == TokenType::_Compare) return std::make_shared<Value>(lhs == rhs);
        if (op == TokenType::_NotEqual) return std::make_shared<Value>(lhs != rhs);
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> BinaryOpNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate binary" << std::endl;

    if (op == TokenType::_Equals) {
        // An equals is a special case
        std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);
        if (!right_value.has_value()) {
            runtimeError("Failed to set variable. Operand could not be computed", line, column);
        }

        // Give it the actual left string, not the value of the variable
        if (auto identifier_node = dynamic_cast<IdentifierNode*>(left.get())) {
            env.set(identifier_node->value, right_value.value());
        } else if (auto index_node = dynamic_cast<IndexNode*>(left.get())) {
            index_node->assignIndex(env, right_value.value());
        } else if (auto list_node = std::dynamic_pointer_cast<ListNode>(left)) {
            if (!std::holds_alternative<std::shared_ptr<List>>(*right_value.value())) {
                runtimeError("Expected list. Cannot unpack " + getValueStr(right_value.value()), line, column);
            }

            auto right_list = std::get<std::shared_ptr<List>>(*right_value.value());
            if (right_list->size() > list_node->list.size()) {
                runtimeError("Too many values to unpack", line, column);
            } else if (right_list->size() < list_node->list.size()) {
                runtimeError("Too few values to unpack", line, column);
            }

            for (int i = 0; i < right_list->size(); i++) {
                if (auto identifier_node = std::dynamic_pointer_cast<IdentifierNode>(list_node->list.at(i))) {
                    env.set(identifier_node->value, right_list->at(i));
                }
                else {
                    runtimeError("Cannot assign value to literal", line, column);
                }
            }
        }
        else {
            runtimeError("The operator '=' can only be used with variables", line, column);
        }
    } else if (op == TokenType::_Dot) {
        // Handle member functions of types
        std::optional<std::shared_ptr<Value>> left_value = left->evaluate(env);
        if (!left_value.has_value()) {
            runtimeError("Failed to get member function. Identifier could not be computed", line, column);
        }

        // Get the type of the member
        std::shared_ptr<ValueType> member_type = std::make_shared<ValueType>(getValueType(left_value.value()));
        if (auto func_node = std::dynamic_pointer_cast<FuncCallNode>(right)) {
            // It's a member function
            // Save the result of the member to pass into the function
            func_node->member_value = left_value.value();
            // Pass member type so evaluate knows to search for functions of that type
            return func_node->evaluate(env, member_type);
        }
        else if (auto ident_node = std::dynamic_pointer_cast<IdentifierNode>(right)) {
            return ident_node->evaluate(env, member_type);
        }
        else {
            runtimeError("Invalid syntax", line, column);
        }
    } else if (op == TokenType::_In) {
        auto left_value = left->evaluate(env);
        auto right_value = right->evaluate(env);
        if (!left_value.has_value() || !right_value.has_value()) {
            runtimeError("Failed arguments of 'in'", line, column);
        }

        if (std::holds_alternative<std::shared_ptr<List>>(*right_value.value())) {
            auto list = std::get<std::shared_ptr<List>>(*right_value.value());
            for (const auto& item : *list) {
                auto result = std::visit([&](auto lhs) -> std::optional<std::shared_ptr<Value>> {
                    return std::visit([&](auto rhs) -> std::optional<std::shared_ptr<Value>> {
                        return doArithmetic(lhs, rhs, TokenType::_Compare, line, column);
                    }, *item);
                }, *left_value.value());

                if (result && std::holds_alternative<bool>(*result.value()) && std::get<bool>(*result.value())) {
                    return std::make_shared<Value>(true);
                }
            }
            return std::make_shared<Value>(false);
        }
        else if (std::holds_alternative<std::shared_ptr<Dictionary>>(*right_value.value())) {
            auto dict = std::get<std::shared_ptr<Dictionary>>(*right_value.value());
            for (const auto& pair : *dict) {
                auto result = std::visit([&](auto lhs) -> std::optional<std::shared_ptr<Value>> {
                    return std::visit([&](auto rhs) -> std::optional<std::shared_ptr<Value>> {
                        return doArithmetic(lhs, rhs, TokenType::_Compare, line, column);
                    }, *pair.first);
                }, *left_value.value());

                if (result && std::holds_alternative<bool>(*result.value()) && std::get<bool>(*result.value())) {
                    return std::make_shared<Value>(true);
                }
            }
            return std::make_shared<Value>(false);
        }
        else if (std::holds_alternative<std::string>(*right_value.value())) {
            auto string = std::get<std::string>(*right_value.value());
            auto left_val = *left_value.value();
            if (!std::holds_alternative<std::string>(left_val)) {
                return std::make_shared<Value>(false);
            }
            std::string left = std::get<std::string>(left_val);
            auto index = string.find(left);
            if (index != std::string::npos) {
                return std::make_shared<Value>(true);
            }
            return std::make_shared<Value>(false);
        }
        else {
            runtimeError("Expected list or dictionary for 'in' evaluation", line, column);
        }
    }
    else {
        std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);
        std::optional<std::shared_ptr<Value>> left_value = left->evaluate(env);
        // Arithmetic operations need two values to operate on
        if (!left_value.has_value() || !right_value.has_value()) {
            runtimeError(std::format("Failed to evaluate expression with operator '{}': one or both operands could not be computed",
                                            token_labels[op]), line, column);
        }

        // Perform arithmetic operation
        auto result = std::visit([&](auto lhs) -> std::optional<std::shared_ptr<Value>> {
            return std::visit([&](auto rhs) -> std::optional<std::shared_ptr<Value>> {
                return doArithmetic(lhs, rhs, op, line, column);
            }, *right_value.value());
        }, *left_value.value());

        if (!result.has_value()) {
            runtimeError(std::format("Unsupported operand types for operation. operation was {} '{}' {}",
                                                getValueStr(left_value.value()), token_labels[op], getValueStr(right_value.value())), line, column);
        }

        if (op == TokenType::_PlusEquals || op == TokenType::_MinusEquals || op == TokenType::_MultiplyEquals || op == TokenType::_DivideEquals) {
            // Handle setting +=, -= etc.
            if (auto identifierNode = dynamic_cast<IdentifierNode*>(left.get())) {
                env.set(identifierNode->value, result.value());
            }
            else {
                runtimeError("The operator '=' can only be used with variables", line, column);
            }
        }
        else {
            return result;
        }

    }
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> UnaryOpNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate unary" << std::endl;
    std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);
    if (!right_value.has_value()) {
        runtimeError(std::format("Failed to evaluate expression with operator '{}': the operand could not be computed",
                                         token_labels[op]), line, column);
    }

    std::string RHS = getValueStr(right_value.value());

    if (RHS == "integer") {
        if (op == TokenType::_Minus) {
            int rhs = std::get<int>(*right_value.value());
            return std::make_shared<Value>(-rhs);
        }
        else if (op == TokenType::_Plus) {
            return right_value;
        }
        else if (op == TokenType::_Not || op == TokenType::_Exclamation) {
            int rhs = std::get<int>(*right_value.value());
            return std::make_shared<Value>(rhs == 0);
        }
    }
    else if (RHS == "float") {
        if (op == TokenType::_Minus) {
            double rhs = std::get<double>(*right_value.value());
            return std::make_shared<Value>(-rhs);
        }
        else if (op == TokenType::_Plus) {
            return right_value;
        }
        else if (op == TokenType::_Not || op == TokenType::_Exclamation) {
            double rhs = std::get<double>(*right_value.value());
            return std::make_shared<Value>(rhs == 0.0);
        }
    }
    else if (RHS == "boolean") {
        if (op == TokenType::_Not || op == TokenType::_Exclamation) {
            bool rhs = std::get<bool>(*right_value.value());
            return std::make_shared<Value>(!rhs);
        }
    }
    else if (RHS == "type") {
        ValueType rhs = std::get<ValueType>(*right_value.value());
        if (rhs == ValueType::Null && (op == TokenType::_Not || op == TokenType::_Exclamation)) {
            return std::make_shared<Value>(true);
        }
    }
    else if (RHS == "string") {
        if (op == TokenType::_Not || op == TokenType::_Exclamation) {
            std::string rhs = std::get<std::string>(*right_value.value());
            return std::make_shared<Value>(rhs == "");
        }
    }

    runtimeError(std::format("Unsupported operand types for operation. Operation was '{}' {}", token_labels[op], RHS), line, column);
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> ParenthesisOpNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate parenthesis" << std::endl;
    std::optional<std::shared_ptr<Value>> expr_value = expr->evaluate(env);
    return expr_value;
}

std::optional<std::shared_ptr<Value>> ScopeNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate scope" << std::endl;
    env.addScope();
    for (auto statement : block) {
        auto result = statement->evaluate(env);
    }

    env.removeScope();
    return std::nullopt;
}

bool ScopedNode::getComparisonValue(Environment& env) const {
    auto result = comparison->evaluate(env);
    if (result.has_value()) {
        auto checkTruthy = [](const Value& value) -> bool {
            return std::visit([](const auto& v) -> bool {
                using T = std::decay_t<decltype(v)>;
                if constexpr (std::is_same_v<T, bool>) {
                    return v;
                } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, double>) {
                    return v != 0;
                } else if constexpr (std::is_same_v<T, std::string>) {
                    return !v.empty();
                } else if constexpr (std::is_same_v<T, std::shared_ptr<List>>) {
                    return !v->empty();
                } else if constexpr (std::is_same_v<T, std::shared_ptr<Dictionary>>) {
                    return !v->empty();
                } else if constexpr (std::is_same_v<T, ValueType>) {
                    return v != ValueType::Null;
                }
                return false;
            }, value);
        };

        return checkTruthy(*result.value());
    }
    else {
        runtimeError("Missing a boolean comparison for keyword to evaluate", line, column);
        return false;
    }
}

std::optional<std::shared_ptr<Value>> ScopedNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate scoped" << std::endl;
    if (if_link) {
        // It's connected to a former if statement
        if (if_link->last_comparison_result) {
            // The previous if was true, so skip
            last_comparison_result = true;
            return {};
        }
    }

    if (comparison && getComparisonValue(env) == false) {
        last_comparison_result = false;
        return std::nullopt;
    } else if (comparison) {
        last_comparison_result = true;
    }

    std::string str = token_labels[keyword];
    if (str == "if" || str == "elif" || str == "else" || str == "while" || str == "for") {
        env.addScope();

        if (str == "while") {
            env.addLoop();
            while (getComparisonValue(env)) {
                try {
                    for (int i = 0; i < statements_block.size(); i++) {
                        auto result = statements_block[i]->evaluate(env);
                    }
                }
                catch (const BreakException) {
                    break;
                }
                catch (const ContinueException) {
                    continue;
                }
            }
            env.removeLoop();
        }
        else {
            for (int i = 0; i < statements_block.size(); i++) {
                std::optional<std::shared_ptr<Value>> result = statements_block[i]->evaluate(env);
            }
        }

        env.removeScope();
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> ForNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate for" << std::endl;
    env.addScope();
    env.addLoop();
    auto init_node = dynamic_cast<BinaryOpNode*>(initialization.get());
    if (init_node->op != TokenType::_In) {
        initialization->evaluate(env);

        int variable;
        if (std::holds_alternative<int>(*env.get(*init_string))) {
            auto i = std::get<int>(*env.get(*init_string));
            variable = i;
        }
        else {
            runtimeError("For loop requires int variable", line, column);
        }

        while (true) {
            auto cond_value = condition_value->evaluate(env);
            if (!cond_value) {
                runtimeError("Unable to evaluate for loop condition", line, column);
            }

            if (std::holds_alternative<bool>(*cond_value.value())) {
                auto bool_value = std::get<bool>(*cond_value.value());
                if (!bool_value) break;
            } else {
                runtimeError("For loop requires boolean condition", line, column);
            }

            int original_variable = variable;

            try {
                for (auto statement : block) {
                    auto result = statement->evaluate(env);
                }
            }
            catch (const BreakException) {
                break;
            }
            catch (const ContinueException) {
                continue;
            }

            variable = original_variable;
            // set variable in env
            env.set(*init_string.get(), std::make_shared<Value>(variable));
            // increment them
            increment->evaluate(env);
            if (std::holds_alternative<int>(*env.get(*init_string))) {
                auto i = std::get<int>(*env.get(*init_string));
                variable = i;
            }
        }
    } else {
        // in was used with for loop instead of integer
        auto container_result = init_node->right->evaluate(env);
        if (std::holds_alternative<std::shared_ptr<List>>(*container_result.value())) {
            auto list = std::get<std::shared_ptr<List>>(*container_result.value());
            auto ident_node = dynamic_cast<IdentifierNode*>(init_node->left.get());
            std::string var_string = ident_node->value;

            for (std::shared_ptr<Value> item : *list) {
                env.set(var_string, item);
                try {
                    for (auto statement : block) {
                        auto result = statement->evaluate(env);
                    }
                }
                catch (const BreakException) {
                    break;
                }
                catch (const ContinueException) {
                    continue;
                }
            }
        }
        else if (std::holds_alternative<std::shared_ptr<Dictionary>>(*container_result.value())) {
            auto dict = std::get<std::shared_ptr<Dictionary>>(*container_result.value());
            auto ident_node = dynamic_cast<IdentifierNode*>(init_node->left.get());
            std::string var_string = ident_node->value;

            for (const auto& pair : *dict) {
                std::shared_ptr<List> arg_list = std::make_shared<List>();
                arg_list->push_back(pair.first);
                arg_list->push_back(pair.second);
                env.set(var_string, std::make_shared<Value>(arg_list));
                try {
                    for (auto statement : block) {
                        auto result = statement->evaluate(env);
                    }
                }
                catch (const BreakException) {
                    break;
                }
                catch (const ContinueException) {
                    continue;
                }
            }
        }
        else if (std::holds_alternative<std::string>(*container_result.value())) {
            auto string = std::get<std::string>(*container_result.value());
            auto ident_node = dynamic_cast<IdentifierNode*>(init_node->left.get());
            std::string var_string = ident_node->value;

            for (char c : string) {
                env.set(var_string, std::make_shared<Value>(std::string(1, c)));
                try {
                    for (auto statement : block) {
                        auto result = statement->evaluate(env);
                    }
                }
                catch (const BreakException) {
                    break;
                }
                catch (const ContinueException) {
                    continue;
                }
            }
        }
        else {
            runtimeError("For loop expected iterable container", line, column);
        }
    }

    env.removeScope();
    env.removeLoop();
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> KeywordNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate keyword" << std::endl;
    if (keyword == TokenType::_Break) {
        if (env.inLoop()) {
            throw BreakException();
        }
        else {
            runtimeError("Break used outside of loop", line, column);
        }
    } else if (keyword == TokenType::_Continue) {
        if (env.inLoop()) {
            throw ContinueException();
        }
        else {
            runtimeError("Continue used outside of loop", line, column);
        }
    }
    else if (keyword == TokenType::_Return) {
        if (right != nullptr) {
            throw ReturnException(right->evaluate(env));
        } else {
            throw ReturnException(std::nullopt);
        }
    }
    else if (keyword == TokenType::_Import) {
        std::string path = GlobalContext::instance().getFilename();
        std::string new_path = path.substr(0, path.find_last_of('/'));
        auto right_value = right->evaluate(env);
        if (!right_value.has_value() || !std::holds_alternative<std::string>(*right_value.value())) {
            runtimeError("Import expected filename string", line, column);
        }
        new_path = new_path + "/" + std::get<std::string>(*right_value.value()) + ".funcy";
        if (new_path == path) {
            runtimeError("A file cannot import itself", line, column);
        }

        std::string source_code = readSourceCodeFromFile(new_path);

        if (source_code.empty()) {
            runtimeError("File " + new_path + " is empty or could not be read", line, column);
        }

        GlobalContext::instance().setFilename(new_path);

        Lexer lexer{source_code};
        std::vector<Token> tokens = lexer.tokenize();

        // for (auto token : tokens) {
        //     std::cout << token << std::endl;
        // }

        Parser parser{tokens};
        std::vector<std::shared_ptr<ASTNode>> statements = parser.parse();
        
        // ASTPrinter printer;
        // printer.print(statements);

        int stmnt_num = 0;
        for (auto statement : statements) {
            try {
                stmnt_num += 1;
                std::optional<std::shared_ptr<Value>> result = statement->evaluate(env);
            }
            catch (const ReturnException) {
                runtimeError("Return was used outside of function", line, column);
            }
            catch (const BreakException) {
                runtimeError("Break was used outside of loop", line, column);
            }
            catch (const ContinueException) {
                runtimeError("Continue was used outside of loop", line, column);
            }
        }

        GlobalContext::instance().removeFilename();
        return std::nullopt;
    }
    else if (token_value_map.contains(keyword)) {
        return std::make_shared<Value>(token_value_map[keyword]);
    }

    if (right) {
        return right->evaluate(env);
    } else {
        return std::nullopt;
    }
}

std::optional<std::shared_ptr<Value>> FuncNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate function" << std::endl;
    return std::make_shared<Value>(std::static_pointer_cast<ASTNode>(std::make_shared<FuncNode>(*this)));
}

void FuncNode::setArgs(std::vector<std::shared_ptr<Value>> values, Environment& base_env, Environment& local_env) {
    if (values.size() != args.size()) {
        runtimeError(std::format("Incorrect number of args were passed in. {} instead of {}", values.size(), args.size()), line, column);
    }

    for (int i = 0; i < values.size(); i++) {
        if (auto ident_node = dynamic_cast<IdentifierNode*>(args.at(i).get())) {
            std::string arg_string = ident_node->value;
            local_env.set(arg_string, values.at(i));
        } else {
            runtimeError("Unable to convert identifier for function argument", line, column);
        }
    }

    return;
}

std::optional<std::shared_ptr<Value>> FuncNode::callFunc(List values, Environment& env) {
    Environment local_env = env;
    local_env.resetLoop();
    local_env.addScope();
    setArgs(values, env, local_env);
    for (auto statement : block) {
        try {
            if (auto func_statement = dynamic_cast<FuncCallNode*>(statement.get())) {
                func_statement->base_env = std::make_shared<Environment>(env);
                auto result = func_statement->evaluate(local_env);
            } else {
                auto result = statement->evaluate(local_env);
            }
        }
        catch (const ReturnException& e) {
            return e.value;
        }
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> FuncCallNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate function call" << std::endl;
    auto mapped_value = identifier->evaluate(env).value();
    if (std::holds_alternative<std::shared_ptr<ASTNode>>(*mapped_value)) {
        auto func_value = std::get<std::shared_ptr<ASTNode>>(*mapped_value);
        if (auto func = dynamic_cast<FuncNode*>(func_value.get())) {
            if (base_env) {
                return func->callFunc(evaluateArgs(env), *base_env.get());
            }
            else {
                return func->callFunc(evaluateArgs(env), env);
            }
        } else {
            runtimeError("Unable to call function " + dynamic_cast<IdentifierNode*>(identifier.get())->value, line, column);
        }
    } else if (std::holds_alternative<std::shared_ptr<BuiltInFunction>>(*mapped_value)) {
        auto func_value = std::get<std::shared_ptr<BuiltInFunction>>(*mapped_value);
        List values = evaluateArgs(env);
        try {
            return (*func_value)(values, env);
        }
        catch (const std::exception e) {
            runtimeError(e.what(), line, column);
        }
    } else {
        runtimeError("Object type " + getValueStr(mapped_value) + " is not callable", line, column);
    }
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> FuncCallNode::evaluate(Environment& env, std::shared_ptr<ValueType> member_type) {
    if (debug) std::cout << "evaluate function call" << std::endl;
    auto ident_node = dynamic_cast<IdentifierNode*>(&*identifier);
    auto mapped_value = ident_node->evaluate(env, member_type).value();
    if (std::holds_alternative<std::shared_ptr<ASTNode>>(*mapped_value)) {
        auto func_value = std::get<std::shared_ptr<ASTNode>>(*mapped_value);
        if (auto func = dynamic_cast<FuncNode*>(func_value.get())) {
            auto values = evaluateArgs(env);
            values.insert(values.begin(), member_value);
            if (base_env) {
                return func->callFunc(values, *base_env.get());
            }
            else {
                return func->callFunc(values, env);
            }
        } else {
            runtimeError("Unable to call function " + dynamic_cast<IdentifierNode*>(identifier.get())->value, line, column);
        }
    } else if (std::holds_alternative<std::shared_ptr<BuiltInFunction>>(*mapped_value)) {
        auto func_value = std::get<std::shared_ptr<BuiltInFunction>>(*mapped_value);
        auto values = evaluateArgs(env);
        values.insert(values.begin(), member_value);
        try {
            return (*func_value)(values, env);
        }
        catch (const std::exception e) {
            runtimeError(e.what(), line, column);
        }
    } else {
        runtimeError("Object type " + getTypeStr(*member_type) + " has no member function " + ident_node->value, line, column);
    }
    return std::nullopt;
}

List FuncCallNode::evaluateArgs(Environment& env) {
    List evaluated_values;
    for (auto value_node : values) {
        auto result = value_node->evaluate(env);
        if (!result.has_value()) {
            runtimeError("Unable to evaluate function argument", line, column);
        }
        evaluated_values.push_back(result.value());
    }

    return evaluated_values;
}


const Token* Parser::getToken() const {
    return &tokens.at(token_index);
}

std::vector<std::shared_ptr<ASTNode>> Parser::parse() {
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (getToken()->type != TokenType::_EndOfFile) {
        statements.push_back(parseFoundation());
    }

    return statements;
}

void Parser::addIfElseScope() {
    last_if_else.push_back(nullptr);
}

void Parser::removeIfElseScope() {
    last_if_else.pop_back();
}

const Token* Parser::consume() {
    const Token* t{getToken()};
    if (t->type == TokenType::_EndOfFile) {
        parsingError("File ended unexpectedly!", t->line, t->column);
    }

    token_index += 1;
    return t;
}

std::optional<const Token*> Parser::peek(int ahead=1) const {
    if (token_index + ahead >= tokens.size()) {
        return &tokens.back();
    }
    else {
        return &tokens.at(token_index + ahead);
    }
}

std::string Parser::getTokenStr() const {
    return token_labels[getToken()->type];
}

bool Parser::tokenIs(std::string str) const {
    return str == getTokenStr();
}

bool Parser::nextTokenIs(std::string str, int ahead = 1) const {
    return str == token_labels[peek(ahead).value()->type];
}

std::shared_ptr<ASTNode> Parser::parseFoundation() {
    if (debug) std::cout << "parse foundation " << getTokenStr() << std::endl;
    if (keyword_tokens.contains(getTokenStr())) {
        return parseControlFlowStatement();
    }
    else  {
        bool plain_scope = tokenIs("{");
        auto statement = parseStatement(nullptr);
        if (!plain_scope && !tokenIs(";")) {
            parsingError("Expected ';' but got " + getTokenStr(), getToken()->line, getToken()->column);
        } else if (!plain_scope) {
            consume();
            return statement;
        } else {
            return statement;
        }
        return nullptr;
    }
}

std::shared_ptr<ASTNode> Parser::parseControlFlowStatement() {
    if (debug) std::cout << "parse control flow " << getTokenStr() << std::endl;
    if (peek() && peek().value()->type == TokenType::_Equals) {
        // Make sure they're not trying to use a keyword as a variable
        parsingError(getTokenStr() + " is a keyword and is not allowed to be redefined", getToken()->line, getToken()->column);
    }

    std::string t_str = getTokenStr();
    if (scoped_keyword_tokens.contains(getTokenStr())) {
        const Token* keyword = getToken();
        consume();
        std::shared_ptr<ASTNode> comparison_expr = nullptr;
        std::shared_ptr<ASTNode> for_initialization;
        auto variable_str = std::make_shared<std::string>(""); // For loop saves the string of the variable to increment
        std::shared_ptr<ASTNode> for_increment;
        std::vector<std::shared_ptr<ASTNode>> func_args;
        std::shared_ptr<ASTNode> func_name;
        auto func_str = std::make_shared<std::string>("");
        if (t_str == "if" || t_str == "elif" || t_str == "while") {
            if (tokenIs("{")) {
                parsingError("Missing boolean expression ", getToken()->line, getToken()->column);
            }

            comparison_expr = parseLogicalOr();
        } else if (t_str == "for") {
            if (tokenIs("{") || !tokenIs("ident")) {
                parsingError("Missing for loop expression", getToken()->line, getToken()->column);
            }
            for_initialization = parseStatement(variable_str);
            auto in_node = dynamic_cast<BinaryOpNode*>(for_initialization.get());
            if (in_node->op != TokenType::_In) {
                if (!tokenIs(",")) {
                    parsingError("Invalid for loop syntax", getToken()->line, getToken()->column);
                }
                consume();
                comparison_expr = parseRelation();
                if (!tokenIs(",")) {
                    parsingError("Invalid for loop syntax", getToken()->line, getToken()->column);
                }
                consume();
                auto var_test = std::make_shared<std::string>("");
                for_increment = parseStatement(var_test);
                if (*var_test != *variable_str) {
                    parsingError("For loop requires manipulation of the initialized variable", getToken()->line, getToken()->column);
                }
            }
        } else if (t_str == "func") {
            if (!tokenIs("ident")) {
                parsingError("Expected an identifier but got " + getTokenStr(), getToken()->line, getToken()->column);
            }
            func_name = parseIdentifier(func_str);
            if (!tokenIs("(")) {
                parsingError("Expected an '(' but got " + getTokenStr(), getToken()->line, getToken()->column);
            }
            consume();
            auto arg_name = std::make_shared<std::string>("");
            std::vector<std::string> arg_strings;
            while (!tokenIs(")") && !tokenIs("eof") && !tokenIs("{")) {
                if (!tokenIs("ident")) {
                    parsingError("Expected argument but got " + getTokenStr(), getToken()->line, getToken()->column);
                }
                func_args.push_back(parseIdentifier(arg_name));
                if (std::find(arg_strings.begin(), arg_strings.end(), *arg_name) == arg_strings.end()) {
                    arg_strings.push_back(*arg_name);
                    *arg_name = "";
                } else {
                    parsingError("Duplicate argument names found in function creation", getToken()->line, getToken()->column);
                }

                if (tokenIs("ident")) {
                    parsingError("Expected ',' but got argument", getToken()->line, getToken()->column);
                } else if (tokenIs(",")) {
                    consume();
                    if (!tokenIs("ident")) {
                        parsingError("Expected argument but got " + getTokenStr(), getToken()->line, getToken()->column);
                    }
                }
            }
            if (tokenIs("eof")) {
                parsingError("Missing ')'", getToken()->line, getToken()->column);
            } else if (tokenIs("{")) {
                parsingError("Missing ')' before '{'", getToken()->line, getToken()->column);
            } else {
                consume();
            }
        }

        if (!tokenIs("{")) {
            parsingError("Expected '{' but got " + getTokenStr(), getToken()->line, getToken()->column);
        }
        consume();

        // Prevent connected elif to if outside of scope
        addIfElseScope();

        std::vector<std::shared_ptr<ASTNode>> block;
        while (!tokenIs("eof") && !tokenIs("}")) {
            block.push_back(parseFoundation());
        }

        if (tokenIs("eof")) {
            parsingError("Expected '}'", getToken()->line, getToken()->column);
        }
        consume();

        removeIfElseScope();

        if (t_str == "if") {
            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword->type, nullptr, comparison_expr, block, keyword->line, keyword->column);
            last_if_else.back() = keyword_node;
            return keyword_node;
        } else if (t_str == "elif") {
            if (last_if_else.back() == nullptr) {
                parsingError("Missing 'if' before 'elif'", getToken()->line, getToken()->column);
            }

            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword->type, last_if_else.back(), comparison_expr, block, keyword->line, keyword->column);
            last_if_else.back() = keyword_node;
            return keyword_node;
        } else if (t_str == "else") {
            if (last_if_else.back() == nullptr) {
                parsingError("Missing 'if' before 'else'", getToken()->line, getToken()->column);
            }

            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword->type, last_if_else.back(), comparison_expr, block, keyword->line, keyword->column);
            last_if_else.back() = nullptr;
            return keyword_node;
        } else if (t_str == "for") {
            // If 'in' was used, only for_initialization will not be nullptr and will contain the variable and list
            return std::make_shared<ForNode>(keyword->type, for_initialization, variable_str, comparison_expr, for_increment, block, keyword->line, keyword->column);
        } else if (t_str == "func") {
            return std::make_shared<BinaryOpNode>(func_name, TokenType::_Equals, std::make_shared<FuncNode>(func_str, func_args, block, keyword->line, keyword->column), keyword->line, keyword->column);
        } else {
            return std::make_shared<ScopedNode>(keyword->type, nullptr, comparison_expr, block, keyword->line, keyword->column);
        }
    }
    else {
        auto node = parseKeyword();
        if (tokenIs(";")) {
            consume();
            return node;
        }
        else {
            parsingError("Expected ; but got " + getTokenStr(), getToken()->line, getToken()->column);
            return nullptr;
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseKeyword() {
    // Token not, and, or
    std::string t_str = getTokenStr();
    if (t_str == "not" || t_str == "and" || t_str == "or") {
        return parseLogicalOr();
    }
    else {
        const Token* token = getToken();
        std::shared_ptr<KeywordNode> node;
        if (tokenIs("return") && !nextTokenIs(";")) {
            consume();
            auto right = parseLogicalOr();
            node = std::make_shared<KeywordNode>(TokenType::_Return, right, token->line, token->column);
        }
        else if (tokenIs("import")) {
            consume();
            auto right = parseAtom();
            node = std::make_shared<KeywordNode>(TokenType::_Import, right, token->line, token->column);
        }
        else {
            node = std::make_shared<KeywordNode>(getToken()->type, nullptr, token->line, token->column);
            consume();
        }
        return node;
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseStatement(std::shared_ptr<std::string> varString = nullptr) {
    if (debug) std::cout << "parse statement " << getTokenStr() << std::endl;

    if (tokenIs("ident") && peek() && (nextTokenIs("=") || nextTokenIs("+=") || nextTokenIs("-=") || nextTokenIs("*=") || nextTokenIs("/="))) {
        auto left = parseIdentifier(varString);

        const Token* op = getToken();
        consume();
        auto right = parseLogicalOr();
        return std::make_shared<BinaryOpNode>(left, op->type, right, op->line, op->column);
    } else if (tokenIs("ident") && peek() && nextTokenIs("[")) {
        int i = 1;
        while (!nextTokenIs(";", i)) {
            if (nextTokenIs("=", i) || nextTokenIs("+=", i) || nextTokenIs("-=", i) || nextTokenIs("*=", i) || nextTokenIs("/=", i)) {
                break;
            }
            i++;
        }
        if (nextTokenIs(";", i)) {
            return parseLogicalOr();
        } else {
            auto left = parseIndexing();
            const Token* op = getToken();
            consume();
            auto right = parseLogicalOr();
            return std::make_shared<BinaryOpNode>(left, op->type, right, op->line, op->column);
        }
    }
    else {
        auto left = parseLogicalOr();
        if (auto left_list = std::dynamic_pointer_cast<ListNode>(left)) {
            if (tokenIs("=")) {
                const Token* op = getToken();
                consume();
                auto right = parseLogicalOr();
                return std::make_shared<BinaryOpNode>(left, op->type, right, op->line, op->column);
            } else {
                return left;
            }
        }
        else {
            return left;
        }
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalOr() {
    if (debug) std::cout << "parse or " << getTokenStr() << std::endl;
    auto left = parseLogicalAnd();

    while (tokenIs("or")) {
        const Token* k_word = getToken();
        consume();
        auto right = parseLogicalAnd();
        left = std::make_shared<BinaryOpNode>(left, k_word->type, right, k_word->line, k_word->column);
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseLogicalAnd() {
    if (debug) std::cout << "parse and " << getTokenStr() << std::endl;
    auto left = parseEquality();

    while (tokenIs("and")) {
        const Token* k_word = getToken();
        consume();
        auto right = parseEquality();
        left = std::make_shared<BinaryOpNode>(left, k_word->type, right, k_word->line, k_word->column);
    }
    
    return left;
}

std::shared_ptr<ASTNode> Parser::parseEquality() {
    if (debug) std::cout << "parse equality " << getTokenStr() << std::endl;
    auto left = parseRelation();

    while (tokenIs("==") || tokenIs("!=")) {
        const Token* op = getToken();
        consume();
        auto right = parseRelation();
        left = std::make_shared<BinaryOpNode>(left, op->type, right, op->line, op->column);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseRelation() {
    if (debug) std::cout << "parse relation " << getTokenStr() << std::endl;
    auto left = parseExpression();

    while (tokenIs("<") || tokenIs("<=") || tokenIs(">") || tokenIs(">=") || tokenIs("in") || (tokenIs("not") && nextTokenIs("in"))) {
        if (tokenIs("not")) {
            const Token* token = getToken();
            consume();
            const Token* op = getToken();
            consume();
            auto right = parseExpression();
            left = std::make_shared<BinaryOpNode>(left, op->type, right, op->line, op->column);
            left = std::make_shared<UnaryOpNode>(TokenType::_Not, left, token->line, token->column);
        } else {
            const Token* op = getToken();
            consume();
            auto right = parseExpression();
            left = std::make_shared<BinaryOpNode>(left, op->type, right, op->line, op->column);
        }
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    if (debug) std::cout << "parse expression " << getTokenStr() << std::endl;
    auto left = parseTerm();

    while (tokenIs("+") || tokenIs("-")) {
        const Token* op = getToken();
        consume();
        auto right = parseTerm();
        left = std::make_shared<BinaryOpNode>(left, op->type, right, op->line, op->column);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseTerm() {
    if (debug) std::cout << "parse term " << getTokenStr() << std::endl;
    auto left = parseFactor();

    while (tokenIs("*") || tokenIs("/") || tokenIs("//") || tokenIs("%")) {
        const Token* op = getToken();
        consume();
        auto right = parseFactor();
        left = std::make_shared<BinaryOpNode>(left, op->type, right, op->line, op->column);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseFactor() {
    if (debug) std::cout << "parse factor " << getTokenStr() << std::endl;
    if (tokenIs("+") || tokenIs("-")) {
        const Token* op = getToken();
        consume();
        auto right = parsePower();
        return std::make_shared<UnaryOpNode>(op->type, right, op->line, op->column);
    }
    else {
        return parsePower();
    }
}

std::shared_ptr<ASTNode> Parser::parsePower() {
    if (debug) std::cout << "parse power " << getTokenStr() << std::endl;
    auto left = parseLogicalNot();

    if (tokenIs("^") || tokenIs("**")) {
        const Token* op = getToken();
        consume();
        auto right = parseFactor();
        return std::make_shared<BinaryOpNode>(left, op->type, right, op->line, op->column);
    }
    else {
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalNot() {
    if (debug) std::cout << "parse not " << getTokenStr() << std::endl;
    if (tokenIs("not") || tokenIs("!")) {
        const Token* k_word = getToken();
        consume();
        auto right = parseMemberAccess();
        return std::make_shared<UnaryOpNode>(k_word->type, right, k_word->line, k_word->column);
    }
    else {
        return parseMemberAccess();
    }
}

std::shared_ptr<ASTNode> Parser::parseMemberAccess() {
    if (debug) std::cout << "parse member" << std::endl;
    std::shared_ptr<ASTNode> node = parseIndexing();

    while (true) {
        if (tokenIs(".")) {
            const Token* token = getToken();
            consume();
            if (tokenIs("ident")) {
                std::shared_ptr<ASTNode> right;
                if (nextTokenIs("(")) {
                    right = parseFuncCall();
                } else {
                    right = parseIdentifier();
                }
                node = std::make_shared<BinaryOpNode>(node, TokenType::_Dot, right, token->line, token->column);
            }
        } else if (tokenIs("[")) {
            node = parseIndexing(node);
        } else if (tokenIs("(")) {
            node = parseFuncCall(node);
        } else {
            break;
        }
    }

    return node;
}

std::shared_ptr<ASTNode> Parser::parseIndexing(std::shared_ptr<ASTNode> left) {
    if (debug) std::cout << "parse indexing " << getTokenStr() << std::endl;
    if (!left) {
        left = parseCollection();
    }
    while (tokenIs("[")) {
        const Token* token = getToken();
        consume();
        auto start = parseExpression(); // Assuming it ends up as an int
        if (tokenIs("]")) {
            consume();
            left = std::make_shared<IndexNode>(left, start, nullptr, token->line, token->column);
        } else if (!tokenIs(":")) {
            parsingError("Expected either ']' or ':'", getToken()->line, getToken()->column);
        } else {
            // Is :
            consume();
            auto end = parseExpression();
            if (!tokenIs("]")) {
                parsingError("Expected ']'", getToken()->line, getToken()->column);
            }
            consume();
            left = std::make_shared<IndexNode>(left, start, end, token->line, token->column);
        }
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseCollection() {
    if (debug) std::cout << "parse collection " << getTokenStr() << std::endl;
    if (tokenIs("[")) {
        const Token* token = getToken();
        consume();
        ASTList list;
        while (!tokenIs("]") && !tokenIs("eof") && !tokenIs(";")) {
            auto element = parseLogicalOr();
            list.push_back(element);
            if (tokenIs(",") && !nextTokenIs("]")) {
                consume();
            } else if (tokenIs(",")) {
                parsingError("Expected more values", getToken()->line, getToken()->column);
            }
        }
        if (tokenIs("eof") || tokenIs(";")) {
            parsingError("Expected ']'", getToken()->line, getToken()->column);
        }
        consume();
        return std::make_shared<ListNode>(list, token->line, token->column);
    }
    else if (tokenIs("{")) {
        const Token* token = getToken();
        consume();
        ASTDictionary dict;
        while (!tokenIs("}") && !tokenIs("eof") && !tokenIs(";")) {
            auto key = parseLogicalOr();
            if (!tokenIs(":")) {
                parsingError("Expected ':'", getToken()->line, getToken()->column);
            }
            consume();
            auto value = parseLogicalOr();
            dict.push_back(std::make_pair(key, value));
            if (tokenIs(",") && !nextTokenIs("}")) {
                consume();
            } else if (tokenIs(",")) {
                parsingError("Expected more values", getToken()->line, getToken()->column);
            }
        }
        if (tokenIs("eof") || tokenIs(";")) {
            parsingError("Expected '}'", getToken()->line, getToken()->column);
        }
        consume();
        return std::make_shared<DictionaryNode>(dict, token->line, token->column);
    }
    else if (tokenIs("(")) {
        const Token* token = getToken();
        consume();
        auto parse_or = parseLogicalOr();
        if (!tokenIs(")")) {
            parsingError("Expected ')' but got " + getTokenStr(), getToken()->line, getToken()->column);
        }
        consume();
        return std::make_shared<ParenthesisOpNode>(parse_or, token->line, token->column);
    }
    else {
        return parseAtom();
    }
}

std::shared_ptr<ASTNode> Parser::parseAtom() {
    if (debug) std::cout << "parse atom " << getTokenStr() << std::endl;
    if (tokenIs("integer") || tokenIs("float") || tokenIs("boolean") || tokenIs("string")) {
        const Token* token = getToken();
        if (std::holds_alternative<int>(token->value)) {
            auto int_value = std::get<int>(token->value);
            consume();
            return std::make_shared<AtomNode>(int_value, token->line, token->column);
        }
        else if (std::holds_alternative<double>(token->value)) {
            auto float_value = std::get<double>(token->value);
            consume();
            return std::make_shared<AtomNode>(float_value, token->line, token->column);
        }
        else if (std::holds_alternative<bool>(token->value)) {
            auto bool_value = std::get<bool>(token->value);
            consume();
            return std::make_shared<AtomNode>(bool_value, token->line, token->column);
        }
        else if (std::holds_alternative<std::string>(token->value)) {
            auto string_value = std::get<std::string>(token->value);
            consume();
            return std::make_shared<AtomNode>(string_value, token->line, token->column);
        }
    }
    else if (tokenIs("ident") && peek() && peek().value()->type == TokenType::_OpenParen) {
        return parseFuncCall();
    }
    else if (tokenIs("ident")) {
        return parseIdentifier(nullptr);
    }
    else if (getTokenStr().find("type:") != std::string::npos) {
        return parseKeyword();
    }
    else {
        parsingError(std::format("Expected atom but got {}", getTokenStr()), getToken()->line, getToken()->column);
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseFuncCall(std::shared_ptr<ASTNode> identifier) {
    if (debug) std::cout << "parse func call " << getTokenStr() << std::endl;
    if (!identifier) {
        if (!tokenIs("ident")) {
            parsingError("Expected function name but got " + getTokenStr(), getToken()->line, getToken()->column);
        }
        identifier = parseIdentifier(nullptr);
    }
    if (!tokenIs("(")) {
        parsingError("Missing '(' at function call", getToken()->line, getToken()->column);
    }
    consume();
    std::vector<std::shared_ptr<ASTNode>> arguments;
    while (!tokenIs(")") && !tokenIs("eof") && !tokenIs(";")) {
        arguments.push_back(parseLogicalOr());
        if (!tokenIs(")") && !tokenIs(",")) {
            parsingError("Expected ','", getToken()->line, getToken()->column);
        } else if (tokenIs(",")) {
            consume();
            if (tokenIs(")")) {
                parsingError("Expected another argument", getToken()->line, getToken()->column);
            }
        }
    }
    if (tokenIs("eof") || tokenIs(";")) {
        parsingError("Expected ')'", getToken()->line, getToken()->column);
    }
    consume();
    return std::make_shared<FuncCallNode>(identifier, arguments, identifier->line, identifier->column);
}

std::shared_ptr<ASTNode> Parser::parseIdentifier(std::shared_ptr<std::string> varString) {
    if (debug) std::cout << "parse identifier " << getTokenStr() << std::endl;
    if (tokenIs("ident")) {
        const Token* token = getToken();
        consume();
        if (std::holds_alternative<std::string>(token->value)) {
            auto ident_value = std::get<std::string>(token->value);
            if (varString != nullptr) {
                *varString = ident_value;
            }
            return std::make_shared<IdentifierNode>(ident_value, token->line, token->column);
        }
        else {
            parsingError("Attempted to create an identifier with an invalid type", token->line, token->column);
        }
    }
    else {
        parsingError(std::format("Expected identifier but got {}", getTokenStr()), getToken()->line, getToken()->column);
    }
    return nullptr;
}


////////////////////////////////////////////////////////////////
// ENVIRONMENT
////////////////////////////////////////////////////////////////

std::string getValueStr(std::shared_ptr<Value> value) {
    if (std::holds_alternative<int>(*value)) {
        return "integer";
    } else if (std::holds_alternative<double>(*value)) {
        return "float";
    } else if (std::holds_alternative<bool>(*value)) {
        return "boolean";
    } else if (std::holds_alternative<std::string>(*value)) {
        return "string";
    } else if (std::holds_alternative<std::shared_ptr<ASTNode>>(*value)) {
        return "function";
    } else if (std::holds_alternative<std::shared_ptr<List>>(*value)) {
        return "list";
    } else if (std::holds_alternative<std::shared_ptr<Dictionary>>(*value)) {
        return "dictionary";
    } else if (std::holds_alternative<std::shared_ptr<BuiltInFunction>>(*value)) {
        return "built-in function";
    } else if (std::holds_alternative<ValueType>(*value)) {
        return "type";
    } else {
        throw std::runtime_error("Attempted to get string of unrecognized type.");
    }
    return "";
}

ValueType getValueType(std::shared_ptr<Value> value) {
    if (std::holds_alternative<int>(*value)) {
        return ValueType::Integer;
    } else if (std::holds_alternative<double>(*value)) {
        return ValueType::Float;
    } else if (std::holds_alternative<bool>(*value)) {
        return ValueType::Boolean;
    } else if (std::holds_alternative<std::string>(*value)) {
        return ValueType::String;
    } else if (std::holds_alternative<std::shared_ptr<ASTNode>>(*value)) {
        return ValueType::Function;
    } else if (std::holds_alternative<std::shared_ptr<List>>(*value)) {
        return ValueType::List;
    } else if (std::holds_alternative<std::shared_ptr<Dictionary>>(*value)) {
        return ValueType::Dictionary;
    } else if (std::holds_alternative<std::shared_ptr<BuiltInFunction>>(*value)) {
        return ValueType::BuiltInFunction;
    } else if (std::holds_alternative<ValueType>(*value)) {
        return ValueType::Type;
    } else {
        throw std::runtime_error("Attempted to get type of unrecognized type.");
        return ValueType::Null;
    }
}

std::string getTypeStr(ValueType value) {
    if (value == ValueType::Integer) {
        return "Type:Integer";
    } else if (value == ValueType::Float) {
        return "Type:Float";
    } else if (value == ValueType::Boolean) {
        return "Type:Bool";
    } else if (value == ValueType::String) {
        return "Type:String";
    } else if (value == ValueType::List) {
        return "Type:List";
    } else if (value == ValueType::Dictionary) {
        return "Type:Dictionary";
    } else if (value == ValueType::Function) {
        return "Type:Function";
    } else if (value == ValueType::BuiltInFunction) {
        return "Type:BuiltInFunction";
    } else if (value == ValueType::Null) {
        return "Type:Null";
    } else if (value == ValueType::Type) {
        return "Type:Type";
    }
    throw std::runtime_error("Reached end of type str.");
    return "";
}

void Scope::set(const std::string variable, std::shared_ptr<Value> value) {
    variables[variable] = value;
}

std::shared_ptr<Value> Scope::get(const std::string variable) const {
    auto element = variables.find(variable);
    if (element != variables.end()) {
        return element->second;
    } else {
        throw std::runtime_error(std::format("Unrecognized variable {}.", variable));
    }
}

bool Scope::has(const std::string variable) const {
    return variables.find(variable) != variables.end();
}

// Method to display the contents of the environment
void Scope::display() const {
    for (const std::pair<const std::string, std::shared_ptr<Value>> pair : variables) {
        const std::string& name = pair.first;
        std::shared_ptr<Value> value = pair.second;

        std::cout << name << " = ";
        std::visit([](const auto& v) { std::cout << v; }, *value.get());
        std::cout << std::endl;
    }
}


void Environment::addScope() {
    scopes.push_back(Scope());
}

void Environment::removeScope() {
    scopes.pop_back();
}

void Environment::set(const std::string variable, std::shared_ptr<Value> value) {
    if (scopes.empty()) {
        throw std::runtime_error("Attempted to access empty environment.");
    }
    for (int i = scopes.size() - 1; i > -1; i--) {
        if (scopes.at(i).has(variable)) {
            scopes.at(i).set(variable, value);
            return;
        }
    }

    scopes.back().set(variable, value);
}

// Method to display the contents of the environment
void Environment::display() const {
    if (scopes.empty()) {
        throw std::runtime_error("No environment to display.");
    }

    for (auto scope : scopes) {
        scope.display();
    }
}

std::shared_ptr<Value> Environment::get(const std::string variable) const {
    if (scopes.empty()) {
        throw std::runtime_error("Attempted to access empty environment.");
    }
    for (const auto scope : scopes) {
        if (scope.has(variable)) {
            return scope.get(variable);
        }
    }

    throw std::runtime_error(std::format("Unrecognized variable {}.", variable));
}

bool Environment::has(const std::string variable) const {
    if (scopes.empty()) {
        throw std::runtime_error("Attempted to access empty environment.");
    }
    for (const auto scope : scopes) {
        if (scope.has(variable)) {
            return true;
        }
    }

    return false;
}

int Environment::scopeDepth() const {
    return scopes.size();
}

void Environment::addLoop() {
    loop_depth += 1;
}

void Environment::removeLoop() {
    loop_depth -= 1;
}

bool Environment::inLoop() const {
    return loop_depth > 0;
}

void Environment::resetLoop() {
    loop_depth = 0;
}

void Environment::addFunction(const std::string& name, std::shared_ptr<Value> func) {
    built_in_functions[name] = func;
    built_in_names[func] = name;
}

std::shared_ptr<Value> Environment::getFunction(const std::string& name) const {
    auto func = built_in_functions.find(name);
    if (func != built_in_functions.end()) {
        return func->second;
    }

    throw std::runtime_error("Unrecognized built-in function: " + name);
}

std::string Environment::getName(const std::shared_ptr<Value> func) const {
    auto name = built_in_names.find(func);
    if (name != built_in_names.end()) {
        return name->second;
    }

    throw std::runtime_error("Unrecognized built-in name.");
}

bool Environment::hasFunction(const std::string& name) const {
    auto func = built_in_functions.find(name);
    return func != built_in_functions.end();
}

void Environment::addMember(ValueType type, const std::string& name, std::shared_ptr<Value> func) {
    member_functions[type][name] = func;
    built_in_names[func] = getTypeStr(type) + "." + name;
}

std::shared_ptr<Value> Environment::getMember(std::shared_ptr<ValueType> type, const std::string& name) const {
    auto members = member_functions.find(*type);
    if (members != member_functions.end()) {
        auto func = members->second.find(name);
        if (func != members->second.end()) {
            return func->second;
        }
    }

    throw std::runtime_error("Unrecognized member function: " + name);
}

bool Environment::hasMember(std::shared_ptr<ValueType> type, const std::string& name) const {
    auto members = member_functions.find(*type);
    if (members != member_functions.end()) {
        auto func = members->second.find(name);
        return func != members->second.end();
    }
    return false;
}