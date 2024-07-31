#include "parser.h"



std::string ParseError::getMessage() const {
    return message;
}

int ParseError::getLine() const {
    return line;
}

int ParseError::getColumn() const {
    return column;
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



void handleError(const std::string& message, int line, int column) {
    throw ParseError("Parser Error: " + message, line, column);
}



const Token Parser::getToken() const {
    return tokens.at(token_index);
}

const Token Parser::consume() {
    Token t{getToken()};
    if (t.type == TokenType::_EndOfFile) {
        handleError("File ended unexpectedly!", t.line, t.column);
    }

    token_index += 1;
    return t;
}

std::optional<Token> Parser::peek(int ahead=1) const {
    if (token_index + ahead >= tokens.size()) {
        return std::nullopt;
    }
    else {
        return tokens.at(token_index + ahead);
    }
}

std::string Parser::getTokenStr() const {
    return token_labels[getToken().type];
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

}

std::unique_ptr<ASTNode> Parser::parseFactor() {

}

std::unique_ptr<ASTNode> Parser::parsePower() {

}

std::unique_ptr<ASTNode> Parser::parsePrimary() {

}

std::unique_ptr<ASTNode> Parser::parseNumber() {

}