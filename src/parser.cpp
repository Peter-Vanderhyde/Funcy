#include "parser.h"





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



void handleError(const std::string& message, int line, int column) {
    throw std::runtime_error(std::format("{} at line {} column {}", message, line, column));
}



const Token* Parser::getToken() const {
    return &tokens.at(token_index);
}

std::vector<std::unique_ptr<ASTNode>> Parser::parse() {
    std::vector<std::unique_ptr<ASTNode>> statements;
    while (getToken()->type != TokenType::_EndOfFile) {
        statements.push_back(parseExpression());
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

std::string Parser::getTokenStr() const {
    return token_labels[getToken()->type];
}

bool Parser::tokenIs(std::string str) const {
    return str == getTokenStr();
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
        return std::make_unique<ParenthesisOpNode>('(', std::move(expr), ')');
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