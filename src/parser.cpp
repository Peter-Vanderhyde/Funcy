#include "parser.h"
#include "token.h"
#include <format>
#include "library.h"
#include <iostream>


Parser::Parser(const std::vector<Token>& tokens)
    : tokens{tokens}, current_index{0} {}

void Parser::parsingError(std::string message, int line, int column) const {
    handleError(message, line, column, "Syntax Error");
}

std::optional<const Token*> Parser::peekToken(int ahead) const {
    if (current_index + ahead > tokens.size()) {
        const Token& token = getToken();
        parsingError("Attempted to peek out of range", token.line, token.column);
    } else {
        return &tokens[current_index + ahead];
    }
}

const Token& Parser::getToken() const {
    return tokens[current_index];
}

const Token& Parser::consumeToken() {
    const Token& token = tokens[current_index];
    if (token.type == TokenType::_EOF) {
        parsingError("File ended unexpectedly!", token.line, token.column);
    }
    current_index++;
    return token;
}

std::string Parser::getTokenStr() const {
    return getTokenTypeLabel(getToken().type);
}

bool Parser::tokenIs(std::string str) const {
    return str == getTokenStr();
}

bool Parser::nextTokenIs(std::string str, int ahead) const {
    return str == getTokenTypeLabel(peekToken(ahead).value()->type);
}


std::vector<std::shared_ptr<ASTNode>> Parser::parse() {
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (getToken().type != TokenType::_EOF) {
        statements.push_back(parseFoundation());
    }

    return statements;
}


std::shared_ptr<ASTNode> Parser::parseFoundation() {
    if (debug) std::cout << "Parse Foundation " << getTokenStr() << std::endl;
    auto statement = parseStatement();
    if (!tokenIs(";")) {
        handleError("Expected ';' but got " + getTokenStr(), getToken().line, getToken().column, "Syntax Error");
    } else {
        consumeToken();
    }
    return statement;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (debug) std::cout << "Parse Statement " << getTokenStr() << std::endl;
    if (tokenIs("identifier") && peekToken() && nextTokenIs("=")) {
        auto left = parseIdentifier();
        const Token& op = consumeToken();
        auto right = parseExpression();
        return std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    } else {
        auto left = parseExpression();
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    if (debug) std::cout << "Parse Expression " << getTokenStr() << std::endl;
    auto left = parseTerm();

    while (tokenIs("+") || tokenIs("-")) {
        const Token& op = consumeToken();
        auto right = parseTerm();
        left = std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseTerm() {
    if (debug) std::cout << "Parse Term " << getTokenStr() << std::endl;
    auto left = parseFactor();

    while (tokenIs("*") || tokenIs("/") || tokenIs("//") || tokenIs("%")) {
        const Token& op = consumeToken();
        auto right = parseFactor();
        left = std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseFactor() {
    if (debug) std::cout << "Parse Factor " << getTokenStr() << std::endl;
    if (tokenIs("+") || tokenIs("-")) {
        const Token& op = consumeToken();
        auto right = parsePower();
        return std::make_shared<UnaryOpNode>(op.type, right, op.line, op.column);
    }
    else {
        return parsePower();
    }
}

std::shared_ptr<ASTNode> Parser::parsePower() {
    if (debug) std::cout << "Parse Power " << getTokenStr() << std::endl;
    auto left = parseCollection();

    if (tokenIs("^") || tokenIs("**")) {
        const Token& op = consumeToken();
        auto right = parseFactor();
        return std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    }
    else {
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseCollection() {
    if (debug) std::cout << "Parse Collection " << getTokenStr() << std::endl;
    if (tokenIs("(")) {
        const Token& token = consumeToken();
        // auto parse_or = parseLogicalOr();
        auto parse_or = parseExpression();
        if (!tokenIs(")")) {
            parsingError("Expected ')' but got " + getTokenStr(), getToken().line, getToken().column);
        }
        consumeToken();
        return std::make_shared<ParenthesisOpNode>(parse_or, token.line, token.column);
    }
    else {
        return parseAtom();
    }
}

std::shared_ptr<ASTNode> Parser::parseAtom() {
    if (debug) std::cout << "Parse Atom " << getTokenStr() << std::endl;
    if (tokenIs("integer") || tokenIs("float") || tokenIs("boolean") || tokenIs("string")) {
        const Token& token = consumeToken();
        if (std::holds_alternative<int>(token.value)) {
            auto int_value = std::get<int>(token.value);
            return std::make_shared<AtomNode>(int_value, token.line, token.column);
        }
        else if (std::holds_alternative<double>(token.value)) {
            auto float_value = std::get<double>(token.value);
            return std::make_shared<AtomNode>(float_value, token.line, token.column);
        }
        else if (std::holds_alternative<bool>(token.value)) {
            auto bool_value = std::get<bool>(token.value);
            return std::make_shared<AtomNode>(bool_value, token.line, token.column);
        }
        else if (std::holds_alternative<std::string>(token.value)) {
            auto string_value = std::get<std::string>(token.value);
            return std::make_shared<AtomNode>(string_value, token.line, token.column);
        }
    } else if (tokenIs("identifier")) {
        return parseIdentifier();
    }
    else {
        parsingError(std::format("Expected atom but got {}", getTokenStr()), getToken().line, getToken().column);
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseIdentifier() {
    if (debug) std::cout << "Parse Identifier " << getTokenStr() << std::endl;
    if (tokenIs("identifier")) {
        const Token& token = consumeToken();
        if (std::holds_alternative<std::string>(token.value)) {
            return std::make_shared<IdentifierNode>(std::get<std::string>(token.value), token.line, token.column);
        } else {
            parsingError("Identifier was not a string??", token.line, token.column);
        }
    } else {
        parsingError(std::format("Expected identifier but got {}", getTokenStr()), getToken().line, getToken().column);
    }

    return nullptr;
}