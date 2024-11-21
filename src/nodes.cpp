#include "nodes.h"
#include <iostream>
#include "library.h"
#include <format>
#include <math.h>
#include <format>


void runtimeError(std::string message, int line, int column) {
    handleError(message, line, column, "Runtime Error");
}

void runtimeError(std::string message) {
    handleError(message, 0, 0, "Runtime Error");
}


ASTNode::ASTNode(int line, int column)
    : line{line}, column{column} {}


AtomNode::AtomNode(std::variant<int, double, bool, std::string> value, int line, int column)
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


UnaryOpNode::UnaryOpNode(TokenType op, std::shared_ptr<ASTNode> right, int line, int column)
    : ASTNode{line, column}, op{op}, right{right} {}

std::optional<std::shared_ptr<Value>> UnaryOpNode::evaluate(Environment& env) {
    if (debug) std::cout << "Evaluate Unary" << std::endl;
    std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);
    if (!right_value.has_value()) {
        runtimeError(std::format("Failed to evaluate unary operand with operator '{}'", getTokenTypeLabel(op)), line, column);
    }

    std::string val_string = getValueStr(right_value.value());
    if (val_string == "integer") {
        if (op == TokenType::_Minus) {
            int val = std::get<int>(*right_value.value());
            return std::make_shared<Value>(-val);
        }
        else if (op == TokenType::_Plus) {
            return right_value.value();
        }
    }
    else if (val_string == "float") {
        if (op == TokenType::_Minus) {
            int val = std::get<double>(*right_value.value());
            return std::make_shared<Value>(-val);
        }
        else if (op == TokenType::_Plus) {
            return right_value.value();
        }
    }
    else if (val_string == "boolean") {
        if (op == TokenType::_Minus) {
            bool val = std::get<bool>(*right_value.value());
            if (val) {
                return std::make_shared<Value>(-1);
            } else {
                return std::make_shared<Value>(0);
            }
        }
        else if (op == TokenType::_Plus) {
            bool val = std::get<bool>(*right_value.value());
            if (val) {
                return std::make_shared<Value>(1);
            } else {
                return std::make_shared<Value>(0);
            }
        }
    }

    runtimeError(std::format("Unsupported operand types for operation. Operation was '{}' {}",
                                getTokenTypeLabel(op), val_string), line, column);
    return std::nullopt;
}


BinaryOpNode::BinaryOpNode(std::shared_ptr<ASTNode> left, TokenType op, std::shared_ptr<ASTNode> right, int line, int column)
    : ASTNode{line, column}, left{left}, op{op}, right{right} {}

std::optional<std::shared_ptr<Value>> BinaryOpNode::performOperation(std::shared_ptr<Value> left_value,
                                                                    std::shared_ptr<Value>(right_value)) {
    std::string left_str = getValueStr(left_value);
    std::string right_str = getValueStr(right_value);

    if (left_str == "string" && right_str == "string") {
        // BOTH STRINGS
        if (op == TokenType::_Plus) {
            std::string lhs = std::get<std::string>(*left_value);
            std::string rhs = std::get<std::string>(*right_value);
            return std::make_shared<Value>(lhs + rhs);
        }
    }

    else if (left_str == "string" && right_str == "integer") {
        // STRING AND INT
        if (op == TokenType::_Multiply) {
            std::string new_str = "";
            std::string copying = std::get<std::string>(*left_value);
            for (int i = 0; i < std::get<int>(*right_value); i++) {
                new_str += copying;
            }
            return std::make_shared<Value>(new_str);
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
        if (op == TokenType::_Plus) {op_result = new_left + new_right;}
        else if (op == TokenType::_Minus) {op_result = new_left - new_right;}
        else if (op == TokenType::_Multiply) {op_result = new_left * new_right;}
        else if (op == TokenType::_Divide) {
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
        else {
            return std::nullopt;
        }

        if (result_type == "int") {
            return std::make_shared<Value>(static_cast<int>(op_result));
        } else {
            return std::make_shared<Value>(op_result);
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
            env.add(ident_node->name, right_opt.value());
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
            return result;
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
    } else {
        runtimeError(std::format("Name '{}' is not defined", name), line, column);
    }
}



std::string getValueStr(std::shared_ptr<Value> value) {
    if (std::holds_alternative<int>(*value)) {
        return "integer";
    } else if (std::holds_alternative<double>(*value)) {
        return "float";
    } else if (std::holds_alternative<bool>(*value)) {
        return "boolean";
    } else if (std::holds_alternative<std::string>(*value)) {
        return "string";
    } else {
        runtimeError("Attempted to get string of unrecognized Value type.");
    }
}

std::string getValueStr(Value value) {
    if (std::holds_alternative<int>(value)) {
        return "integer";
    } else if (std::holds_alternative<double>(value)) {
        return "float";
    } else if (std::holds_alternative<bool>(value)) {
        return "boolean";
    } else if (std::holds_alternative<std::string>(value)) {
        return "string";
    } else {
        runtimeError("Attempted to get string of unrecognized Value type.");
    }
}