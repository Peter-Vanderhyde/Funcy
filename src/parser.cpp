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

void Parser::addIfElseScope() {
    last_if_else.push_back(nullptr);
}

void Parser::removeIfElseScope() {
    last_if_else.pop_back();
}


std::shared_ptr<ASTNode> Parser::parseFoundation() {
    if (debug) std::cout << "Parse Foundation " << getTokenStr() << std::endl;
    if (keyword_tokens.contains(getTokenStr())) {
        return parseControlFlowStatement();
    }
    else {
        auto statement = parseStatement();
        if (!tokenIs(";")) {
            handleError("Expected ';' but got " + getTokenStr(), getToken().line, getToken().column, "Syntax Error");
        } else {
            consumeToken();
        }
        return statement;
    }
}

std::shared_ptr<ASTNode> Parser::parseControlFlowStatement() {
    if (debug) std::cout << "Parse Control Flow " << getTokenStr() << std::endl;
    if (peekToken() && peekToken().value()->type == TokenType::_Equals) {
        // Make sure they're not trying to use a keyword as a variable
        parsingError(getTokenStr() + " is a keyword and is not allowed to be redefined", getToken().line, getToken().column);
    }

    std::unordered_map<std::string, TokenType> scoped_keyword_tokens {
        {"if", TokenType::_If},
        {"elif", TokenType::_Elif},
        {"else", TokenType::_Else},
        {"while", TokenType::_While}
    };

    std::string t_str = getTokenStr();
    if (scoped_keyword_tokens.contains(t_str)) {
        const Token& keyword = consumeToken();
        std::shared_ptr<ASTNode> comparison_expr = nullptr;
        std::shared_ptr<ASTNode> for_initialization;
        auto variable_str = std::make_shared<std::string>(""); // For loop saves the string of the variable to increment
        std::shared_ptr<ASTNode> for_increment;
        std::vector<std::shared_ptr<ASTNode>> func_args;
        std::shared_ptr<ASTNode> func_name;
        auto func_str = std::make_shared<std::string>("");
        if (t_str == "if" || t_str == "elif" || t_str == "while") {
            if (tokenIs("{")) {
                parsingError("Missing boolean expression ", getToken().line, getToken().column);
            }

            comparison_expr = parseLogicalOr();
        }

        if (!tokenIs("{")) {
            parsingError("Expected '{' but got " + getTokenStr(), getToken().line, getToken().column);
        }
        consumeToken();

        // Prevent connected elif to if outside of scope
        addIfElseScope();

        std::vector<std::shared_ptr<ASTNode>> block;
        while (!tokenIs("eof") && !tokenIs("}")) {
            block.push_back(parseFoundation());
        }

        if (tokenIs("eof")) {
            parsingError("Expected '}'", getToken().line, getToken().column);
        }
        consumeToken();

        removeIfElseScope();

        if (t_str == "if") {
            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword.type, nullptr, comparison_expr, block, keyword.line, keyword.column);
            last_if_else.back() = keyword_node;
            return keyword_node;
        } else if (t_str == "elif") {
            if (last_if_else.back() == nullptr) {
                parsingError("Missing 'if' before 'elif'", getToken().line, getToken().column);
            }

            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword.type, last_if_else.back(), comparison_expr, block, keyword.line, keyword.column);
            last_if_else.back() = keyword_node;
            return keyword_node;
        } else if (t_str == "else") {
            if (last_if_else.back() == nullptr) {
                parsingError("Missing 'if' before 'else'", getToken().line, getToken().column);
            }

            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword.type, last_if_else.back(), comparison_expr, block, keyword.line, keyword.column);
            last_if_else.back() = nullptr;
            return keyword_node;
        } else {
            return std::make_shared<ScopedNode>(keyword.type, nullptr, comparison_expr, block, keyword.line, keyword.column);
        }
    }
    else {
        auto node = parseKeyword();
        if (tokenIs(";")) {
            consumeToken();
            return node;
        }
        else {
            parsingError("Expected ; but got " + getTokenStr(), getToken().line, getToken().column);
            return nullptr;
        }
    }

    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseKeyword() {
    // Token not, and, or
    std::string t_str = getTokenStr();
    if (t_str == "not" || t_str == "and" || t_str == "or") {
        return parseLogicalOr();
    }
    else {
        const Token& token = getToken();
        std::shared_ptr<KeywordNode> node;

        node = std::make_shared<KeywordNode>(getToken().type, nullptr, token.line, token.column);
        consumeToken();
        return node;
    }

    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    if (debug) std::cout << "Parse Statement " << getTokenStr() << std::endl;
    if (tokenIs("identifier") && peekToken() && (nextTokenIs("=") || nextTokenIs("+=") || nextTokenIs("-=") || nextTokenIs("*=") || nextTokenIs("/="))) {
        auto left = parseIdentifier();
        const Token& op = consumeToken();
        auto right = parseLogicalOr();
        return std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    } else {
        auto left = parseLogicalOr();
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalOr() {
    if (debug) std::cout << "Parse Logical Or " << getTokenStr() << std::endl;
    auto left = parseLogicalAnd();
    while (tokenIs("or")) {
        const Token& op = consumeToken();
        auto right = parseLogicalAnd();
        left = std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseLogicalAnd() {
    if (debug) std::cout << "Parse Logical And " << getTokenStr() << std::endl;
    auto left = parseEquality();
    while (tokenIs("and")) {
        const Token& op = consumeToken();
        auto right = parseEquality();
        left = std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    }
    return left;
}

std::shared_ptr<ASTNode> Parser::parseEquality() {
    if (debug) std::cout << "Parse Equality " << getTokenStr() << std::endl;
    auto left = parseRelation();

    while (tokenIs("==") || tokenIs("!=")) {
        const Token& op = getToken();
        consumeToken();
        auto right = parseRelation();
        left = std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseRelation() {
    if (debug) std::cout << "Parse Relation " << getTokenStr() << std::endl;
    auto left = parseExpression();

    while (tokenIs("<") || tokenIs("<=") || tokenIs(">") || tokenIs(">=")) {
        const Token& op = consumeToken();
        auto right = parseExpression();
        left = std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    }

    return left;
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
    auto left = parseLogicalNot();

    if (tokenIs("^") || tokenIs("**")) {
        const Token& op = consumeToken();
        auto right = parseFactor();
        return std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    }
    else {
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalNot() {
    if (debug) std::cout << "Parse Not " << getTokenStr() << std::endl;
    if (tokenIs("not") || tokenIs("!")) {
        const Token& k_word = consumeToken();
        auto right = parseCollection();
        return std::make_shared<UnaryOpNode>(k_word.type, right, k_word.line, k_word.column);
    }
    else {
        return parseCollection();
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