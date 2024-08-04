#include "parser.h"


void handleError(const std::string& message, int line, int column) {
    if (line > 0 && column > 0) {
        throw std::runtime_error(std::format("{} at line {} column {}", message, line, column));
    } else {
        throw std::runtime_error(message);
    }
}


std::optional<std::shared_ptr<Value>> AtomNode::evaluate(Environment& env) const {
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


std::optional<std::shared_ptr<Value>> IdentifierNode::evaluate(Environment& env) const {
    if (env.has(value)) {
        return env.get(value);
    } else {
        return {};
    }
}

template <typename T1, typename T2>
std::optional<std::shared_ptr<Value>> doArithmetic(const T1 lhs, const T2 rhs, const TokenType op) {
    if constexpr (std::is_same_v<T1, std::string> && std::is_same_v<T2, std::string>) {
        // BOTH STRINGS
        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) return std::make_shared<Value>(lhs + rhs);
        else if (op == TokenType::_Compare) return std::make_shared<Value>(lhs == rhs);
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(lhs != rhs);
    }
    else if constexpr ((std::is_same_v<T1, int> || std::is_same_v<T1, bool> || std::is_same_v<T1, double>) && (std::is_same_v<T2, int> || std::is_same_v<T2, bool> || std::is_same_v<T2, double>)) {
        if (std::is_same_v<T1, double> || std::is_same_v<T2, double>) {
            if (!std::is_same_v<T1, T2>) {
                double lhs_double = static_cast<double>(lhs);
                double rhs_double = static_cast<double>(rhs);
            }
        }
        // MIX OF INTS OR BOOLS OR DOUBLES
        if (op == TokenType::_Plus || op == TokenType::_PlusEquals) return std::make_shared<Value>(lhs + rhs);
        else if (op == TokenType::_Minus || op == TokenType::_MinusEquals) return std::make_shared<Value>(lhs - rhs);
        else if (op == TokenType::_Multiply || op == TokenType::_MultiplyEquals) return std::make_shared<Value>(lhs * rhs);
        else if (op == TokenType::_Divide || op == TokenType::_DivideEquals) {
            if (rhs == 0) throw std::runtime_error("Attempted division by 0.");
            return std::make_shared<Value>(static_cast<double>(lhs) / static_cast<double>(rhs));
        }
        else if (op == TokenType::_FloorDiv) {
            if (rhs == 0) throw std::runtime_error("Attempted division by 0.");
            return std::make_shared<Value>(static_cast<int>(lhs / rhs));
        }
        else if (op == TokenType::_Mod) {
            if (rhs == 0) throw std::runtime_error("Attempted division by 0.");
            if (std::is_same_v<T1, double> || std::is_same_v<T2, double>) {
                throw std::runtime_error("The modulus '%' can only be peformed on ints.");
            } else {
                return std::make_shared<Value>(static_cast<int>(lhs) % static_cast<int>(rhs));
            }
        }
        else if (op == TokenType::_Caret || op == TokenType::_DoubleMultiply) return std::make_shared<Value>(std::pow(lhs, rhs));
        else if (op == TokenType::_Compare) return std::make_shared<Value>(lhs == rhs);
        else if (op == TokenType::_NotEqual) return std::make_shared<Value>(lhs != rhs);
        else if (op == TokenType::_GreaterThan) return std::make_shared<Value>(lhs > rhs);
        else if (op == TokenType::_GreaterEquals) return std::make_shared<Value>(lhs >= rhs);
        else if (op == TokenType::_LessThan) return std::make_shared<Value>(lhs < rhs);
        else if (op == TokenType::_LessEquals) return std::make_shared<Value>(lhs <= rhs);
    }
    else if constexpr ((std::is_same_v<T1, std::string> || std::is_same_v<T2, std::string>) && !std::is_same_v<T1, T2>) {
        // A MIX OF STRING AND BOOL
        return std::nullopt;
    }
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> BinaryOpNode::evaluate(Environment& env) const {
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

std::optional<std::shared_ptr<Value>> UnaryOpNode::evaluate(Environment& env) const {
    std::optional<std::shared_ptr<Value>> right_value = right->evaluate(env);
    if (!right_value.has_value()) {
        throw std::runtime_error(std::format("Failed to evaluate expression with operator '{}': the operand could not be computed.",
                                         token_labels[op]));
    }

    std::string RHS = getValueStr(right_value.value());

    if (RHS == "int") {
        if (op == TokenType::_Minus) {
            int rhs = std::get<int>(*right_value.value().get());
            return std::make_shared<Value>(-rhs);
        }
        else if (op == TokenType::_Plus) {
            return right_value;
        }
    }
    else if (RHS == "double") {
        if (op == TokenType::_Minus) {
            double rhs = std::get<double>(*right_value.value().get());
            return std::make_shared<Value>(-rhs);
        }
        else if (op == TokenType::_Plus) {
            return right_value;
        }
    }

    throw std::runtime_error(std::format("Unsupported operand types for operation. operation was '{}' {}", token_labels[op], RHS));
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
            std::cout << token_labels[getToken()->type];
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

    //std::cout << getTokenStr() << std::endl;
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

bool Parser::nextTokenIs(std::string str) const {
    return str == token_labels[peek().value()->type];
}

std::unique_ptr<ASTNode> Parser::parseStatement() {

    if (tokenIs("Ident") && peek() && (nextTokenIs("=") || nextTokenIs("+=") || nextTokenIs("-=") || nextTokenIs("*=") || nextTokenIs("/="))) {
        auto left = parseIdentifier();

        TokenType op = getToken()->type;
        consume();
        auto right = parseComparison();
        return std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }
    else {
        auto comp = parseComparison();

        // This is pretty cheaty
        return std::make_unique<ParenthesisOpNode>(std::move(comp));
    }
}

std::unique_ptr<ASTNode> Parser::parseComparison() {
    auto left = parseExpression();

    while (tokenIs("==") || tokenIs("!=") || tokenIs("<") || tokenIs("<=") || tokenIs(">") || tokenIs(">=")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseExpression();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left = parseTerm();

    while (tokenIs("+") || tokenIs("-")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseTerm();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left = parseFactor();

    while (tokenIs("*") || tokenIs("/") || tokenIs("//") || tokenIs("%")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseFactor();
        left = std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    if (tokenIs("+") || tokenIs("-")) {
        TokenType op = getToken()->type;
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

    if (tokenIs("^") || tokenIs("**")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseFactor();
        return std::make_unique<BinaryOpNode>(std::move(left), op, std::move(right));
    }
    else {
        return left;
    }
}

std::unique_ptr<ASTNode> Parser::parsePrimary() {
    if (tokenIs("(")) {
        consume();
        auto comp = parseComparison();
        if (!tokenIs(")")) {
            handleError("Expected ')'", getToken()->line, getToken()->column);
        }
        consume();
        return std::make_unique<ParenthesisOpNode>(std::move(comp));
    }
    else if (tokenIs("Ident")) {
        return parseIdentifier();
    }
    else {
        return parseAtom();
    }
}

std::unique_ptr<ASTNode> Parser::parseAtom() {
    if (tokenIs("Int") || tokenIs("Float") || tokenIs("Bool") || tokenIs("String")) {
        const Token* token = getToken();
        if (auto int_value = std::get_if<int>(&token->value)) {
            consume();
            return std::make_unique<AtomNode>(*int_value);
        }
        else if (auto float_value = std::get_if<double>(&token->value)) {
            consume();
            return std::make_unique<AtomNode>(*float_value);
        }
        else if (auto bool_value = std::get_if<bool>(&token->value)) {
            consume();
            return std::make_unique<AtomNode>(*bool_value);
        }
        else if (auto string_value = std::get_if<std::string>(&token->value)) {
            consume();
            return std::make_unique<AtomNode>(*string_value);
        }
    }
    else {
        handleError(std::format("Expected atom but got {}", getTokenStr()), getToken()->line, getToken()->column);
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