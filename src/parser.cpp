#include "parser.h"


void handleError(const std::string& message, int line, int column) {
    if (line > 0 && column > 0) {
        throw std::runtime_error(std::format("{} at line {} column {}", message, line, column));
    } else {
        throw std::runtime_error(message);
    }
}


std::optional<std::shared_ptr<Value>> NumberNode::evaluate(Environment& env) const {
    if (isInteger()) {
        return std::make_shared<Value>(getInteger());
    } else if (isFloat()) {
        return std::make_shared<Value>(getFloat());
    } else {
        handleError("Unable to evaluate number value.", 0, 0);
        return std::nullopt;
    }
}

std::variant<int, double> NumberNode::getValue() const {
    return value;
}

bool NumberNode::isInteger() const {
    return std::holds_alternative<int>(value);
}

bool NumberNode::isFloat() const {
    return std::holds_alternative<int>(value);
}

int NumberNode::getInteger() const {
    return std::get<int>(value);
}

double NumberNode::getFloat() const {
    return std::get<double>(value);
}


std::optional<std::shared_ptr<Value>> IdentifierNode::evaluate(Environment& env) const {
    if (env.has(value)) {
        return env.get(value);
    } else {
        return {};
    }
}


std::optional<std::shared_ptr<Value>> BinaryOpNode::evaluate(Environment& env) const {
    // Postpone evaluating left
    std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);

    if (op == '=') {
        // An equals is a special case
        if (!right_value.has_value()) {
            throw std::runtime_error("Failed to set variable. Operand could not be computed.");
        }

        // Give it the actual left string, not the value of the variable
        if (auto identifierNode = dynamic_cast<IdentifierNode*>(left.get())) {
            env.set(identifierNode->value, right_value.value());
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
                                            op));
        }

        std::string LHS = getValueStr(left_value.value());
        std::string RHS = getValueStr(right_value.value());


        if (LHS == "int" && RHS == "int") {
            int lhs = std::get<int>(*left_value.value().get());
            int rhs = std::get<int>(*right_value.value().get());
            if (op == '+') return std::make_shared<Value>(lhs + rhs);
            else if (op == '-') return std::make_shared<Value>(lhs - rhs);
            else if (op == '*') return std::make_shared<Value>(lhs * rhs);
            else if (op == '/') {
                if (rhs == 0) throw std::runtime_error("Attempted division by 0.");
                return std::make_shared<Value>(lhs / rhs);
            }
            else if (op == '^') return std::make_shared<Value>(std::pow(lhs, rhs));
        }
        else if (LHS == "double" && RHS == "double") {
            double lhs = std::get<double>(*left_value.value().get());
            double rhs = std::get<double>(*right_value.value().get());
            if (op == '+') return std::make_shared<Value>(lhs + rhs);
            else if (op == '-') return std::make_shared<Value>(lhs - rhs);
            else if (op == '*') return std::make_shared<Value>(lhs * rhs);
            else if (op == '/') {
                if (rhs == 0.0) throw std::runtime_error("Attempted division by 0.");
                return std::make_shared<Value>(lhs / rhs);
            }
            else if (op == '^') return std::make_shared<Value>(std::pow(lhs, rhs));
        }
        else if (LHS == "string" && RHS == "string") {
            std::string lhs = std::get<std::string>(*left_value.value().get());
            std::string rhs = std::get<std::string>(*right_value.value().get());
            if (op == '+') return std::make_shared<Value>(lhs + rhs);
        }
        
        throw std::runtime_error(std::format("Unsupported operand types for operation. operation was {} '{}' {}", LHS, op, RHS));
    }
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> UnaryOpNode::evaluate(Environment& env) const {
    std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);
    if (!right_value.has_value()) {
        throw std::runtime_error(std::format("Failed to evaluate expression with operator '{}': the operand could not be computed.",
                                         op));
    }

    std::string RHS = getValueStr(right_value.value());

    if (RHS == "int") {
        if (op == '-') {
            int rhs = std::get<int>(*right_value.value().get());
            return std::make_shared<Value>(-rhs);
        }
        else if (op == '+') {
            return right_value;
        }
    }
    else if (RHS == "double") {
        if (op == '-') {
            double rhs = std::get<double>(*right_value.value().get());
            return std::make_shared<Value>(-rhs);
        }
        else if (op == '+') {
            return right_value;
        }
    }

    throw std::runtime_error(std::format("Unsupported operand types for operation. operation was '{}' {}", op, RHS));
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> ParenthesisOpNode::evaluate(Environment& env) const {
    std::optional<std::shared_ptr<Value>> expr_value = expr->evaluate(env);
    return expr_value;
}



const Token* Parser::getToken() const {
    return &tokens.at(token_index);
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    while (getToken()->type != TokenType::_EndOfFile) {
        statements.push_back(parseStatement());
        if (getToken()->type != TokenType::_Semi) {
            handleError("Expected ';'", getToken()->line, getToken()->column);
        }
        else {
            consume();
        }
    }

    return statements;
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


std::unique_ptr<ASTNode> Parser::parseStatement() {
    if (tokenIs("Ident") && peek().has_value() && peek().value()->type == TokenType::_Equals) {
        auto left = parseIdentifier();
        if (!tokenIs("=")) {
            handleError(std::format("Expected '=', but got {}", getTokenStr()), getToken()->line, getToken()->column);
        }

        char op = '=';
        consume();
        auto right = parseExpression();
        return std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }
    else {
        auto expr = parseExpression();
        // This is pretty cheaty
        return std::make_unique<ParenthesisOpNode>(std::move(expr));
    }
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseTerm();

    while (tokenIs("+") || tokenIs("-")) {
        char op = getTokenStr()[0];
        consume();
        auto right = parseTerm();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();

    while (tokenIs("*") || tokenIs("/")) {
        char op = getTokenStr()[0];
        consume();
        auto right = parseFactor();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    if (tokenIs("+") || tokenIs("-")) {
        char op = getTokenStr()[0];
        consume();
        auto right = parsePower();
        return std::make_unique<UnaryOpNode>(op, std::move(right));
    }
    else {
        return parsePower();
    }
}

std::unique_ptr<ASTNode> Parser::parsePower() {
    auto left = parsePrimary();

    if (tokenIs("^")) {
        consume();
        auto right = parseFactor();
        return std::make_unique<BinaryOpNode>(std::move(left), '^', std::move(right));
    }
    else {
        return left;
    }
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    if (tokenIs("(")) {
        consume();
        auto expr = parseExpression();
        if (!tokenIs(")")) {
            handleError("Expected ')'", getToken()->line, getToken()->column);
        }
        consume();
        return std::make_unique<ParenthesisOpNode>(std::move(expr));
    }
    else if (tokenIs("Ident")) {
        return parseIdentifier();
    }
    else {
        return parseNumber();
    }
}

std::unique_ptr<ASTNode> Parser::parseNumber() {
    if (tokenIs("Int") || tokenIs("Float")) {
        const Token* token = getToken();
        if (auto int_value = std::get_if<int>(&token->value)) {
            consume();
            return std::make_unique<NumberNode>(*int_value);
        }
        else if (auto float_value = std::get_if<double>(&token->value)) {
            consume();
            return std::make_unique<NumberNode>(*float_value);
        }
    }
    else {
        handleError(std::format("Expected number but got {}", getTokenStr()), getToken()->line, getToken()->column);
    }
    return nullptr;
}

std::unique_ptr<ASTNode> Parser::parseIdentifier() {
    if (tokenIs("Ident")) {
        const Token* token = getToken();
        consume();
        if (auto ident_value = std::get_if<std::string>(&token->value)) {
            return std::make_unique<IdentifierNode>(*ident_value);
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