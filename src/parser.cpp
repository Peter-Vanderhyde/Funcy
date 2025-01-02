#include "parser.h"
#include "token.h"
#include <format>
#include "library.h"
#include <iostream>
#include "errorDefs.h"


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
    if (str == "ident") {
        runtimeError("Found ident instead of identifier");
    }
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
        {"while", TokenType::_While},
        {"for", TokenType::_For},
        {"func", TokenType::_Func}
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
        } else if (t_str == "for") {
            if (tokenIs("{") || (!tokenIs("identifier") && !tokenIs("["))) {
                parsingError("Missing for loop expression", getToken().line, getToken().column);
            }
            for_initialization = parseStatement(variable_str);
            auto in_node = dynamic_cast<BinaryOpNode*>(for_initialization.get());
            if (in_node && in_node->op != TokenType::_In) {
                if (in_node->op != TokenType::_Equals) {
                    parsingError("Invalid for loop syntax", getToken().line, getToken().column);
                }
                if (!tokenIs(",")) {
                    parsingError("Invalid for loop syntax", getToken().line, getToken().column);
                }
                consumeToken();
                comparison_expr = parseRelation();
                if (!tokenIs(",")) {
                    parsingError("Invalid for loop syntax", getToken().line, getToken().column);
                }
                consumeToken();
                auto var_test = std::make_shared<std::string>("");
                for_increment = parseStatement(var_test);
                if (*var_test != *variable_str) {
                    parsingError("For loop requires manipulation of the initialized variable", getToken().line, getToken().column);
                }
            }
            else {
                if (tokenIs(",")) {
                    parsingError("For loop requires [] surrounding unpacking variables", getToken().line, getToken().column);
                }
            }
        } else if (t_str == "func") {
            if (!tokenIs("identifier")) {
                parsingError("Expected an identifier but got " + getTokenStr(), getToken().line, getToken().column);
            }
            func_name = parseIdentifier(func_str);
            if (!tokenIs("(")) {
                parsingError("Expected an '(' but got " + getTokenStr(), getToken().line, getToken().column);
            }
            consumeToken();
            auto arg_name = std::make_shared<std::string>("");
            std::vector<std::string> arg_strings;
            while (!tokenIs(")") && !tokenIs("EndOfFile") && !tokenIs("{")) {
                if (!tokenIs("identifier")) {
                    parsingError("Expected argument but got " + getTokenStr(), getToken().line, getToken().column);
                }
                func_args.push_back(parseIdentifier(arg_name));
                if (std::find(arg_strings.begin(), arg_strings.end(), *arg_name) == arg_strings.end()) {
                    arg_strings.push_back(*arg_name);
                    *arg_name = "";
                } else {
                    parsingError("Duplicate argument names found in function creation", getToken().line, getToken().column);
                }

                if (tokenIs("identifier")) {
                    parsingError("Expected ',' but got argument", getToken().line, getToken().column);
                } else if (tokenIs(",")) {
                    consumeToken();
                    if (!tokenIs("identifier")) {
                        parsingError("Expected argument but got " + getTokenStr(), getToken().line, getToken().column);
                    }
                }
            }
            if (tokenIs("EndOfFile")) {
                parsingError("Missing ')'", getToken().line, getToken().column);
            } else if (tokenIs("{")) {
                parsingError("Missing ')' before '{'", getToken().line, getToken().column);
            } else {
                consumeToken();
            }
        }

        if (!tokenIs("{")) {
            parsingError("Expected '{' but got " + getTokenStr(), getToken().line, getToken().column);
        }
        consumeToken();

        // Prevent connected elif to if outside of scope
        addIfElseScope();

        std::vector<std::shared_ptr<ASTNode>> block;
        while (!tokenIs("EndOfFile") && !tokenIs("}")) {
            block.push_back(parseFoundation());
        }

        if (tokenIs("EndOfFile")) {
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
        } else if (t_str == "for") {
            // If 'in' was used, only for_initialization will not be nullptr and will contain the variable and list
            return std::make_shared<ForNode>(keyword.type, for_initialization, variable_str, comparison_expr, for_increment, block, keyword.line, keyword.column);
        } else if (t_str == "func") {
            return std::make_shared<BinaryOpNode>(func_name, TokenType::_Equals, std::make_shared<FuncNode>(func_str, func_args, block, keyword.line, keyword.column), keyword.line, keyword.column);
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
        if (tokenIs("return") && !nextTokenIs(";")) {
            consumeToken();
            auto right = parseLogicalOr();
            node = std::make_shared<KeywordNode>(TokenType::_Return, right, token.line, token.column);
        }
        else if (tokenIs("import")) {
            consumeToken();
            auto right = parseAtom();
            node = std::make_shared<KeywordNode>(TokenType::_Import, right, token.line, token.column);
        }
        else if (tokenIs("global")) {
            consumeToken();
            auto right = parseIdentifier();
            node = std::make_shared<KeywordNode>(TokenType::_Global, right, token.line, token.column);
        }

        else {
            node = std::make_shared<KeywordNode>(getToken().type, nullptr, token.line, token.column);
            consumeToken();
        }
        return node;
    }

    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseStatement(std::shared_ptr<std::string> varString) {
    if (debug) std::cout << "Parse Statement " << getTokenStr() << std::endl;
    if (tokenIs("identifier") && peekToken() && (nextTokenIs("=") || nextTokenIs("+=") || nextTokenIs("-=") || nextTokenIs("*=") || nextTokenIs("/="))) {
        auto left = parseIdentifier(varString);

        const Token& op = consumeToken();
        auto right = parseLogicalOr();
        return std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
    } else if (tokenIs("identifier") && peekToken() && nextTokenIs("[")) {
        int i = 1;
        while (!nextTokenIs(";", i)) {
            if (nextTokenIs("=", i) || nextTokenIs("+=", i) || nextTokenIs("-=", i) || nextTokenIs("*=", i) || nextTokenIs("/=", i)) {
                break;
            }
            i++;
        }
        if (nextTokenIs(";", i)) {
            return parseLogicalOr();
        } else {
            auto left = parseIndexing();
            const Token& op = consumeToken();
            auto right = parseLogicalOr();
            return std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
        }
    }
    else {
        auto left = parseLogicalOr();
        if (auto left_list = std::dynamic_pointer_cast<ListNode>(left)) {
            if (tokenIs("=")) {
                const Token& op = consumeToken();
                auto right = parseLogicalOr();
                return std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
            } else {
                return left;
            }
        }
        else {
            return left;
        }
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

    while (tokenIs("<") || tokenIs("<=") || tokenIs(">") || tokenIs(">=") || tokenIs("in") || (tokenIs("not") && nextTokenIs("in"))) {
        if (tokenIs("not")) {
            const Token& token = consumeToken();
            const Token& op = consumeToken();
            auto right = parseExpression();
            left = std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
            left = std::make_shared<UnaryOpNode>(TokenType::_Not, left, token.line, token.column);
        } else {
            const Token& op = consumeToken();
            auto right = parseExpression();
            left = std::make_shared<BinaryOpNode>(left, op.type, right, op.line, op.column);
        }
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
        auto right = parseMemberAccess();
        return std::make_shared<UnaryOpNode>(k_word.type, right, k_word.line, k_word.column);
    }
    else {
        return parseMemberAccess();
    }
}

std::shared_ptr<ASTNode> Parser::parseMemberAccess() {
    if (debug) std::cout << "Parse Member" << std::endl;
    std::shared_ptr<ASTNode> node = parseIndexing();

    while (true) {
        if (tokenIs(".")) {
            const Token& token = consumeToken();
            if (tokenIs("identifier")) {
                std::shared_ptr<ASTNode> right;
                if (nextTokenIs("(")) {
                    right = parseFuncCall();
                } else {
                    right = parseIdentifier();
                }
                node = std::make_shared<BinaryOpNode>(node, TokenType::_Dot, right, token.line, token.column);
            }
        } else if (tokenIs("[")) {
            node = parseIndexing(node);
        } else if (tokenIs("(")) {
            node = parseFuncCall(node);
        } else {
            break;
        }
    }

    return node;
}

std::shared_ptr<ASTNode> Parser::parseIndexing(std::shared_ptr<ASTNode> left) {
    if (debug) std::cout << "Parse Indexing " << getTokenStr() << std::endl;
    if (!left) {
        left = parseCollection();
    }
    while (tokenIs("[")) {
        const Token& token = consumeToken();
        std::shared_ptr<ASTNode> start;
        if (tokenIs(":")) {
            start = std::make_shared<AtomNode>(0, token.line, token.column);
        } else {
            start = parseExpression(); // Assuming it ends up as an int
        }
        if (tokenIs("]")) {
            consumeToken();
            left = std::make_shared<IndexNode>(left, start, nullptr, token.line, token.column);
        } else if (!tokenIs(":")) {
            parsingError("Expected either ']' or ':'", getToken().line, getToken().column);
        } else {
            // Is :
            consumeToken();
            std::shared_ptr<ASTNode> end;
            if (tokenIs("]")) {
                end = std::make_shared<AtomNode>(SpecialIndex::End, token.line, token.column);
            } else {
                end = parseExpression();
            }
            if (!tokenIs("]")) {
                parsingError("Expected ']'", getToken().line, getToken().column);
            }
            consumeToken();
            left = std::make_shared<IndexNode>(left, start, end, token.line, token.column);
        }
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseCollection() {
    if (debug) std::cout << "Parse Collection " << getTokenStr() << std::endl;
    if (tokenIs("[")) {
        const Token& token = consumeToken();
        ASTList list;
        while (!tokenIs("]") && !tokenIs("EndOfFile") && !tokenIs(";")) {
            auto element = parseLogicalOr();
            list.push_back(element);
            if (tokenIs(",") && !nextTokenIs("]")) {
                consumeToken();
            } else if (tokenIs(",")) {
                parsingError("Expected more values", getToken().line, getToken().column);
            }
        }
        if (tokenIs("EndOfFile") || tokenIs(";")) {
            parsingError("Expected ']'", getToken().line, getToken().column);
        }
        consumeToken();
        return std::make_shared<ListNode>(list, token.line, token.column);
    }
    else if (tokenIs("{")) {
        const Token& token = consumeToken();
        ASTDictionary dict;
        while (!tokenIs("}") && !tokenIs("EndOfFile") && !tokenIs(";")) {
            auto key = parseLogicalOr();
            if (!tokenIs(":")) {
                parsingError("Expected ':'", getToken().line, getToken().column);
            }
            consumeToken();
            auto value = parseLogicalOr();
            dict.push_back(std::make_pair(key, value));
            if (tokenIs(",") && !nextTokenIs("}")) {
                consumeToken();
            } else if (tokenIs(",")) {
                parsingError("Expected more values", getToken().line, getToken().column);
            }
        }
        if (tokenIs("EndOfFile") || tokenIs(";")) {
            parsingError("Expected '}'", getToken().line, getToken().column);
        }
        consumeToken();
        return std::make_shared<DictionaryNode>(dict, token.line, token.column);
    }
    else if (tokenIs("(")) {
        const Token& token = consumeToken();
        auto parse_or = parseLogicalOr();
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
    else if (tokenIs("integer") || tokenIs("float") || tokenIs("boolean") || tokenIs("string")) {
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
    } else if (tokenIs("identifier") && peekToken() && peekToken().value()->type == TokenType::_ParenOpen) {
        return parseFuncCall();
    } else if (tokenIs("identifier")) {
        return parseIdentifier();
    } else if (getTokenStr().find("type:") != std::string::npos) {
        return parseKeyword();
    }
    else {
        parsingError(std::format("Expected atom but got {}", getTokenStr()), getToken().line, getToken().column);
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseFuncCall(std::shared_ptr<ASTNode> identifier) {
    if (debug) std::cout << "Parse Func Call " << getTokenStr() << std::endl;
    if (!identifier) {
        if (!tokenIs("identifier")) {
            parsingError("Expected function name but got " + getTokenStr(), getToken().line, getToken().column);
        }
        identifier = parseIdentifier(nullptr);
    }
    if (!tokenIs("(")) {
        parsingError("Missing '(' at function call", getToken().line, getToken().column);
    }
    consumeToken();
    std::vector<std::shared_ptr<ASTNode>> arguments;
    while (!tokenIs(")") && !tokenIs("EndOfFile") && !tokenIs(";")) {
        arguments.push_back(parseLogicalOr());
        if (!tokenIs(")") && !tokenIs(",")) {
            parsingError("Expected ','", getToken().line, getToken().column);
        } else if (tokenIs(",")) {
            consumeToken();
            if (tokenIs(")")) {
                parsingError("Expected another argument", getToken().line, getToken().column);
            }
        }
    }
    if (tokenIs("EndOfFile") || tokenIs(";")) {
        parsingError("Expected ')'", getToken().line, getToken().column);
    }
    consumeToken();
    return std::make_shared<FuncCallNode>(identifier, arguments, identifier->line, identifier->column);
}

std::shared_ptr<ASTNode> Parser::parseIdentifier(std::shared_ptr<std::string> varString) {
    if (debug) std::cout << "Parse Identifier " << getTokenStr() << std::endl;
    if (tokenIs("identifier")) {
        const Token& token = consumeToken();
        if (std::holds_alternative<std::string>(token.value)) {
            auto ident_value = std::get<std::string>(token.value);
            if (varString != nullptr) {
                *varString = ident_value;
            }
            return std::make_shared<IdentifierNode>(ident_value, token.line, token.column);
        } else {
            parsingError("Identifier was not a string??", token.line, token.column);
        }
    } else {
        parsingError(std::format("Expected identifier but got {}", getTokenStr()), getToken().line, getToken().column);
    }

    return nullptr;
}