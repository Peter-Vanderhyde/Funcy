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
#include "parser.h"
#include "context.h"
#include "lexer.h"

std::unordered_map<TokenType, ValueType> type_map{
    {TokenType::_IntType, ValueType::Integer},
    {TokenType::_FloatType, ValueType::Float},
    {TokenType::_BoolType, ValueType::Boolean},
    {TokenType::_StrType, ValueType::String},
    {TokenType::_ListType, ValueType::List},
    {TokenType::_FuncType, ValueType::Function},
    {TokenType::_BuiltInType, ValueType::BuiltInFunction},
    {TokenType::_Class, ValueType::Class},
    {TokenType::_Instance, ValueType::Instance},
    {TokenType::_NullType, ValueType::None}
};

ASTNode::ASTNode(int line, int column)
    : line{line}, column{column} {}


AtomNode::AtomNode(std::variant<int, double, bool, std::string, SpecialIndex> value, int line, int column)
    : ASTNode{line, column}, value(std::move(value)) {}

std::optional<std::shared_ptr<Value>> AtomNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Atom" << std::endl;
    else if (isInt()) {
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
    return std::nullopt;
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
    else if (val_type == ValueType::None) {
        if (op == TokenType::_Not || op == TokenType::_Exclamation) {
            return std::make_shared<Value>(true);
        }
    }

    runtimeError(std::format("Unsupported operand types for operation. Operation was '{}' {}",
                                getTokenTypeLabel(op), getTypeStr(value->getType())), line, column);
    return std::nullopt;
}


BinaryOpNode::BinaryOpNode(std::shared_ptr<ASTNode> left, TokenType op, std::shared_ptr<ASTNode> right, int line, int column)
    : ASTNode{line, column}, left{left}, op{op}, right{right} {}

// Helper function to determine the truthiness of a Value object
bool check_truthy(const Value& value) {
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
        case ValueType::Dictionary: {
            return !value.get<std::shared_ptr<Dictionary>>()->empty();
        }
        case ValueType::Function: {
            return true;
        }
        case ValueType::BuiltInFunction: {
            return true;
        }
        case ValueType::Class: {
            return true;
        }
        case ValueType::Instance: {
            return true;
        }
        case ValueType::Type: {
            return value.get<ValueType>() != ValueType::None;
        }
        case ValueType::None: {
            return false; // None is always false
        }
        default: {
            runtimeError("Check Truthy called on unknown value type");
        }
    }
};

std::optional<std::shared_ptr<Value>> BinaryOpNode::performOperation(std::shared_ptr<Value> left_value,
                                                                    std::shared_ptr<Value>(right_value),
                                                                    TokenType* custom_op) {
    std::string left_str = getValueStr(left_value);
    std::string right_str = getValueStr(right_value);

    TokenType operation;
    if (!custom_op) {
        operation = op;
    } else {
        operation = *custom_op;
    }

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

    // Deep comparison function for lists
    std::function<bool(const std::shared_ptr<Dictionary>&, const std::shared_ptr<Dictionary>&)> deepCompareDictionaries;

    deepCompareDictionaries = [&](const std::shared_ptr<Dictionary>& lhs_dict,
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
            auto result = performOperation(lhs_value, rhs_value, &operation);

            if (!result) {
                return false;
            }
        }

        return true;
    };

    if (operation == TokenType::_And) {
        if (check_truthy(*left_value)) {
            if (check_truthy(*right_value)) {
                return std::make_shared<Value>(true);
            }
        }
        return std::make_shared<Value>(false);
    }
    else if (operation == TokenType::_Or) {
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
        if (operation == TokenType::_Plus || operation == TokenType::_PlusEquals) {
            std::shared_ptr<List> new_list = std::make_shared<List>();
            new_list->insert(left_value->get<std::shared_ptr<List>>()); // push_back the second list
            new_list->insert(right_value->get<std::shared_ptr<List>>()); // push_back the second list
            return std::make_shared<Value>(new_list);
        }
        else if (operation == TokenType::_In) {
            auto list = right_value->get<std::shared_ptr<List>>();
            for (int i = 0; i < list->size(); i++) {
                if (list->at(i)->getType() == ValueType::List) {
                    if (deepCompareLists(left_value->get<std::shared_ptr<List>>(), list->at(i)->get<std::shared_ptr<List>>())) {
                        return std::make_shared<Value>(true);
                    }
                }
            }
            return std::make_shared<Value>(false);
        }
        else if (operation == TokenType::_Compare) return std::make_shared<Value>(deepCompareLists(left_value->get<std::shared_ptr<List>>(),
                                                                                            right_value->get<std::shared_ptr<List>>()));
        else if (operation == TokenType::_NotEqual) return std::make_shared<Value>(!deepCompareLists(left_value->get<std::shared_ptr<List>>(), 
                                                                                            right_value->get<std::shared_ptr<List>>()));
    }

    else if (left_str == "dictionary" && right_str == "dictionary") {
        // BOTH ARE DICTIONARIES
        if (operation == TokenType::_Compare) return std::make_shared<Value>(deepCompareDictionaries(left_value->get<std::shared_ptr<Dictionary>>(),
                                                                                            right_value->get<std::shared_ptr<Dictionary>>()));
        else if (operation == TokenType::_NotEqual) return std::make_shared<Value>(!deepCompareDictionaries(left_value->get<std::shared_ptr<Dictionary>>(),
                                                                                            right_value->get<std::shared_ptr<Dictionary>>()));
    }

    else if (left_str == "string" && right_str == "string") {
        // BOTH STRINGS
        std::string lhs = left_value->get<std::string>();
        std::string rhs = right_value->get<std::string>();
        if (operation == TokenType::_Plus || operation == TokenType::_PlusEquals) {
            return std::make_shared<Value>(lhs + rhs);
        }
        else if (operation == TokenType::_Compare) {
            return std::make_shared<Value>(lhs == rhs);
        }
        else if (operation == TokenType::_NotEqual) {
            return std::make_shared<Value>(lhs != rhs);
        }
    }

    else if (left_str == "string" && right_str == "integer") {
        // STRING AND INT
        if (operation == TokenType::_Multiply || operation == TokenType::_MultiplyEquals) {
            std::string new_str = "";
            std::string copying = left_value->get<std::string>();
            for (int i = 0; i < right_value->get<int>(); i++) {
                new_str += copying;
            }
            return std::make_shared<Value>(new_str);
        }
        else if (operation == TokenType::_Compare) {
            return std::make_shared<Value>(false);
        }
        else if (operation == TokenType::_NotEqual) {
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
        if (operation == TokenType::_Plus || operation == TokenType::_PlusEquals) {op_result = new_left + new_right;}
        else if (operation == TokenType::_Minus || operation == TokenType::_MinusEquals) {op_result = new_left - new_right;}
        else if (operation == TokenType::_Multiply || operation == TokenType::_MultiplyEquals) {op_result = new_left * new_right;}
        else if (operation == TokenType::_Divide || operation == TokenType::_DivideEquals) {
            if (new_right == 0.0) {
                handleError("Attempted division by zero", line, column, "Zero Division Error");
            }
            return std::make_shared<Value>(new_left / new_right);
        }
        else if (operation == TokenType::_DoubleDivide) {
            if (new_right == 0.0) {
                handleError("Attempted division by zero", line, column, "Zero Division Error");
            }
            int result = static_cast<int>(new_left / new_right);
            return std::make_shared<Value>(result);
        }
        else if (operation == TokenType::_Caret) {op_result = pow(new_left, new_right);}
        else if (operation == TokenType::_DoubleMultiply) {op_result = pow(new_left, new_right);}
        else if (operation == TokenType::_Mod) {op_result = fmod(new_left, new_right);}
        else if (operation == TokenType::_LessThan) {return std::make_shared<Value>(new_left < new_right);}
        else if (operation == TokenType::_LessEquals) {return std::make_shared<Value>(new_left <= new_right);}
        else if (operation == TokenType::_GreaterThan) {return std::make_shared<Value>(new_left > new_right);}
        else if (operation == TokenType::_GreaterEquals) {return std::make_shared<Value>(new_left >= new_right);}
        else if (operation == TokenType::_Compare) {
            return std::make_shared<Value>(new_left == new_right);
        }
        else if (operation == TokenType::_NotEqual) {
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
        if (operation == TokenType::_Compare || operation == TokenType::_NotEqual) {
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
                    case ValueType::Function:
                        return lhs.get<std::shared_ptr<ASTNode>>() == rhs.get<std::shared_ptr<ASTNode>>();
                    case ValueType::BuiltInFunction:
                        return lhs.get<std::shared_ptr<BuiltInFunction>>() == rhs.get<std::shared_ptr<BuiltInFunction>>();
                    case ValueType::Type:
                        return lhs.get<ValueType>() == rhs.get<ValueType>();
                    case ValueType::None:
                        return true; // Two "None" values are equal
                    default:
                        return false; // Fallback for unknown types
                }
            };

            try {
                // Perform equality comparison
                bool result = equalityCheck(*left_value, *right_value);

                if (operation == TokenType::_Compare) {
                    return std::make_shared<Value>(result); // Return true/false
                } else if (operation == TokenType::_NotEqual) {
                    return std::make_shared<Value>(!result); // Negate result for "NotEqual"
                }
            } catch (const std::runtime_error& e) {
                // Handle unexpected errors (e.g., type mismatch or invalid access)
                runtimeError(e.what());
                return std::nullopt;
            }
        }
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> BinaryOpNode::evaluate(Environment& env) {
    if (op == TokenType::_Equals) {
        // An equals is a special case
        std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);
        if (!right_value.has_value()) {
            runtimeError("Failed to set variable. Operand could not be computed", line, column);
        }

        // Give it the actual left string, not the value of the variable
        if (auto node = std::dynamic_pointer_cast<BinaryOpNode>(left)) {
            // It's a class.member = value
            if (auto attr_ident = std::dynamic_pointer_cast<IdentifierNode>(node->right)) {
                if (auto instance_ident = std::dynamic_pointer_cast<IdentifierNode>(node->left)) {
                    auto instance_value = env.get(instance_ident->name);
                    if (instance_value->getType() != ValueType::Instance) {
                        runtimeError(getTypeStr(instance_value->getType()) + " object has no attribute " + attr_ident->name, line, column);
                    }
                    auto instance = instance_value->get<std::shared_ptr<Instance>>();
                    instance->getEnvironment().setMember(attr_ident->name, right_value.value());
                } else {
                    auto left_value = node->left->evaluate(env);
                    runtimeError(getValueStr(left_value.value()) + " object has no attribute " + attr_ident->name, line, column);
                }
            } else {
                runtimeError("Invalid syntax", line, column);
            }
        } else if (auto identifier_node = std::dynamic_pointer_cast<IdentifierNode>(left)) {
            env.set(identifier_node->name, right_value.value());
        } else if (auto index_node = dynamic_cast<IndexNode*>(left.get())) {
            index_node->assignIndex(env, right_value.value());
        } else if (auto list_node = std::dynamic_pointer_cast<ListNode>(left)) {
            if (right_value.value()->getType() != ValueType::List) {
                runtimeError("Expected list. Cannot unpack " + getValueStr(right_value.value()), line, column);
            }

            auto right_list = right_value.value()->get<std::shared_ptr<List>>();
            if (right_list->size() > list_node->list.size()) {
                runtimeError("Too many values to unpack", line, column);
            } else if (right_list->size() < list_node->list.size()) {
                runtimeError("Too few values to unpack", line, column);
            }

            for (int i = 0; i < right_list->size(); i++) {
                if (auto identifier_node = std::dynamic_pointer_cast<IdentifierNode>(list_node->list.at(i))) {
                    env.set(identifier_node->name, right_list->at(i));
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
        ValueType member_type = left_value.value()->getType();
        if (auto func_node = std::dynamic_pointer_cast<MethodCallNode>(right)) {
            // It's a member function
            // Save the result of the member to pass into the function
            func_node->member_value = left_value.value();
            // Pass member type so evaluate knows to search for functions of that type
            return func_node->evaluate(env, member_type);
        }
        else if (auto ident_node = std::dynamic_pointer_cast<IdentifierNode>(right)) {
            Environment environment{env};
            if (member_type == ValueType::Instance) {
                environment = left_value.value()->get<std::shared_ptr<Instance>>()->getEnvironment();
            }
            return ident_node->evaluate(environment, member_type);
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

        if (right_value.value()->getType() == ValueType::List) {
            auto list = right_value.value()->get<std::shared_ptr<List>>();
            for (int i = 0; i < list->size(); i++) {
                const auto& item = list->at(i);
                TokenType compare = TokenType::_Compare;
                auto result = performOperation(left_value.value(), item, &compare);

                if (result && result.value()->getType() == ValueType::Boolean && result.value()->get<bool>()) {
                    return std::make_shared<Value>(true);
                }
            }
            return std::make_shared<Value>(false);
        }
        else if (right_value.value()->getType() == ValueType::Dictionary) {
            auto dict = right_value.value()->get<std::shared_ptr<Dictionary>>();
            for (const auto& pair : *dict) {
                TokenType compare = TokenType::_Compare;
                auto result = performOperation(left_value.value(), pair.first, &compare);

                if (result && result.value()->getType() == ValueType::Boolean && result.value()->get<bool>()) {
                    return std::make_shared<Value>(true);
                }
            }
            return std::make_shared<Value>(false);
        }
        else if (right_value.value()->getType() == ValueType::String) {
            auto string = right_value.value()->get<std::string>();
            auto left_val = *left_value.value();
            if (left_val.getType() != ValueType::String) {
                return std::make_shared<Value>(false);
            }
            std::string left = left_val.get<std::string>();
            auto index = string.find(left);
            if (index != std::string::npos) {
                return std::make_shared<Value>(true);
            }
            return std::make_shared<Value>(false);
        }
        else {
            runtimeError("Expected list or dictionary for 'in' evaluation", line, column);
        }
    } else if (op == TokenType::_And || op == TokenType::_Or) {
        auto left_value = left->evaluate(env);
        if (!left_value.has_value()) {
            runtimeError("Unable to evaluate left operand for 'and' or 'or'", line, column);
        }

        bool left_truthy = check_truthy(*left_value.value());

        // Short-circuit logic for 'and' and 'or'
        if (op == TokenType::_And) {
            if (!left_truthy) {
                // Short-circuit: if left is false, return false immediately
                return std::make_shared<Value>(false);
            }
        } else if (op == TokenType::_Or) {
            if (left_truthy) {
                // Short-circuit: if left is true, return true immediately
                return std::make_shared<Value>(true);
            }
        }

        // Evaluate the right-hand side only if necessary
        auto right_value = right->evaluate(env);
        if (!right_value.has_value()) {
            runtimeError("Unable to evaluate right operand for 'and' or 'or'", line, column);
        }

        bool right_truthy = check_truthy(*right_value.value());
        return std::make_shared<Value>(right_truthy);
    }else {
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
    } else if (env.isGlobal(name)) {
        return std::nullopt;
    } else {
        runtimeError(std::format("Name '{}' is not defined", name), line, column);
    }
}

std::optional<std::shared_ptr<Value>> IdentifierNode::evaluate(Environment& env, ValueType member_type) {
    if (debug) std::cout << "Evaluate Identifier" << std::endl;
    if (env.hasMember(member_type, name)) {
        return env.getMember(member_type, name);
    } else {
        runtimeError(name + " is not defined", line, column);
    }
    return std::nullopt;
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
                case ValueType::Dictionary:
                    return !value.get<std::shared_ptr<Dictionary>>()->empty();
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
    if (init_node->op != TokenType::_In) {
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
                }
            }
            catch (const BreakException) {
                break;
            }
            catch (const ContinueException) {
                // Does nothing but catches exception and proceeds to increment the variable
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
    } else {
        // in was used with for loop instead of integer
        auto container_result = init_node->right->evaluate(env);
        if (container_result.value()->getType() == ValueType::List) {
            auto list = container_result.value()->get<std::shared_ptr<List>>();
            auto ident_node = dynamic_cast<IdentifierNode*>(init_node->left.get());
            if (ident_node) {
                std::string var_string = ident_node->name;

                for (int i = 0; i < list->size(); i++) {
                    auto item = list->at(i);
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
            } else {
                auto list_node = std::dynamic_pointer_cast<ListNode>(init_node->left);
                if (!list_node) {
                    runtimeError("For loop expected identifier or list", line, column);
                }

                for (int i = 0; i < list->size(); i++) {
                    auto list_result = list->at(i);
                    if (list_result->getType() != ValueType::List) {
                        runtimeError("Expected a list, but got " + getTypeStr(list_result->getType()), line, column);
                    }
                    auto list = list_result->get<std::shared_ptr<List>>();
                    if (list->size() > list_node->list.size()) {
                        runtimeError("Too many arguments to unpack", line, column);
                    } else if (list->size() < list_node->list.size()) {
                        runtimeError("Too few arguments to unpack", line, column);
                    }

                    for (int index = 0; index < list->size(); index++) {
                        auto ident_node = std::dynamic_pointer_cast<IdentifierNode>(list_node->list.at(index));
                        if (!ident_node) {
                            runtimeError("Can only assign values to identifiers", line, column);
                        }
                        env.set(ident_node->name, list->at(index));
                    }
                    
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
        }
        else if (container_result.value()->getType() == ValueType::Dictionary) {
            auto dict = container_result.value()->get<std::shared_ptr<Dictionary>>();
            auto ident_node = dynamic_cast<IdentifierNode*>(init_node->left.get());
            if (ident_node) {
                std::string var_string = ident_node->name;

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
            } else {
                auto list_node = std::dynamic_pointer_cast<ListNode>(init_node->left);
                if (!list_node) {
                    runtimeError("For loop expected identifier or list", line, column);
                }
                if (list_node->list.size() < 2) {
                    runtimeError("Too many arguments to unpack", line, column);
                } else if (list_node->list.size() > 2) {
                    runtimeError("Too few arguments to unpack", line, column);
                }
                auto first_node = std::dynamic_pointer_cast<IdentifierNode>(list_node->list.at(0));
                auto second_node = std::dynamic_pointer_cast<IdentifierNode>(list_node->list.at(1));

                for (const auto& pair : *dict) {
                    env.set(first_node->name, pair.first);
                    env.set(second_node->name, pair.second);
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
        }
        else if (container_result.value()->getType() == ValueType::String) {
            auto string = container_result.value()->get<std::string>();
            auto ident_node = dynamic_cast<IdentifierNode*>(init_node->left.get());
            std::string var_string = ident_node->name;

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
    if (debug) std::cout << "Evaluate Keyword" << std::endl;
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
    } else if (keyword == TokenType::_Global) {
        if (auto ident = std::dynamic_pointer_cast<IdentifierNode>(right)) {
            env.addGlobal(ident->name);
        } else {
            runtimeError("global expected an identifier", line, column);
        }
    } else if (keyword == TokenType::_Import) {
        std::string path = currentExecutionContext();
        std::string new_path = path.substr(0, path.find_last_of('/'));
        auto right_value = right->evaluate(env);
        if (!right_value.has_value() || right_value.value()->getType() != ValueType::String) {
            runtimeError("Import expected filename string", line, column);
        }
        new_path = new_path + "/" + right_value.value()->get<std::string>() + ".fy";
        if (new_path == path) {
            runtimeError("A file cannot import itself", line, column);
        }

        std::string source_code = readSourceCodeFromFile(new_path);

        if (source_code.empty()) {
            runtimeError("File " + new_path + " is empty or could not be read", line, column);
        }

        pushExecutionContext(new_path);

        Lexer lexer{source_code};
        auto tokens = lexer.tokenize();

        Parser parser{tokens};
        std::vector<std::shared_ptr<ASTNode>> statements;
        statements = parser.parse();


        for (auto statement : statements) {
            try {
                auto result = statement->evaluate(env);
            }
            catch (const ReturnException) {
                runtimeError("Return was used outside of function");
            }
            catch (const BreakException) {
                runtimeError("Break was used outside of loop");
            }
            catch (const ContinueException) {
                runtimeError("Continue was used outside of loop");
            }
            catch (const StackOverflowException) {
                handleError("Maximum recursion depth exceeded", 0, 0, "StackOverflowError");
            }
            catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
        }

        popExecutionContext();
        return std::nullopt;
    } else if (type_map.contains(keyword)) {
        if (keyword == TokenType::_NullType) {
            return std::make_shared<Value>();
        }
        return std::make_shared<Value>(type_map[keyword]);
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
        else if (eval.value()->getType() == ValueType::Dictionary) {
            auto dict_val = eval.value()->get<std::shared_ptr<Dictionary>>();
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
                                                                    std::shared_ptr<Dictionary>> distr,
                                                        int index, int line, int column) {
    // The function is given the container to extract the index from

    if (std::holds_alternative<std::shared_ptr<std::string>>(distr)) {
        auto string = std::get<std::shared_ptr<std::string>>(distr);
        if (index >= 0 && index < string->length()) {
            return string->at(index);
        } else if (index >= string->length() * -1 && index < 0) {
            return string->at(string->length() - -index);
        } else {
            runtimeError("String index out of range", line, column);
        }
    } else if (std::holds_alternative<std::shared_ptr<List>>(distr)) {
        auto list = std::get<std::shared_ptr<List>>(distr);
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
            runtimeError("Invalid index type");
        };

        auto start_result = start_index->evaluate(env);
        auto end_result = end_index->evaluate(env);

        if (start_result && end_result) {
            // Check if the container is a string or list
            if (std::holds_alternative<std::shared_ptr<std::string>>(distr)) {
                auto str = std::get<std::shared_ptr<std::string>>(distr);
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
            } else if (std::holds_alternative<std::shared_ptr<List>>(distr)) {
                auto list_ptr = std::get<std::shared_ptr<List>>(distr);
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
                    new_list->push_back(list_val);
                }
                return std::make_shared<Value>(new_list);
            } else {
                runtimeError("Invalid type for getting index", line, column);
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
                if (result.value()->getType() == ValueType::Integer) {
                    int int_val = result.value()->get<int>();
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
                    runtimeError("The index was not given an int", line, column);
                }
            } else {
                runtimeError("Failed to evaluate start_index", line, column);
                return std::nullopt;
            }
        }
    }
    
    return std::nullopt;
}

void setAtIndex(std::variant<std::shared_ptr<List>, std::shared_ptr<Dictionary>> env_dist, std::shared_ptr<Value> index_key, std::shared_ptr<Value> value) {
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
                        env_list->push_back(char_str);
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
                        env_list->push_back(list_val->at(i));
                    } else {
                        env_list->insert(list_index, list_val->at(i));
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
    if (env_val->getType() == ValueType::List) {
        std::shared_ptr<List> env_list = env_val->get<std::shared_ptr<List>>();

        if (end_index == nullptr) {
            // A single index assignment
            auto index_eval = start_index->evaluate(env);
            if (index_eval) {
                if (index_eval.value()->getType() == ValueType::Integer) {
                    int index = index_eval.value()->get<int>();
                    if (index >= 0 && index < env_list->size()) {
                        env_list->set(index, value);
                    } else if (index < 0 && index >= env_list->size() * -1) {
                        env_list->set(env_list->size() - index, value);
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
    else if (env_val->getType() == ValueType::Dictionary) {
        std::shared_ptr<Dictionary> env_dict = env_val->get<std::shared_ptr<Dictionary>>();

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

std::optional<std::shared_ptr<Value>> FuncNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Function" << std::endl;
    local_env = Environment{env};
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

std::optional<std::shared_ptr<Value>> FuncNode::callFunc(std::vector<std::shared_ptr<Value>> values, Environment& global_env) {
    Scope local_scope;
    local_scope.set(*func_name, global_env.get(*func_name));
    setArgs(values, local_scope);
    if (global_env.isClassEnv()) {
        local_env.removeScope();
        local_env.setClassEnv(global_env.getClassGlobals());
    }
    local_env.addScope(local_scope);
    recursion += 1;
    if (recursion > 500) {
        throw StackOverflowException();
    }
    for (auto statement : block) {
        try {
            auto result = statement->evaluate(local_env);
        }
        catch (const ReturnException& e) {
            return e.value;
        }
    }

    if (global_env.isClassEnv()) {
        global_env.setClassGlobals(local_env.getClassGlobals());
    }

    auto scopes = local_env.copyScopes();
    for (const auto& pair : scopes.at(0).getPairs()) {
        global_env.setGlobalValue(pair.first, pair.second);
    }
    recursion -= 1;

    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> MethodCallNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Method Call" << std::endl;
    auto mapped_value = identifier->evaluate(env).value();
    if (mapped_value->getType() == ValueType::Function) {
        auto func_value = mapped_value->get<std::shared_ptr<ASTNode>>();
        if (auto func = dynamic_cast<FuncNode*>(func_value.get())) {
            return func->callFunc(evaluateArgs(env), env);
        } else {
            runtimeError("Unable to call function " + dynamic_cast<IdentifierNode*>(identifier.get())->name, line, column);
        }
    } else if (mapped_value->getType() == ValueType::BuiltInFunction) {
        auto func_value = mapped_value->get<std::shared_ptr<BuiltInFunction>>();
        std::vector<std::shared_ptr<Value>> values = evaluateArgs(env);
        try {
            return (*func_value)(values, env);
        }
        catch (const std::exception& e) {
            runtimeError(e.what(), line, column);
        }
    } else if (mapped_value->getType() == ValueType::Class) {
        auto class_value = mapped_value->get<std::shared_ptr<Class>>();
        try {
            std::shared_ptr<Instance> instance = class_value->createInstance();
            auto constructor = instance->getConstructor();
            auto node = constructor->get<std::shared_ptr<ASTNode>>();
            auto func_node = std::static_pointer_cast<FuncNode>(node);
            func_node->callFunc(evaluateArgs(env), instance->getEnvironment());
            return std::make_shared<Value>(instance);
        }
        catch (const std::exception& e) {
            runtimeError(e.what(), line, column);
        }
    }
    else {
        runtimeError("Object type " + getValueStr(mapped_value) + " is not callable", line, column);
    }
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> MethodCallNode::evaluate(Environment& env, ValueType member_type) {
    if (debug) std::cout << "Evaluate Function Call" << std::endl;
    auto ident_node = std::static_pointer_cast<IdentifierNode>(identifier);
    Environment environment{env};
    if (member_value->getType() == ValueType::Instance) {
        auto inst_node = member_value->get<std::shared_ptr<Instance>>();
        environment = inst_node->getEnvironment();
    }
    std::shared_ptr<Value> mapped_value;
    mapped_value = ident_node->evaluate(environment, member_type).value();
    if (mapped_value->getType() == ValueType::Function) {
        auto func_value = mapped_value->get<std::shared_ptr<ASTNode>>();
        if (auto func = dynamic_cast<FuncNode*>(func_value.get())) {
            auto values = evaluateArgs(env);
            if (member_type == ValueType::Instance) {
                auto result = func->callFunc(values, environment);
                auto inst_node = member_value->get<std::shared_ptr<Instance>>();
                inst_node->getEnvironment().setClassGlobals(environment.getClassGlobals());
                return result;
            }
            else {
                values.insert(values.begin(), member_value);
                return func->callFunc(values, environment);
            }
        } else {
            runtimeError("Unable to call function " + dynamic_cast<IdentifierNode*>(identifier.get())->name, line, column);
        }
    } else if (mapped_value->getType() == ValueType::BuiltInFunction) {
        auto func_value = mapped_value->get<std::shared_ptr<BuiltInFunction>>();
        auto values = evaluateArgs(env);
        values.insert(values.begin(), member_value);
        try {
            return (*func_value)(values, environment);
        }
        catch (const std::exception& e) {
            runtimeError(e.what(), line, column);
        }
    }
    else {
        runtimeError("Object type " + getTypeStr(member_type) + " has no member function " + ident_node->name, line, column);
    }
    return std::nullopt;
}

std::vector<std::shared_ptr<Value>> MethodCallNode::evaluateArgs(Environment& env) {
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

std::optional<std::shared_ptr<Value>> DictionaryNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Dict" << std::endl;
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

std::optional<std::shared_ptr<Value>> ClassNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Class" << std::endl;
    // Prevent the constructor overwriting the class name in the env
    env.addScope();
    try {
        for (auto statement : block) {
            statement->evaluate(env);
        }
    }
    catch (const ReturnException) {
        runtimeError("Return was used outside of function");
    }
    catch (const BreakException) {
        runtimeError("Break was used outside of loop");
    }
    catch (const ContinueException) {
        runtimeError("Continue was used outside of loop");
    }
    catch (const StackOverflowException) {
        handleError("Maximum recursion depth exceeded", 0, 0, "StackOverflowError");
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    local_scope = env.getScope();
    if (!local_scope.contains(name)) {
        runtimeError("Class " + name + " is missing a constructor", line, column);
    }

    env.removeScope();

    Environment class_env{env};
    class_env.setClassEnv(local_scope);

    return std::make_shared<Value>(std::make_shared<Class>(name, class_env));
}