#include "nodes.h"
#include <iostream>
#include "library.h"
#include <format>
#include <math.h>
#include <format>
#include "values.h"
#include "errorDefs.h"
#include <functional>
#include <algorithm>


ASTNode::ASTNode(int line, int column)
    : line{line}, column{column} {}


AtomNode::AtomNode(std::variant<int, double, bool, std::string, SpecialIndex> value, int line, int column)
    : ASTNode{line, column}, value(std::move(value)) {}

std::optional<std::shared_ptr<Value>> AtomNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Atom" << std::endl;
    if (isInt()) {
        return std::make_shared<Value>(getInt());
    }
    else if (isFloat()) {
        return std::make_shared<Value>(getFloat());
    }
    else if (isBool()) {
        return std::make_shared<Value>(getBool());
    }
    else if (isString()) {
        return std::make_shared<Value>(getString());
    }
    else if (isIndex()) {
        return std::make_shared<Value>(getIndex());
    }
    else {
        runtimeError("Unable to evaluate atom", line, column);
    }
}

bool AtomNode::isInt() {
    return std::holds_alternative<int>(value);
}
bool AtomNode::isFloat() {
    return std::holds_alternative<double>(value);
}
bool AtomNode::isBool() {
    return std::holds_alternative<bool>(value);
}
bool AtomNode::isString() {
    return std::holds_alternative<std::string>(value);
}
bool AtomNode::isIndex() {
    return std::holds_alternative<SpecialIndex>(value);
}

int AtomNode::getInt() {
    return std::get<int>(value);
}
double AtomNode::getFloat() {
    return std::get<double>(value);
}
bool AtomNode::getBool() {
    return std::get<bool>(value);
}
std::string AtomNode::getString() {
    return std::get<std::string>(value);
}
SpecialIndex AtomNode::getIndex() {
    return std::get<SpecialIndex>(value);
}


UnaryOpNode::UnaryOpNode(TokenType op, std::shared_ptr<ASTNode> right, int line, int column)
    : ASTNode{line, column}, op{op}, right{right} {}

std::optional<std::shared_ptr<Value>> UnaryOpNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Unary" << std::endl;
    std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);
    if (!right_value.has_value()) {
        runtimeError(std::format("Failed to evaluate unary operand with operator '{}'", getTokenTypeLabel(op)), line, column);
    }

    std::shared_ptr<Value> value = right_value.value();
    ValueType val_type = value->getType();
    if (val_type == ValueType::Integer) {
        int val = value->get<int>();
        if (op == TokenType::_Minus) {
            return std::make_shared<Value>(-val);
        }
        else if (op == TokenType::_Plus) {
            return value;
        }
        else if (op == TokenType::_Not || op == TokenType::_Exclamation) {
            return std::make_shared<Value>(val == 0);
        }
    }
    else if (val_type == ValueType::Float) {
        double val = value->get<double>();
        if (op == TokenType::_Minus) {
            return std::make_shared<Value>(-val);
        }
        else if (op == TokenType::_Plus) {
            return value;
        }
        else if (op == TokenType::_Not || op == TokenType::_Exclamation) {
            return std::make_shared<Value>(val == 0.0);
        }
    }
    else if (val_type == ValueType::Boolean) {
        bool val = value->get<bool>();
        if (op == TokenType::_Minus) {
            if (val) {
                return std::make_shared<Value>(-1);
            } else {
                return std::make_shared<Value>(0);
            }
        }
        else if (op == TokenType::_Plus) {
            if (val) {
                return std::make_shared<Value>(1);
            } else {
                return std::make_shared<Value>(0);
            }
        }
        else if (op == TokenType::_Not || op == TokenType::_Exclamation) {
            return std::make_shared<Value>(!val);
        }
    }
    else if (val_type == ValueType::String) {
        if (op == TokenType::_Not || op == TokenType::_Exclamation) {
            std::string val = value->get<std::string>();
            return std::make_shared<Value>(val == "");
        }
    }

    runtimeError(std::format("Unsupported operand types for operation. Operation was '{}' {}",
                                getTokenTypeLabel(op), getTypeStr(value->getType())), line, column);
    return std::nullopt;
}


BinaryOpNode::BinaryOpNode(std::shared_ptr<ASTNode> left, TokenType op, std::shared_ptr<ASTNode> right, int line, int column)
    : ASTNode{line, column}, left{left}, op{op}, right{right} {}

std::optional<std::shared_ptr<Value>> BinaryOpNode::performOperation(std::shared_ptr<Value> left_value,
                                                                    std::shared_ptr<Value>(right_value)) {
    std::string left_str = getValueStr(left_value);
    std::string right_str = getValueStr(right_value);

    // Helper function to determine the truthiness of a Value object
    auto check_truthy = [](const Value& value) -> bool {
        switch (value.getType()) {
            case ValueType::Boolean: {
                return value.get<bool>(); // Directly get and return the bool value
            }
            case ValueType::Integer: {
                return value.get<int>() != 0; // Integers are truthy if non-zero
            }
            case ValueType::Float: {
                return value.get<double>() != 0.0; // Floats are truthy if non-zero
            }
            case ValueType::String: {
                return !value.get<std::string>().empty(); // Strings are truthy if not empty
            }
            case ValueType::List: {
                return !value.get<std::shared_ptr<List>>()->empty();
            }
            case ValueType::None: {
                return false; // None is always false
            }
            default: {
                runtimeError("Check Truthy called on unknown value type.");
            }
        }
    };

    // Deep comparison function for lists
    std::function<bool(const std::shared_ptr<List>&, const std::shared_ptr<List>&)> deepCompareLists;

    deepCompareLists = [&](const std::shared_ptr<List>& lhs_list, const std::shared_ptr<List>& rhs_list) -> bool {
        if (lhs_list->size() != rhs_list->size()) {
            return false;
        }
        for (size_t i = 0; i < lhs_list->size(); ++i) {
            auto lhs_element = lhs_list->at(i);
            auto rhs_element = rhs_list->at(i);

            // Compare both values recursively or directly
            if (lhs_element->getType() != rhs_element->getType()) {
                return false; // Types must match
            }

            switch (lhs_element->getType()) {
                case ValueType::Boolean:
                    if (lhs_element->get<bool>() != rhs_element->get<bool>()) return false;
                    break;
                case ValueType::Integer:
                    if (lhs_element->get<int>() != rhs_element->get<int>()) return false;
                    break;
                case ValueType::Float:
                    if (lhs_element->get<double>() != rhs_element->get<double>()) return false;
                    break;
                case ValueType::String:
                    if (lhs_element->get<std::string>() != rhs_element->get<std::string>()) return false;
                    break;
                case ValueType::List:
                    if (!deepCompareLists(lhs_element->get<std::shared_ptr<List>>(), 
                                        rhs_element->get<std::shared_ptr<List>>())) {
                        return false; // Recursive call for nested lists
                    }
                    break;
                case ValueType::None:
                    break; // None values are considered equal
                default:
                    return false; // Unknown types are not equal
            }
        }
        return true; // All elements match
    };

    if (op == TokenType::_And) {
        if (check_truthy(*left_value)) {
            if (check_truthy(*right_value)) {
                return std::make_shared<Value>(true);
            }
        }
        return std::make_shared<Value>(false);
    }
    else if (op == TokenType::_Or) {
        if (check_truthy(*left_value)) {
            return std::make_shared<Value>(true);
        } else if (check_truthy(*right_value)) {
            return std::make_shared<Value>(true);
        } else {
            return std::make_shared<Value>(false);
        }
    }

    if (left_str == "list" && right_str == "list") {
        // BOTH ARE LISTS
        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) {
            std::shared_ptr<List> new_list = std::make_shared<List>();
            new_list->insert(left_value->get<std::shared_ptr<List>>()); // append the second list
            new_list->insert(right_value->get<std::shared_ptr<List>>()); // append the second list
            return std::make_shared<Value>(new_list);
        }
        else if (op == TokenType::_Compare) return std::make_shared<Value>(deepCompareLists(left_value->get<std::shared_ptr<List>>(),
                                                                                            right_value->get<std::shared_ptr<List>>()));
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(!deepCompareLists(left_value->get<std::shared_ptr<List>>(), 
                                                                                            right_value->get<std::shared_ptr<List>>()));
    }

    else if (left_str == "string" && right_str == "string") {
        // BOTH STRINGS
        std::string lhs = left_value->get<std::string>();
        std::string rhs = right_value->get<std::string>();
        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) {
            return std::make_shared<Value>(lhs + rhs);
        }
        else if (op == TokenType::_Compare) {
            return std::make_shared<Value>(lhs == rhs);
        }
        else if (op == TokenType::_NotEqual) {
            return std::make_shared<Value>(lhs != rhs);
        }
    }

    else if (left_str == "string" && right_str == "integer") {
        // STRING AND INT
        if (op == TokenType::_Multiply || op == TokenType::_MultiplyEquals) {
            std::string new_str = "";
            std::string copying = left_value->get<std::string>();
            for (int i = 0; i < right_value->get<int>(); i++) {
                new_str += copying;
            }
            return std::make_shared<Value>(new_str);
        }
        else if (op == TokenType::_Compare) {
            return std::make_shared<Value>(false);
        }
        else if (op == TokenType::_NotEqual) {
            return std::make_shared<Value>(true);
        }
    }

    else if ((left_str == "boolean" || left_str == "integer" || left_str == "float") &&
            (right_str == "boolean" || right_str == "integer" || right_str == "float")) {
        
        // Convert int/float/bool to a number to perform operation
        std::vector<std::variant<int, double>> result_vec = transformNums(left_value, right_value);
        std::string result_type;
        // Int vs Float influences final type
        double new_left, new_right;
        if (std::holds_alternative<int>(result_vec[0])) {
            result_type = "int";
            new_left = std::get<int>(result_vec[0]);
            new_right = std::get<int>(result_vec[1]);
        } else {
            result_type = "float";
            new_left = std::get<double>(result_vec[0]);
            new_right = std::get<double>(result_vec[1]);
        }

        double op_result;
        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) {op_result = new_left + new_right;}
        else if (op == TokenType::_Minus || op == TokenType::_MinusEquals) {op_result = new_left - new_right;}
        else if (op == TokenType::_Multiply || op == TokenType::_MultiplyEquals) {op_result = new_left * new_right;}
        else if (op == TokenType::_Divide || op == TokenType::_DivideEquals) {
            if (new_right == 0.0) {
                handleError("Attempted division by zero", line, column, "Zero Division Error");
            }
            return std::make_shared<Value>(new_left / new_right);
        }
        else if (op == TokenType::_DoubleDivide) {
            if (new_right == 0.0) {
                handleError("Attempted division by zero", line, column, "Zero Division Error");
            }
            int result = static_cast<int>(new_left / new_right);
            return std::make_shared<Value>(result);
        }
        else if (op == TokenType::_Caret) {op_result = pow(new_left, new_right);}
        else if (op == TokenType::_DoubleMultiply) {op_result = pow(new_left, new_right);}
        else if (op == TokenType::_LessThan) {return std::make_shared<Value>(new_left < new_right);}
        else if (op == TokenType::_LessEquals) {return std::make_shared<Value>(new_left <= new_right);}
        else if (op == TokenType::_GreaterThan) {return std::make_shared<Value>(new_left > new_right);}
        else if (op == TokenType::_GreaterEquals) {return std::make_shared<Value>(new_left >= new_right);}
        else if (op == TokenType::_Compare) {
            return std::make_shared<Value>(new_left == new_right);
        }
        else if (op == TokenType::_NotEqual) {
            return std::make_shared<Value>(new_left != new_right);
        }
        else {
            return std::nullopt;
        }

        if (result_type == "int") {
            return std::make_shared<Value>(static_cast<int>(op_result));
        } else {
            return std::make_shared<Value>(op_result);
        }
    }

    else {
        // Unknown Type Combination
        if (op == TokenType::_Compare || op == TokenType::_NotEqual) {
            // Lambda to compare values of the same type
            auto equalityCheck = [](const Value& lhs, const Value& rhs) -> bool {
                // Compare based on type
                if (lhs.getType() != rhs.getType()) {
                    return false; // Types don't match, return false
                }

                // Use switch to handle specific types
                switch (lhs.getType()) {
                    case ValueType::Boolean:
                        return lhs.get<bool>() == rhs.get<bool>();
                    case ValueType::Integer:
                        return lhs.get<int>() == rhs.get<int>();
                    case ValueType::Float:
                        return lhs.get<double>() == rhs.get<double>();
                    case ValueType::String:
                        return lhs.get<std::string>() == rhs.get<std::string>();
                    case ValueType::None:
                        return true; // Two "None" values are equal
                    default:
                        return false; // Fallback for unknown types
                }
            };

            try {
                // Perform equality comparison
                bool result = equalityCheck(*left_value, *right_value);

                if (op == TokenType::_Compare) {
                    return std::make_shared<Value>(result); // Return true/false
                } else if (op == TokenType::_NotEqual) {
                    return std::make_shared<Value>(!result); // Negate result for "NotEqual"
                }
            } catch (const std::runtime_error& e) {
                // Handle unexpected errors (e.g., type mismatch or invalid access)
                handleError(e.what(), 0, 0, "Runtime Error");
                return std::nullopt;
            }
        }
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> BinaryOpNode::evaluate(Environment& env) {
    if (op == TokenType::_Equals) {
        std::optional<std::shared_ptr<Value>> right_opt = right->evaluate(env);
        if (!right_opt) {
            runtimeError("Unable to evaluate right side of equals", line, column);
        }
        
        auto ident_node = std::dynamic_pointer_cast<IdentifierNode>(left);
        if (ident_node) {
            env.set(ident_node->name, right_opt.value());
            return std::nullopt;
        } else {
            runtimeError("Invalid value assignment", line, column);
        }
    } else {
        std::optional<std::shared_ptr<Value>> left_opt = left->evaluate(env);
        std::optional<std::shared_ptr<Value>> right_opt = right->evaluate(env);

        if (!left_opt.has_value() || !right_opt.has_value()) {
            runtimeError(std::format("Unable to evaluate binary operand for operator '{}'", getTokenTypeLabel(op)), line, column);
        }
        auto result = performOperation(left_opt.value(), right_opt.value());
        if (result) {
            if (op == TokenType::_PlusEquals || op == TokenType::_MinusEquals || op == TokenType::_MultiplyEquals || op == TokenType::_DivideEquals) {
                // Handle setting +=, -= etc.
                if (auto identifierNode = dynamic_cast<IdentifierNode*>(left.get())) {
                    env.set(identifierNode->name, result.value());
                }
                else {
                    runtimeError("The operator '=' can only be used with variables", line, column);
                }
            }
            else {
                return result;
            }
        } else {
            runtimeError(std::format("Unsupported operand types for operation. operation was {} '{}' {}",
                                    getValueStr(left_opt.value()), getTokenTypeLabel(op), getValueStr(right_opt.value())), line, column);
        }
    }
    return std::nullopt;
}


ParenthesisOpNode::ParenthesisOpNode(std::shared_ptr<ASTNode> expr, int line, int column)
        : ASTNode{line, column}, expr{expr} {}

std::optional<std::shared_ptr<Value>> ParenthesisOpNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Parenthesis" << std::endl;
    std::optional<std::shared_ptr<Value>> expr_value = expr->evaluate(env);
    return expr_value;
}


IdentifierNode::IdentifierNode(std::string name, int line, int column)
    : ASTNode{line, column}, name{name} {}

std::optional<std::shared_ptr<Value>> IdentifierNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Identifier" << std::endl;
    if (env.contains(name)) {
        return env.get(name);
    } else if (env.hasFunction(name)) {
        return env.getFunction(name);
    } else {
        runtimeError(std::format("Name '{}' is not defined", name), line, column);
    }
}


ScopedNode::ScopedNode(TokenType keyword, std::shared_ptr<ScopedNode> if_link, std::shared_ptr<ASTNode> comparison,
            std::vector<std::shared_ptr<ASTNode>> statements_block, int line, int column)
    : ASTNode{line, column}, keyword{keyword}, if_link{if_link}, comparison{comparison},
        last_comparison_result{false}, statements_block{statements_block} {}

bool ScopedNode::getComparisonValue(Environment& env) const {
    auto result = comparison->evaluate(env);
    if (result.has_value()) {
        auto checkTruthy = [](const Value& value) -> bool {
            switch (value.getType()) {
                case ValueType::Boolean:
                    return value.get<bool>(); // Return the boolean value directly
                case ValueType::Integer:
                    return value.get<int>() != 0; // Non-zero integers are truthy
                case ValueType::Float:
                    return value.get<double>() != 0.0; // Non-zero floats are truthy
                case ValueType::String:
                    return !value.get<std::string>().empty(); // Non-empty strings are truthy
                case ValueType::List:
                    return !value.get<std::shared_ptr<List>>()->empty(); // Non-empty lists are truthy
                case ValueType::None:
                default:
                    return false; // None or unknown types are always false
            }
        };

        return checkTruthy(*result.value());
    }
    else {
        runtimeError("Missing a boolean comparison for keyword to evaluate", line, column);
        return false;
    }
}

std::optional<std::shared_ptr<Value>> ScopedNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Scoped" << std::endl;
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

    std::string str = getTokenTypeLabel(keyword);
    if (str == "if" || str == "elif" || str == "else" || str == "while" || str == "for") {
        env.addScope();

        if (str == "while") {
            env.addLoop();
            while (getComparisonValue(env)) {
                try {
                    for (int i = 0; i < statements_block.size(); i++) {
                        auto result = statements_block[i]->evaluate(env);
                        if (result.has_value()) {
                            printValue(result.value());
                            std::cout << std::endl;
                        }
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
                if (result.has_value()) {
                    printValue(result.value());
                    std::cout << std::endl;
                }
            }
        }

        env.removeScope();
    }

    return std::nullopt;
}

ForNode::ForNode(TokenType keyword, std::shared_ptr<ASTNode> initialization, std::shared_ptr<std::string> init_string,
        std::shared_ptr<ASTNode> condition_value, std::shared_ptr<ASTNode> increment,
        std::vector<std::shared_ptr<ASTNode>> block, int line, int column)
    : ASTNode{line, column}, keyword{keyword}, initialization{initialization}, init_string{init_string},
        condition_value{condition_value}, increment{increment}, block{block} {}

std::optional<std::shared_ptr<Value>> ForNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate For" << std::endl;
    env.addScope();
    env.addLoop();
    auto init_node = dynamic_cast<BinaryOpNode*>(initialization.get());
    initialization->evaluate(env);

    int variable;
    if (env.get(*init_string)->getType() == ValueType::Integer) {
        variable = env.get(*init_string)->get<int>();
    }
    else {
        runtimeError("For loop requires int variable", line, column);
    }

    while (true) {
        auto cond_value = condition_value->evaluate(env);
        if (!cond_value) {
            runtimeError("Unable to evaluate for loop condition", line, column);
        }

        if (cond_value.value()->getType() == ValueType::Boolean) {
            auto bool_value = cond_value.value()->get<bool>();
            if (!bool_value) break;
        } else {
            runtimeError("For loop requires boolean condition", line, column);
        }

        int original_variable = variable;

        try {
            for (auto statement : block) {
                auto result = statement->evaluate(env);
                if (result.has_value()) {
                    printValue(result.value());
                    std::cout << std::endl;
                }
            }
        }
        catch (const BreakException) {
            break;
        }
        catch (const ContinueException) {
            // Does nothing but catches exception and proceeds to increment the variable
            auto x = 1;
        }

        variable = original_variable;
        // set variable in env
        env.set(*init_string, std::make_shared<Value>(variable));
        // increment them
        increment->evaluate(env);
        if (env.get(*init_string)->getType() == ValueType::Integer) {
            variable = env.get(*init_string)->get<int>();
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
    } else if (keyword == TokenType::_Return) {
        if (right != nullptr) {
            throw ReturnException(right->evaluate(env));
        } else {
            throw ReturnException(std::nullopt);
        }
    }

    if (right) {
        return right->evaluate(env);
    } else {
        return std::nullopt;
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
                evaluated_list.append(result.value());
            } else {
                runtimeError("List element was unable to be evaluated", line, column);
            }
        } else {
            runtimeError("List contained a nullptr pointing to an ASTNode", line, column); // Null ASTNode pointer
        }
    }

    return std::make_shared<Value>(std::make_shared<List>(evaluated_list));
}


std::optional<std::shared_ptr<Value>> IndexNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Index" << std::endl;
    if (container == nullptr) {
        runtimeError("Null object is not subscriptable", line, column);
    }

    auto eval = container->evaluate(env);
    if (eval) {
        if (eval.value()->getType() == ValueType::String) {
            auto string_val = std::make_shared<std::string>(eval.value()->get<std::string>());
            return getIndex(env, string_val);
        }
        else if (eval.value()->getType() == ValueType::List) {
            auto list_val = eval.value()->get<std::shared_ptr<List>>();
            return getIndex(env, list_val);
        }
        else {
            runtimeError("Index node container was an unexpected type", line, column);
        }
    }
    return std::nullopt;
}

std::variant<char, std::shared_ptr<Value>> getAtIndex(std::variant<std::shared_ptr<std::string>,
                                                                    std::shared_ptr<List>> listr,
                                                        int index, int line, int column) {
    // The function is given the container to extract the index from

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
                                                                std::shared_ptr<List>> listr) {
    if (end_index) {
        auto resolveIndex = [&](const Value& index, int container_size) -> int {
            if (index.getType() == ValueType::Integer) {
                int idx = index.get<int>();
                if (idx < 0) {
                    idx += container_size; // Handle negative index
                }
                return std::clamp(idx, 0, container_size); // Clamp within bounds
            } else if (index.getType() == ValueType::Index) {
                if (index.get<SpecialIndex>() == SpecialIndex::End) {
                    return container_size; // Resolve "end" to container size
                }
            }
            throw std::runtime_error("Invalid index type");
        };

        auto start_result = start_index->evaluate(env);
        auto end_result = end_index->evaluate(env);

        if (start_result && end_result) {
            // Check if the container is a string or list
            if (std::holds_alternative<std::shared_ptr<std::string>>(listr)) {
                auto str = std::get<std::shared_ptr<std::string>>(listr);
                int size = str->size();

                // Resolve indices
                int start_val = resolveIndex(*start_result.value(), size);
                int end_val = resolveIndex(*end_result.value(), size);

                // Return empty string if range is invalid
                if (start_val >= end_val) {
                    return std::make_shared<Value>("");
                }

                // Extract substring
                std::string sub_str = str->substr(start_val, end_val - start_val);
                return std::make_shared<Value>(sub_str);
            } else if (std::holds_alternative<std::shared_ptr<List>>(listr)) {
                auto list_ptr = std::get<std::shared_ptr<List>>(listr);
                int size = list_ptr->size();

                // Resolve indices
                int start_val = resolveIndex(*start_result.value(), size);
                int end_val = resolveIndex(*end_result.value(), size);

                // Return empty list if range is invalid
                if (start_val >= end_val) {
                    return std::make_shared<Value>(std::make_shared<List>());
                }

                // Create a new list with the slice
                auto new_list = std::make_shared<List>();
                for (int i = start_val; i < end_val; ++i) {
                    auto list_val = std::get<std::shared_ptr<Value>>(
                        getAtIndex(list_ptr, i, line, column));
                    new_list->append(list_val);
                }
                return std::make_shared<Value>(new_list);
            } else {
                runtimeError("Invalid type for getting index", line, column);
            }
        } else {
            runtimeError("Failed to evaluate start_index or end_index", line, column);
        }
    } else {
        auto result = start_index->evaluate(env);
        if (result) {
            if (result.value()->getType() == ValueType::Integer) {
                int int_val = result.value()->get<int>();
                if (std::holds_alternative<std::shared_ptr<std::string>>(listr)) {
                    auto get_char = getAtIndex(listr, int_val, line, column);
                    if (std::holds_alternative<char>(get_char)) {
                        auto c = std::get<char>(get_char);
                        std::string str = std::string(1, c);
                        return std::make_shared<Value>(str);
                    }
                } else {
                    auto value = std::get<std::shared_ptr<Value>>(getAtIndex(listr, int_val, line, column));
                    return value;
                }
            } else {
                runtimeError("The index was not given an int", line, column);
            }
        } else {
            runtimeError("Failed to evaluate start_index", line, column);
            return std::nullopt;
        }
    }
    
    return std::nullopt;
}

void setAtIndex(std::variant<std::shared_ptr<List>> env_dist, std::shared_ptr<Value> index_key, std::shared_ptr<Value> value) {
    if (std::holds_alternative<std::shared_ptr<List>>(env_dist)) {
        auto env_list = std::get<std::shared_ptr<List>>(env_dist);
        int list_index = index_key->get<int>();
        if (list_index >= 0 && list_index <= env_list->size()) {
            if (value->getType() == ValueType::String) {
                // Assigning a string
                auto str_val = value->get<std::string>();
                std::string s = "";
                int i = 0;
                for (auto c : str_val) {
                    s += c;
                    auto char_str = std::make_shared<Value>(s);
                    if (list_index + i == env_list->size()) {
                        env_list->append(char_str);
                    } else {
                        env_list->insert(list_index + i, char_str);
                    }
                    s = "";
                    i++;
                }
            } else if (value->getType() == ValueType::List) {
                auto list_val = value->get<std::shared_ptr<List>>();
                // Assigning a list
                for (int i = list_val->size() - 1; i >= 0; i--) {
                    if (list_index == env_list->size()) {
                        env_list->append(list_val->at(i));
                    } else {
                        env_list->insert(list_index, list_val->at(i));
                    }
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
        runtimeError("Index assignment unable to evaluate the container", line, column);
        return;
    }
    if (env_val->getType() == ValueType::List) {
        std::shared_ptr<List> env_list = env_val->get<std::shared_ptr<List>>();

        if (end_index == nullptr) {
            // A single index assignment
            auto index_eval = start_index->evaluate(env);
            if (index_eval) {
                if (index_eval.value()->getType() == ValueType::Integer) {
                    int index = index_eval.value()->get<int>();
                    if (index >= 0 && index < env_list->size()) {
                        env_list->at(index) = value;
                    } else if (index < 0 && index >= env_list->size() * -1) {
                        env_list->at(env_list->size() - index) = value;
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
                if (start_eval.value()->getType() == ValueType::Integer) {
                    int start_val = start_eval.value()->get<int>();
                    if (end_eval.value()->getType() == ValueType::Integer) {
                        int end_val = end_eval.value()->get<int>();
                        int slice_size = std::abs(end_val - start_val);
                        // Cut out the slice section of the original list
                        for (int i = 0; i < slice_size; i++) {
                            if (start_val == env_list->size()) {
                                break;
                                // runtimeError("Assignment index went out of range.");
                            }
                            env_list->pop(start_val);
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
    else {
        runtimeError(getValueStr(env_val) + " object does not support item assignment", line, column);
        return;
    }
}

std::optional<std::shared_ptr<Value>> FuncNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Function" << std::endl;
    return std::make_shared<Value>(std::static_pointer_cast<ASTNode>(std::make_shared<FuncNode>(*this)));
}

void FuncNode::setArgs(std::vector<std::shared_ptr<Value>> values, Scope& local_scope) {
    if (values.size() != args.size()) {
        runtimeError(std::format("Incorrect number of args were passed in. {} instead of {}", values.size(), args.size()), line, column);
    }

    for (size_t i = 0; i < values.size(); i++) {
        if (auto ident_node = dynamic_cast<IdentifierNode*>(args.at(i).get())) {
            std::string arg_string = ident_node->name;
            local_scope.set(arg_string, values.at(i));
        } else {
            runtimeError("Unable to convert identifier for function argument", line, column);
        }
    }

    return;
}

std::optional<std::shared_ptr<Value>> FuncNode::callFunc(std::vector<std::shared_ptr<Value>> values, Environment& parent_env) {
    Environment local_env = parent_env;
    Scope local_scope;
    setArgs(values, local_scope);
    local_env.addScope(local_scope);
    for (auto statement : block) {
        try {
            if (auto func_statement = dynamic_cast<FuncCallNode*>(statement.get())) {
                func_statement->parent_env = std::make_shared<Environment>(local_env);
                auto result = func_statement->evaluate(local_env);
                if (result) {
                    printValue(result.value());
                    std::cout << std::endl;
                }
            } else {
                auto result = statement->evaluate(local_env);
                if (result) {
                    printValue(result.value());
                    std::cout << std::endl;
                }
            }
        }
        catch (const ReturnException& e) {
            return e.value;
        }
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> FuncCallNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Function Call" << std::endl;
    auto mapped_value = identifier->evaluate(env).value();
    if (mapped_value->getType() == ValueType::Function) {
        auto func_value = mapped_value->get<std::shared_ptr<ASTNode>>();
        if (auto func = dynamic_cast<FuncNode*>(func_value.get())) {
            if (parent_env) {
                return func->callFunc(evaluateArgs(env), *parent_env.get());
            }
            else {
                return func->callFunc(evaluateArgs(env), env);
            }
        } else {
            runtimeError("Unable to call function " + dynamic_cast<IdentifierNode*>(identifier.get())->name, line, column);
        }
    } else if (mapped_value->getType() == ValueType::BuiltInFunction) {
        auto func_value = mapped_value->get<std::shared_ptr<BuiltInFunction>>();
        std::vector<std::shared_ptr<Value>> values = evaluateArgs(env);
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

std::vector<std::shared_ptr<Value>> FuncCallNode::evaluateArgs(Environment& env) {
    std::vector<std::shared_ptr<Value>> evaluated_values;
    for (auto value_node : values) {
        auto result = value_node->evaluate(env);
        if (!result.has_value()) {
            runtimeError("Unable to evaluate function argument", line, column);
        }
        evaluated_values.push_back(result.value());
    }

    return evaluated_values;
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
        {ValueType::None, "Type:Null"}
    };
    if (types.count(type) != 0) {
        return types[type];
    } else {
        runtimeError("No such type string found");
    }
}