#include "nodes.h"
#include <iostream>
#include "library.h"
#include <format>
#include <math.h>


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
        std::shared_ptr<Value> left_value = left_opt.value();
        std::shared_ptr<Value> right_value = right_opt.value();

        std::string left_str = getValueStr(left_value);
        std::string right_str = getValueStr(right_value);

        if ((left_str == "integer" || left_str == "float") &&
                    (right_str == "integer" || right_str == "float") &&
                    (left_str == "float" || right_str == "float")) {
            double new_left, new_right;
            if (left_str == "integer") {
                new_left = static_cast<double>(std::get<int>(*left_value));
            } else {
                new_left = std::get<double>(*left_value);
            }
            if (right_str == "integer") {
                new_right = static_cast<double>(std::get<int>(*right_value));
            } else {
                new_right = std::get<double>(*right_value);
            }

            if (op == TokenType::_Plus) {
                return std::make_shared<Value>(new_left + new_right);
            }
            else if (op == TokenType::_Minus) {
                return std::make_shared<Value>(new_left - new_right);
            }
            else if (op == TokenType::_Multiply) {
                return std::make_shared<Value>(new_left * new_right);
            }
            else if (op == TokenType::_Divide) {
                if (new_right == 0.0) {
                    handleError("Float division by zero", line, column, "Zero Division Error");
                }
                return std::make_shared<Value>(new_left / new_right);
            }
            else if (op == TokenType::_DoubleDivide) {
                if (new_right == 0.0) {
                    handleError("Float division by zero", line, column, "Zero Division Error");
                }
                int result = static_cast<int>(new_left / new_right);
                return std::make_shared<Value>(result);
            }
            else if (op == TokenType::_Caret) {
                return std::make_shared<Value>(pow(new_left, new_right));
            }
            else if (op == TokenType::_DoubleMultiply) {
                return std::make_shared<Value>(pow(new_left, new_right));
            }
        }
        else if (left_str == "integer" && right_str == "integer") {
            int left = std::get<int>(*left_value);
            int right = std::get<int>(*right_value);
            if (op == TokenType::_Plus) {
                return std::make_shared<Value>(left + right);
            }
            else if (op == TokenType::_Minus) {
                return std::make_shared<Value>(left - right);
            }
            else if (op == TokenType::_Multiply) {
                return std::make_shared<Value>(left * right);
            }
            else if (op == TokenType::_Divide) {
                if (right == 0) {
                    handleError("Integer division by zero", line, column, "Zero Division Error");
                }
                double result = static_cast<double>(left) / static_cast<double>(right);
                return std::make_shared<Value>(result);
            }
            else if (op == TokenType::_DoubleDivide) {
                if (right == 0) {
                    handleError("Integer division by zero", line, column, "Zero Division Error");
                }
                return std::make_shared<Value>(left / right);
            }
            else if (op == TokenType::_Caret || op == TokenType::_DoubleMultiply) {
                return std::make_shared<Value>(static_cast<int>(pow(left, right)));
            }
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