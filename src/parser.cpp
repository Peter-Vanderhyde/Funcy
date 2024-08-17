#include "parser.h"


void handleError(const std::string& message, int line, int column) {
    if (line > 0 && column > 0) {
        throw std::runtime_error(std::format("{} at line {} column {}", message, line, column));
    } else {
        throw std::runtime_error(message);
    }
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
                } else if constexpr (std::is_same_v<T, std::shared_ptr<List>>) {
                    if (v) {
                        os << *v;  // Recursively print the inner list
                    } else {
                        os << "null";
                    }
                } else {
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


void printValue(const std::shared_ptr<Value> value) {
    if (auto int_value = std::get_if<int>(value.get())) {
        std::cout << *int_value;
    } else if (auto double_value = std::get_if<double>(value.get())) {
        std::cout << *double_value;
    } else if (auto bool_value = std::get_if<bool>(value.get())) {
        std::cout << std::boolalpha << *bool_value;
    } else if (std::holds_alternative<std::string>(*value)) {
        auto string_value = std::get<std::string>(*value);
        std::cout << string_value;
    } else if (auto list_value = std::get<std::shared_ptr<List>>(*value)) {
        std::cout << *list_value;
    } else {
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
        handleError("Unable to evaluate value.", 0, 0);
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
        throw std::runtime_error(value + " is not defined.");
        return {};
    }
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
                evaluated_list.push_back(nullptr); // If evaluation fails, store null
            }
        } else {
            throw std::runtime_error("List contained a nullptr pointing to an ASTNode."); // Null ASTNode pointer
        }
    }

    return std::make_shared<Value>(std::make_shared<List>(evaluated_list));
}

std::optional<std::shared_ptr<Value>> IndexNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate index" << std::endl;
    if (auto atom_node = std::dynamic_pointer_cast<AtomNode>(container)) {
        // Make sure it's a string
        auto eval = atom_node->evaluate(env);
        if (eval) {
            if (auto string_val = std::make_shared<std::string>(std::get<std::string>(*eval.value()))) {
                return getIndex(env, string_val);
            } else {
                throw std::runtime_error("Atom evaluation did not return a string for indexing.");
            }
        } else {
            throw std::runtime_error("Was unable to evaluate the atom.");
        }
    } else if (auto list_node = std::dynamic_pointer_cast<ListNode>(container)) {
        auto eval = list_node->evaluate(env);
        if (eval) {
            if (auto list_val = std::get<std::shared_ptr<List>>(*eval.value())) {
                return getIndex(env, list_val);
            } else {
                throw std::runtime_error("List evaluation did not return a list for indexing.");
            }
        } else {
            throw std::runtime_error("Was unable to evaluate the list.");
        }
    } else if (auto ident_node = std::dynamic_pointer_cast<IdentifierNode>(container)) {
        auto eval = ident_node->evaluate(env);
        if (eval) {
            if (std::holds_alternative<std::string>(*eval.value())) {
                auto str_val = std::get<std::string>(*eval.value());
                return getIndex(env, std::make_shared<std::string>(str_val));
            } else if (std::holds_alternative<std::shared_ptr<List>>(*eval.value())) {
                auto list_val = std::get<std::shared_ptr<List>>(*eval.value());
                return getIndex(env, list_val);
            } else {
                throw std::runtime_error("Identifier evaluation did not return a list or string for indexing.");
            }
        } else {
            throw std::runtime_error("Was unable to evaluate the identifier.");
        }
    } else if (auto index_node = std::dynamic_pointer_cast<IndexNode>(container)) {
        auto eval = index_node->evaluate(env);
        if (eval) {
            if (auto list_val = std::get<std::shared_ptr<List>>(*eval.value())) {
                return getIndex(env, list_val);
            } else {
                throw std::runtime_error("Index evaluation did not return a list for indexing.");
            }
        } else {
            throw std::runtime_error("Was unable to evaluate the index.");
        }
    }
    else {
        throw std::runtime_error("Index node container was an unexpected type.");
    }
    return std::nullopt;
}

std::variant<char, std::shared_ptr<Value>> getAtIndex(std::variant<std::shared_ptr<std::string>, std::shared_ptr<List>> listr, int index) {
    if (std::holds_alternative<std::shared_ptr<std::string>>(listr)) {
        auto string = std::get<std::shared_ptr<std::string>>(listr);
        if (index >= 0 && index < string->length()) {
            return string->at(index);
        } else if (index >= string->length() * -1 && index < 0) {
            return string->at(string->length() - -index);
        } else {
            throw std::runtime_error("String index out of range.");
        }
    } else if (std::holds_alternative<std::shared_ptr<List>>(listr)) {
        auto list = std::get<std::shared_ptr<List>>(listr);
        if (index >= 0 && index < list->size()) {
            return list->at(index);
        } else if (index >= list->size() * -1 && index < 0) {
            return list->at(list->size() - -index);
        } else {
            throw std::runtime_error("List index out of range.");
        }
    } else {
        throw std::runtime_error("getAtIndex did not receive a string or list.");
    }
    return nullptr;
}

std::optional<std::shared_ptr<Value>> IndexNode::getIndex(Environment& env,
                                                    std::variant<std::shared_ptr<std::string>, std::shared_ptr<List>> listr) {
    if (end_index) {
        auto start_result = start_index->evaluate(env);
        auto end_result = end_index->evaluate(env);
        if (start_result && end_result) {
            if (std::holds_alternative<int>(*start_result.value())) {
                int start_val = std::get<int>(*start_result.value());
                if (std::holds_alternative<int>(*end_result.value())) {
                    int end_val = std::get<int>(*end_result.value());
                    if (start_val <= end_val) {
                        if (std::holds_alternative<std::shared_ptr<std::string>>(listr)) {
                            if (start_val < 0 && end_val >= 0) {
                                return std::make_shared<Value>("");
                            }

                            std::string sub_str;
                            for (int i = start_val; i < end_val; i++) {
                                char c = std::get<char>(getAtIndex(listr, i));
                                sub_str += c;
                            }
                            return std::make_shared<Value>(sub_str);
                        } else if (std::holds_alternative<std::shared_ptr<List>>(listr)){
                            std::shared_ptr<List> list = std::make_shared<List>();
                            if (start_val < 0 && end_val >= 0) {
                                return std::make_shared<Value>(list);
                            }
                            
                            for (int i = start_val; i < end_val; i++) {
                                auto list_val = std::get<std::shared_ptr<Value>>(getAtIndex(listr, i));
                                list->push_back(list_val);
                            }
                            return std::make_shared<Value>(list);
                        } else {
                            throw std::runtime_error("Invalid type for getting index.");
                        }
                    } else {
                        throw std::runtime_error("Invalid format of slice indeces. The start index is larger than the end.");
                    }
                } else {
                    throw std::runtime_error("The ending index value was not an int.");
                }
            } else {
                throw std::runtime_error("The starting index value was not an int.");
            }
        } else {
            throw std::runtime_error("Failed to evaluate start_index or end_index.");
        }
    } else {
        auto result = start_index->evaluate(env);
        if (result) {
            if (std::holds_alternative<int>(*result.value())) {
                int int_val = std::get<int>(*result.value());
                if (std::holds_alternative<std::shared_ptr<std::string>>(listr)) {
                    auto get_char = getAtIndex(listr, int_val);
                    if (std::holds_alternative<char>(get_char)) {
                        auto c = std::get<char>(get_char);
                        std::string str = std::string(1, c);
                        return std::make_shared<Value>(str);
                    }
                } else {
                    auto value = std::get<std::shared_ptr<Value>>(getAtIndex(listr, int_val));
                    return value;
                }
            } else {
                throw std::runtime_error("The index was not given an int.");
            }
        } else {
            throw std::runtime_error("Failed to evaluate start_index.");
            return std::nullopt;
        }
    }
    return std::nullopt;
}

void setAtIndex(std::shared_ptr<List> env_list, int list_index, std::shared_ptr<Value> value) {
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
        } else if (auto list_val = std::get<std::shared_ptr<List>>(*value)) {
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
}

void IndexNode::assignIndex(Environment& env, std::shared_ptr<Value> value) {
    auto eval = container->evaluate(env);
    std::shared_ptr<Value> env_val;
    if (eval) {
        env_val = eval.value();
    } else {
        throw std::runtime_error("Index assignment unable to evaluate the container.");
        return;
    }
    if (!std::holds_alternative<std::shared_ptr<List>>(*env_val)) {
        throw std::runtime_error(getValueStr(env_val) + " object does not support item assignment.");
        return;
    }
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
                    throw std::runtime_error("Index assignment out of range.");
                }
            } else {
                throw std::runtime_error("Index assignment requires int.");
            }
        } else {
            throw std::runtime_error("Failed to evaluate assignment index");
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
                            // throw std::runtime_error("Assignment index went out of range.");
                        }
                        env_list->erase(env_list->begin() + start_val);
                    }
                    setAtIndex(env_list, start_val, value);
                } else {
                    throw std::runtime_error("Assignment index end value is not an int.");
                }
            } else {
                throw std::runtime_error("Assignment index start value is not an int.");
            }
        } else {
            throw std::runtime_error("Assignment index was unable to evaluate.");
        }
    }
}


template <typename T1, typename T2>
std::optional<std::shared_ptr<Value>> doArithmetic(const T1 lhs, const T2 rhs, const TokenType op) {
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
            } else if constexpr (std::is_same_v<T, List>) {
                return !v.empty();
            }
            return false;
        }, value);
    };

    // Handle AND OR for all type cases
    if (op == TokenType::_And) return std::make_shared<Value>(checkTruthy(lhs) && checkTruthy(rhs));
    else if (op == TokenType::_Or) return std::make_shared<Value>(checkTruthy(lhs) || checkTruthy(rhs));

    if constexpr (std::is_same_v<T1, std::shared_ptr<List>> && std::is_same_v<T2, std::shared_ptr<List>>) {
        // BOTH ARE LISTS
        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) {
            std::shared_ptr<List> result = lhs;  // Start with a copy of the first list
            result->insert(result->end(), rhs->begin(), rhs->end());  // Append the second list
            return std::make_shared<Value>(result);
        }
        else if (op == TokenType::_Compare) return std::make_shared<Value>(*lhs == *rhs);
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(*lhs != *rhs);
    }
    else if constexpr (std::is_same_v<T1, std::string> && std::is_same_v<T2, std::string>) {
        // BOTH STRINGS
        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) return std::make_shared<Value>(lhs + rhs);
        else if (op == TokenType::_Compare) return std::make_shared<Value>(lhs == rhs);
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(lhs != rhs);
    }
    else if constexpr ((std::is_same_v<T1, int> || std::is_same_v<T1, bool> || std::is_same_v<T1, double>) &&
                       (std::is_same_v<T2, int> || std::is_same_v<T2, bool> || std::is_same_v<T2, double>)) {
        // Explicitly handle mixed types
        double lhs_double = static_cast<double>(lhs);
        double rhs_double = static_cast<double>(rhs);

        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) return std::make_shared<Value>(lhs_double + rhs_double);
        else if (op == TokenType::_Minus || op == TokenType::_MinusEquals) return std::make_shared<Value>(lhs_double - rhs_double);
        else if (op == TokenType::_Multiply || op == TokenType::_MultiplyEquals) return std::make_shared<Value>(lhs_double * rhs_double);
        else if (op == TokenType::_Divide || op == TokenType::_DivideEquals) {
            if (rhs_double == 0) throw std::runtime_error("Attempted division by 0.");
            return std::make_shared<Value>(lhs_double / rhs_double);
        }
        else if (op == TokenType::_FloorDiv) {
            if (rhs_double == 0) throw std::runtime_error("Attempted division by 0.");
            return std::make_shared<Value>(static_cast<int>(lhs_double / rhs_double));
        }
        else if (op == TokenType::_Mod) {
            if (rhs_double == 0) throw std::runtime_error("Attempted division by 0.");
            if constexpr (std::is_same_v<T1, double> || std::is_same_v<T2, double>) {
                throw std::runtime_error("The modulus '%' can only be performed on ints.");
            } else {
                return std::make_shared<Value>(static_cast<int>(lhs) % static_cast<int>(rhs));
            }
        }
        else if (op == TokenType::_Caret || op == TokenType::_DoubleMultiply) return std::make_shared<Value>(std::pow(lhs_double, rhs_double));
        else if (op == TokenType::_Compare) return std::make_shared<Value>(lhs_double == rhs_double);
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(lhs_double != rhs_double);
        else if (op == TokenType::_GreaterThan) return std::make_shared<Value>(lhs_double > rhs_double);
        else if (op == TokenType::_GreaterEquals) return std::make_shared<Value>(lhs_double >= rhs_double);
        else if (op == TokenType::_LessThan) return std::make_shared<Value>(lhs_double < rhs_double);
        else if (op == TokenType::_LessEquals) return std::make_shared<Value>(lhs_double <= rhs_double);
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> BinaryOpNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate binary" << std::endl;
    // Postpone evaluating left
    std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);

    if (op == TokenType::_Equals) {
        // An equals is a special case
        if (!right_value.has_value()) {
            throw std::runtime_error("Failed to set variable. Operand could not be computed.");
        }

        // Give it the actual left string, not the value of the variable
        if (auto identifierNode = dynamic_cast<IdentifierNode*>(left.get())) {
            env.set(identifierNode->value, right_value.value());
        } else if (auto indexNode = dynamic_cast<IndexNode*>(left.get())) {
            indexNode->assignIndex(env, right_value.value());
        }
        else {
            throw std::runtime_error("The operator '=' can only be used with variables.");
        }
    }
    else {
        std::optional<std::shared_ptr<Value>> left_value = left->evaluate(env);
        // Arithmetic operations need two values to operate on
        if (!left_value.has_value() || !right_value.has_value()) {
            throw std::runtime_error(std::format("Failed to evaluate expression with operator '{}': one or both operands could not be computed.",
                                            token_labels[op]));
        }

        if (getValueStr(left_value.value()) == "function" || getValueStr(right_value.value()) == "function") {
            throw std::runtime_error(std::format("Unsupported operand types for operation. operation was {} '{}' {}",
                                                getValueStr(left_value.value()), token_labels[op], getValueStr(right_value.value())));
        }

        // Perform arithmetic operation
        auto result = std::visit([&](auto lhs) -> std::optional<std::shared_ptr<Value>> {
            return std::visit([&](auto rhs) -> std::optional<std::shared_ptr<Value>> {
                return doArithmetic(lhs, rhs, op);
            }, *right_value.value());
        }, *left_value.value());

        if (!result.has_value()) {
            throw std::runtime_error(std::format("Unsupported operand types for operation. operation was {} '{}' {}",
                                                getValueStr(left_value.value()), token_labels[op], getValueStr(right_value.value())));
        }

        if (op == TokenType::_PlusEquals || op == TokenType::_MinusEquals || op == TokenType::_MultiplyEquals || op == TokenType::_DivideEquals) {
            // Handle setting +=, -= etc.
            if (auto identifierNode = dynamic_cast<IdentifierNode*>(left.get())) {
                env.set(identifierNode->value, result.value());
            }
            else {
                throw std::runtime_error("The operator '=' can only be used with variables.");
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
        throw std::runtime_error(std::format("Failed to evaluate expression with operator '{}': the operand could not be computed.",
                                         token_labels[op]));
    }

    std::string RHS = getValueStr(right_value.value());

    if (RHS == "int") {
        if (op == TokenType::_Minus) {
            int rhs = std::get<int>(*right_value.value());
            return std::make_shared<Value>(-rhs);
        }
        else if (op == TokenType::_Plus) {
            return right_value;
        }
        else if (op == TokenType::_Not) {
            int rhs = std::get<int>(*right_value.value());
            return std::make_shared<Value>(rhs == 0);
        }
    }
    else if (RHS == "double") {
        if (op == TokenType::_Minus) {
            double rhs = std::get<double>(*right_value.value());
            return std::make_shared<Value>(-rhs);
        }
        else if (op == TokenType::_Plus) {
            return right_value;
        }
        else if (op == TokenType::_Not) {
            double rhs = std::get<double>(*right_value.value());
            return std::make_shared<Value>(rhs == 0.0);
        }
    }
    else if (RHS == "bool") {
        if (op == TokenType::_Not) {
            bool rhs = std::get<bool>(*right_value.value());
            return std::make_shared<Value>(!rhs);
        }
    }

    throw std::runtime_error(std::format("Unsupported operand types for operation. operation was '{}' {}", token_labels[op], RHS));
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
        // if (result) {
        //     printValue(result.value());
        // }
    }

    env.removeScope();
    return std::nullopt;
}

bool ScopedNode::getComparisonValue(Environment& env) const {
    auto result = comparison->evaluate(env);
    if (result && getValueStr(result.value()) == "bool") {
        if (auto bool_value = std::get_if<bool>(result.value().get())) {
            return *bool_value;
        } else {
            throw std::runtime_error("Boolean weirdness.");
        }
    }
    else {
        throw std::runtime_error("Syntax Error: Missing a boolean comparison for keyword to evaluate.");
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
                        // if (result) {
                        //     printValue(result.value());
                        // }
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
                // if (result) {
                //     printValue(result.value());
                // }
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
    initialization->evaluate(env);

    int variable;
    if (std::holds_alternative<int>(*env.get(*init_string))) {
        auto i = std::get<int>(*env.get(*init_string));
        variable = i;
    }
    else {
        throw std::runtime_error("For loop requires int variable.");
    }

    while (true) {
        auto cond_value = condition_value->evaluate(env);
        if (!cond_value) {
            throw std::runtime_error("Unable to evaluate for loop condition.");
        }

        if (auto bool_value = std::get_if<bool>(cond_value.value().get())) {
            if (!*bool_value) break;
        } else {
            throw std::runtime_error("For loop requires boolean condition.");
        }

        int original_variable = variable;

        try {
            for (auto statement : block) {
                auto result = statement->evaluate(env);
                // if (result) {
                //     printValue(result.value());
                // }
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
            throw std::runtime_error("Break used outside of loop.");
        }
    } else if (keyword == TokenType::_Continue) {
        if (env.inLoop()) {
            throw ContinueException();
        }
        else {
            throw std::runtime_error("Continue used outside of loop.");
        }
    }
    else if (keyword == TokenType::_Return) {
        if (right != nullptr) {
            throw ReturnException(right->evaluate(env));
        } else {
            throw ReturnException(std::nullopt);
        }
    }

    if (right) {
        return right->evaluate(env);
    } else {
        return {};
    }
}

std::optional<std::shared_ptr<Value>> FuncNode::evaluate(Environment& env) {
    if (debug) std::cout << "evaluate function" << std::endl;
    return std::make_shared<Value>(std::static_pointer_cast<ASTNode>(std::make_shared<FuncNode>(*this)));
}

void FuncNode::setArgs(std::vector<std::shared_ptr<Value>> values, Environment& base_env, Environment& local_env) {
    if (values.size() != args.size()) {
        throw std::runtime_error(std::format("Incorrect number of args were passed in. {} instead of {}", values.size(), args.size()));
    }

    for (int i = 0; i < values.size(); i++) {
        if (auto ident_node = dynamic_cast<IdentifierNode*>(args.at(i).get())) {
            std::string arg_string = ident_node->value;
            local_env.set(arg_string, values.at(i));
        } else {
            throw std::runtime_error("Unable to convert identifier for function argument.");
        }
    }

    return;
}

std::optional<std::shared_ptr<Value>> FuncNode::callFunc(List values, Environment& env) {
    Environment local_env = env;
    while (local_env.inLoop()) {
        local_env.removeLoop();
    }
    local_env.addScope();
    setArgs(values, env, local_env);
    for (auto statement : block) {
        try {
            if (auto func_statement = dynamic_cast<FuncCallNode*>(statement.get())) {
                func_statement->base_env = std::make_shared<Environment>(env);
                auto result = func_statement->evaluate(local_env);
                // if (result) {
                //     printValue(result.value());
                // }
            } else {
                auto result = statement->evaluate(local_env);
                // if (result) {
                //     printValue(result.value());
                // }
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
            throw std::runtime_error("Unable to call function " + dynamic_cast<IdentifierNode*>(identifier.get())->value + ".");
        }
    } else if (std::holds_alternative<std::shared_ptr<BuiltInFunction>>(*mapped_value)) {
        auto func_value = std::get<std::shared_ptr<BuiltInFunction>>(*mapped_value);
        return (*func_value)(evaluateArgs(env));
    } else {
        throw std::runtime_error("Object type " + getValueStr(mapped_value) + " is not callable");
    }
    return std::nullopt;
}

List FuncCallNode::evaluateArgs(Environment& env) {
    List evaluated_values;
    for (auto value_node : values) {
        auto result = value_node->evaluate(env);
        if (!result) {
            throw std::runtime_error("Unable to evalute function argument");
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
        handleError("File ended unexpectedly!", t->line, t->column);
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
            handleError("Expected ';' but got " + getTokenStr(), getToken()->line, getToken()->column);
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
        handleError(getTokenStr() + " is a keyword and is not allowed to be redefined", getToken()->line, getToken()->column);
    }

    std::string t_str = getTokenStr();
    if (scoped_keyword_tokens.contains(getTokenStr())) {
        TokenType keyword = getToken()->type;
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
                handleError("Syntax Error: Missing boolean expression ", getToken()->line, getToken()->column);
            }

            comparison_expr = parseLogicalOr();
        } else if (t_str == "for") {
            if (tokenIs("{") || !tokenIs("ident")) {
                handleError("Syntax Error: Missing for loop expression", getToken()->line, getToken()->column);
            }
            for_initialization = parseStatement(variable_str);
            // if (!tokenIs("in")) {
            //     handleError("Syntax Error: For loop missing 'in' keyword", getToken()->line, getToken()->column);
            // }
            // consume();
            if (!tokenIs(",")) {
                handleError("Syntax Error: Invalid for loop syntax", getToken()->line, getToken()->column);
            }
            consume();
            comparison_expr = parseRelation();
            if (!tokenIs(",")) {
                handleError("Syntax Error: Invalid for loop syntax", getToken()->line, getToken()->column);
            }
            consume();
            auto var_test = std::make_shared<std::string>("");
            for_increment = parseStatement(var_test);
            if (*var_test != *variable_str) {
                handleError("For loop requires manipulation of the initialized variable", getToken()->line, getToken()->column);
            }
        } else if (t_str == "func") {
            if (!tokenIs("ident")) {
                handleError("Expected an identifier but got " + getTokenStr(), getToken()->line, getToken()->column);
            }
            func_name = parseIdentifier(func_str);
            if (!tokenIs("(")) {
                handleError("Expected an '(' but got " + getTokenStr(), getToken()->line, getToken()->column);
            }
            consume();
            auto arg_name = std::make_shared<std::string>("");
            std::vector<std::string> arg_strings;
            while (!tokenIs(")") && !tokenIs("eof") && !tokenIs("{")) {
                if (!tokenIs("ident")) {
                    handleError("Expected argument but got " + getTokenStr(), getToken()->line, getToken()->column);
                }
                func_args.push_back(parseIdentifier(arg_name));
                if (std::find(arg_strings.begin(), arg_strings.end(), *arg_name) == arg_strings.end()) {
                    arg_strings.push_back(*arg_name);
                    *arg_name = "";
                } else {
                    handleError("Duplicate argument names found in function creation", getToken()->line, getToken()->column);
                }

                if (tokenIs("ident")) {
                    handleError("Expected ',' but got argument", getToken()->line, getToken()->column);
                } else if (tokenIs(",")) {
                    consume();
                    if (!tokenIs("ident")) {
                        handleError("Expected argument but got " + getTokenStr(), getToken()->line, getToken()->column);
                    }
                }
            }
            if (tokenIs("eof")) {
                handleError("Missing ')'", getToken()->line, getToken()->column);
            } else if (tokenIs("{")) {
                handleError("Missing ')' before '{'", getToken()->line, getToken()->column);
            } else {
                consume();
            }
        }

        if (!tokenIs("{")) {
            handleError("Syntax Error: Expected '{' but got " + getTokenStr(), getToken()->line, getToken()->column);
        }
        consume();

        // Prevent connected elif to if outside of scope
        addIfElseScope();

        std::vector<std::shared_ptr<ASTNode>> block;
        while (!tokenIs("eof") && !tokenIs("}")) {
            block.push_back(parseFoundation());
        }

        if (tokenIs("eof")) {
            throw std::runtime_error("Syntax Error: Expected '}'.");
        }
        consume();

        removeIfElseScope();

        if (t_str == "if") {
            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword, nullptr, comparison_expr, block);
            last_if_else.back() = keyword_node;
            return keyword_node;
        } else if (t_str == "elif") {
            if (last_if_else.back() == nullptr) {
                handleError("Missing 'if' before 'elif'", getToken()->line, getToken()->column);
            }

            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword, last_if_else.back(), comparison_expr, block);
            last_if_else.back() = keyword_node;
            return keyword_node;
        } else if (t_str == "else") {
            if (last_if_else.back() == nullptr) {
                handleError("Missing 'if' before 'else'", getToken()->line, getToken()->column);
            }

            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword, last_if_else.back(), comparison_expr, block);
            last_if_else.back() = nullptr;
            return keyword_node;
        } else if (t_str == "for") {
            return std::make_shared<ForNode>(keyword, for_initialization, variable_str, comparison_expr, for_increment, block);
        } else if (t_str == "func") {
            return std::make_shared<BinaryOpNode>(func_name, TokenType::_Equals, std::make_shared<FuncNode>(func_args, block));
        } else {
            return std::make_shared<ScopedNode>(keyword, nullptr, comparison_expr, block);
        }
    }
    else {
        // Token not, and, or
        if (t_str == "not" || t_str == "and" || t_str == "or") {
            return parseLogicalOr();
        }
        else {
            std::shared_ptr<KeywordNode> node;
            if (tokenIs("return") && !nextTokenIs(";")) {
                consume();
                auto right = parseLogicalOr();
                node = std::make_shared<KeywordNode>(TokenType::_Return, right);
            }
            else {
                node = std::make_shared<KeywordNode>(getToken()->type);
                consume();
            }
            if (tokenIs(";")) {
                consume();
                return node;
            }
            else {
                handleError("Expected ; but got " + getTokenStr(), getToken()->line, getToken()->column);
                return nullptr;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseStatement(std::shared_ptr<std::string> varString = nullptr) {
    if (debug) std::cout << "parse statement " << getTokenStr() << std::endl;

    if (tokenIs("ident") && peek() && (nextTokenIs("=") || nextTokenIs("+=") || nextTokenIs("-=") || nextTokenIs("*=") || nextTokenIs("/="))) {
        auto left = parseIdentifier(varString);

        TokenType op = getToken()->type;
        consume();
        auto right = parseLogicalOr();
        return std::make_shared<BinaryOpNode>(left, op, right);
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
            TokenType op = getToken()->type;
            consume();
            auto right = parseLogicalOr();
            return std::make_shared<BinaryOpNode>(left, op, right);
        }
    }
    else {
        return parseLogicalOr();
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalOr() {
    if (debug) std::cout << "parse or " << getTokenStr() << std::endl;
    auto left = parseLogicalAnd();

    if (tokenIs("or")) {
        TokenType k_word = TokenType::_Or;
        consume();
        auto right = parseLogicalAnd();
        return std::make_shared<BinaryOpNode>(left, k_word, right);
    }
    else {
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalAnd() {
    if (debug) std::cout << "parse and " << getTokenStr() << std::endl;
    auto left = parseEquality();

    if (tokenIs("and")) {
        TokenType k_word = TokenType::_And;
        consume();
        auto right = parseEquality();
        return std::make_shared<BinaryOpNode>(left, k_word, right);
    }
    else {
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseEquality() {
    if (debug) std::cout << "parse equality " << getTokenStr() << std::endl;
    auto left = parseRelation();

    while (tokenIs("==") || tokenIs("!=")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseRelation();
        left = std::make_shared<BinaryOpNode>(left, op, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseRelation() {
    if (debug) std::cout << "parse relation " << getTokenStr() << std::endl;
    auto left = parseExpression();

    while (tokenIs("<") || tokenIs("<=") || tokenIs(">") || tokenIs(">=")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseExpression();
        left = std::make_shared<BinaryOpNode>(left, op, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    if (debug) std::cout << "parse expression " << getTokenStr() << std::endl;
    auto left = parseTerm();

    while (tokenIs("+") || tokenIs("-")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseTerm();
        left = std::make_shared<BinaryOpNode>(left, op, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseTerm() {
    if (debug) std::cout << "parse term " << getTokenStr() << std::endl;
    auto left = parseFactor();

    while (tokenIs("*") || tokenIs("/") || tokenIs("//") || tokenIs("%")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseFactor();
        left = std::make_shared<BinaryOpNode>(left, op, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseFactor() {
    if (debug) std::cout << "parse factor " << getTokenStr() << std::endl;
    if (tokenIs("+") || tokenIs("-")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parsePower();
        return std::make_shared<UnaryOpNode>(op, right);
    }
    else {
        return parsePower();
    }
}

std::shared_ptr<ASTNode> Parser::parsePower() {
    if (debug) std::cout << "parse power " << getTokenStr() << std::endl;
    auto left = parseLogicalNot();

    if (tokenIs("^") || tokenIs("**")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseFactor();
        return std::make_shared<BinaryOpNode>(left, op, right);
    }
    else {
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalNot() {
    if (debug) std::cout << "parse not " << getTokenStr() << std::endl;
    if (tokenIs("not") || tokenIs("!")) {
        TokenType k_word = TokenType::_Not;
        consume();
        auto right = parsePrimary();
        return std::make_shared<UnaryOpNode>(k_word, right);
    }
    else {
        return parsePrimary();
    }
}

std::shared_ptr<ASTNode> Parser::parsePrimary() {
    if (debug) std::cout << "parse primary " << getTokenStr() << std::endl;
    if (tokenIs("(")) {
        consume();
        auto parse_or = parseLogicalOr();
        if (!tokenIs(")")) {
            handleError("Expected ')' but got " + getTokenStr(), getToken()->line, getToken()->column);
        }
        consume();
        return std::make_shared<ParenthesisOpNode>(parse_or);
    }
    else if (tokenIs("{")) {
        consume();
        std::vector<std::shared_ptr<ASTNode>> block;
        while (!tokenIs("eof") && !tokenIs("}")) {
            block.push_back(parseFoundation());
        }
        if (!tokenIs("}")) {
            handleError("Expected '}'" + getTokenStr(), getToken()->line, getToken()->column);
        }
        consume();
        return std::make_shared<ScopeNode>(block);
    }
    else {
        return parseIndexing();
    }
}

std::shared_ptr<ASTNode> Parser::parseIndexing() {
    if (debug) std::cout << "parse indexing " << getTokenStr() << std::endl;
    auto left = parseCollection();
    while (tokenIs("[")) {
        consume();
        auto start = parseExpression(); // Assuming it ends up as an int
        if (tokenIs("]")) {
            consume();
            left = std::make_shared<IndexNode>(left, start);
        } else if (!tokenIs(":")) {
            handleError("Expected either ']' or ':'", getToken()->line, getToken()->column);
        } else {
            // Is :
            consume();
            auto end = parseExpression();
            if (!tokenIs("]")) {
                handleError("Expected ']'", getToken()->line, getToken()->column);
            }
            consume();
            left = std::make_shared<IndexNode>(left, start, end);
        }
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseCollection() {
    if (debug) std::cout << "parse collection " << getTokenStr() << std::endl;
    if (!tokenIs("[")) {
        return parseAtom();
    }
    consume();
    ASTList list;
    while (!tokenIs("]") && !tokenIs("eof") && !tokenIs(";")) {
        auto element = parseLogicalOr();
        list.push_back(element);
        if (tokenIs(",") && !nextTokenIs("]")) {
            consume();
        } else if (tokenIs(",")) {
            handleError("Expected more values", getToken()->line, getToken()->column);
        }
    }
    if (tokenIs("eof") || tokenIs(";")) {
        handleError("Expected ']'", getToken()->line, getToken()->column);
    }
    consume();
    return std::make_shared<ListNode>(list);
}

std::shared_ptr<ASTNode> Parser::parseAtom() {
    if (debug) std::cout << "parse atom " << getTokenStr() << std::endl;
    if (tokenIs("int") || tokenIs("float") || tokenIs("bool") || tokenIs("string")) {
        const Token* token = getToken();
        if (auto int_value = std::get_if<int>(&token->value)) {
            consume();
            return std::make_shared<AtomNode>(*int_value);
        }
        else if (auto float_value = std::get_if<double>(&token->value)) {
            consume();
            return std::make_shared<AtomNode>(*float_value);
        }
        else if (auto bool_value = std::get_if<bool>(&token->value)) {
            consume();
            return std::make_shared<AtomNode>(*bool_value);
        }
        else if (auto string_value = std::get_if<std::string>(&token->value)) {
            consume();
            return std::make_shared<AtomNode>(*string_value);
        }
    }
    else if (tokenIs("ident") && peek() && peek().value()->type == TokenType::_OpenParen) {
        return parseFuncCall();
    }
    else if (tokenIs("ident")) {
        return parseIdentifier(nullptr);
    }
    else {
        handleError(std::format("Expected atom but got {}", getTokenStr()), getToken()->line, getToken()->column);
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseFuncCall() {
    if (debug) std::cout << "parse func call " << getTokenStr() << std::endl;
    if (!tokenIs("ident")) {
        handleError("Expected function name but got " + getTokenStr(), getToken()->line, getToken()->column);
    }
    auto identifier = parseIdentifier(nullptr);
    if (!tokenIs("(")) {
        handleError("Missing '(' at function call", getToken()->line, getToken()->column);
    }
    consume();
    std::vector<std::shared_ptr<ASTNode>> arguments;
    while (!tokenIs(")") && !tokenIs("eof") && !tokenIs(";")) {
        arguments.push_back(parseLogicalOr());
        if (!tokenIs(")") && !tokenIs(",")) {
            handleError("Expected ','", getToken()->line, getToken()->column);
        } else if (tokenIs(",")) {
            consume();
            if (tokenIs(")")) {
                handleError("Expected another argument", getToken()->line, getToken()->column);
            }
        }
    }
    if (tokenIs("eof") || tokenIs(";")) {
        handleError("Expected ')'", getToken()->line, getToken()->column);
    }
    consume();
    return std::make_shared<FuncCallNode>(identifier, arguments);
}

std::shared_ptr<ASTNode> Parser::parseIdentifier(std::shared_ptr<std::string> varString = nullptr) {
    if (debug) std::cout << "parse identifier " << getTokenStr() << std::endl;
    if (tokenIs("ident")) {
        const Token* token = getToken();
        consume();
        if (auto ident_value = std::get_if<std::string>(&token->value)) {
            if (varString != nullptr) {
                *varString = *ident_value;
            }
            return std::make_shared<IdentifierNode>(*ident_value);
        }
        else {
            handleError("Attempted to create an identifier with an invalid type", token->line, token->column);
        }
    }
    else {
        handleError(std::format("Expected identifier but got {}", getTokenStr()), getToken()->line, getToken()->column);
    }
    return nullptr;
}


////////////////////////////////////////////////////////////////
// ENVIRONMENT
////////////////////////////////////////////////////////////////

std::string getValueStr(std::shared_ptr<Value> value) {
    if (std::holds_alternative<int>(*value)) {
        return "int";
    } else if (std::holds_alternative<double>(*value)) {
        return "double";
    } else if (std::holds_alternative<bool>(*value)) {
        return "bool";
    } else if (std::holds_alternative<std::string>(*value)) {
        return "string";
    } else if (std::holds_alternative<std::shared_ptr<ASTNode>>(*value)) {
        return "function";
    } else if (std::holds_alternative<std::shared_ptr<List>>(*value)) {
        return "list";
    } else {
        throw std::runtime_error("Attempted to get string of unrecognized type.");
    }
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

void Environment::addFunction(const std::string& name, std::shared_ptr<Value> func) {
    built_in_functions[name] = func;
}

std::shared_ptr<Value> Environment::getFunction(const std::string& name) const {
    auto func = built_in_functions.find(name);
    if (func != built_in_functions.end()) {
        return func->second;
    }

    throw std::runtime_error("Unrecognized built in function: " + name);
}

bool Environment::hasFunction(const std::string& name) const {
    auto func = built_in_functions.find(name);
    return func != built_in_functions.end();
}