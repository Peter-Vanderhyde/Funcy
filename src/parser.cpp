#include "parser.h"


void handleError(const std::string& message, int line, int column) {
    if (line > 0 && column > 0) {
        throw std::runtime_error(std::format("{} at line {} column {}", message, line, column));
    } else {
        throw std::runtime_error(message);
    }
}

void printValue(const std::shared_ptr<Value> value) {
    if (auto int_value = std::get_if<int>(value.get())) {
        std::cout << *int_value << std::endl;
    } else if (auto double_value = std::get_if<double>(value.get())) {
        std::cout << *double_value << std::endl;
    } else if (auto bool_value = std::get_if<bool>(value.get())) {
        std::cout << std::boolalpha << *bool_value << std::endl;
    } else if (auto string_value = std::get_if<std::string>(value.get())) {
        std::cout << *string_value << std::endl;
    } else {
        throw std::runtime_error("Received invalid value type to print.");
    }
}


std::optional<std::shared_ptr<Value>> AtomNode::evaluate(Environment& env) {
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


std::optional<std::shared_ptr<Value>> IdentifierNode::evaluate(Environment& env) {
    if (env.has(value)) {
        return env.get(value);
    } else {
        throw std::runtime_error(value + " is not defined.");
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
        else if (op == TokenType::_And) {
            return std::make_shared<Value>(lhs.length() > 0 && rhs.length() > 0);
        } else if (op == TokenType::_Or) {
            return std::make_shared<Value>(lhs.length() > 0 || rhs.length() > 0);
        }
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
        else if (op == TokenType::_And) return std::make_shared<Value>(lhs && rhs);
        else if (op == TokenType::_Or) return std::make_shared<Value>(lhs || rhs);
    }
    else if constexpr ((std::is_same_v<T1, std::string> || std::is_same_v<T2, std::string>) && !std::is_same_v<T1, T2>) {
        // A MIX OF STRING AND BOOL
        if (op == TokenType::_And) {
            if constexpr (std::is_same_v<T1, std::string>) {
                return std::make_shared<Value>(lhs.length() > 0 && rhs);
            }
            else if constexpr (std::is_same_v<T2, std::string>) {
                return std::make_shared<Value>(lhs && rhs.length() > 0);
            }
        } else if (op == TokenType::_Or) {
            if constexpr (std::is_same_v<T1, std::string>) {
                return std::make_shared<Value>(lhs.length() > 0 || rhs);
            }
            else if constexpr (std::is_same_v<T2, std::string>) {
                return std::make_shared<Value>(lhs || rhs.length() > 0);
            }
        }
    }
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> BinaryOpNode::evaluate(Environment& env) {
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

std::optional<std::shared_ptr<Value>> UnaryOpNode::evaluate(Environment& env) {
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
        else if (op == TokenType::_Not) {
            int rhs = std::get<int>(*right_value.value().get());
            return std::make_shared<Value>(rhs == 0);
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
        else if (op == TokenType::_Not) {
            double rhs = std::get<double>(*right_value.value().get());
            return std::make_shared<Value>(rhs == 0.0);
        }
    }
    else if (RHS == "bool") {
        if (op == TokenType::_Not) {
            bool rhs = std::get<bool>(*right_value.value().get());
            return std::make_shared<Value>(!rhs);
        }
    }

    throw std::runtime_error(std::format("Unsupported operand types for operation. operation was '{}' {}", token_labels[op], RHS));
    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> ParenthesisOpNode::evaluate(Environment& env) {
    std::optional<std::shared_ptr<Value>> expr_value = expr->evaluate(env);
    return expr_value;
}

bool ScopedNode::getComparisonValue(Environment& env) const {
    auto result = comparison->evaluate(env);
    if (result && getValueStr(result.value()) == "bool") {
        if (auto bool_value = std::get_if<bool>(result.value().get())) {
            return *bool_value;
        } else {
            throw std::runtime_error("Boolean weirdness.");
        }
    }
    else {
        throw std::runtime_error("Syntax Error: Missing a boolean comparison for keyword to evaluate.");
    }
}

std::optional<std::shared_ptr<Value>> ScopedNode::evaluate(Environment& env) {
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

    std::string str = token_labels[keyword];
    if (str == "if" || str == "elif" || str == "else" || str == "while" || str == "for") {
        env.addScope();

        if (str == "while") {
            while (getComparisonValue(env)) {
                try {
                    for (int i = 0; i < statements_block.size(); i++) {
                        auto result = statements_block[i]->evaluate(env);
                        if (result) {
                            printValue(result.value());
                        }
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
            for (int i = 0; i < statements_block.size(); i++) {
                std::optional<std::shared_ptr<Value>> result = statements_block[i]->evaluate(env);
                if (result) {
                    printValue(result.value());
                }
            }
        }

        env.removeScope();
    }

    return std::nullopt;
}

std::optional<std::shared_ptr<Value>> KeywordNode::evaluate(Environment& env) {
    if (keyword == TokenType::_Break) {
        throw BreakException();
    } else if (keyword == TokenType::_Continue) {
        throw ContinueException();
    }

    if (right) {
        return right->evaluate(env);
    } else {
        return {};
    }
}


const Token* Parser::getToken() const {
    return &tokens.at(token_index);
}

std::vector<std::shared_ptr<ASTNode>> Parser::parse() {
    std::vector<std::shared_ptr<ASTNode>> statements;
    while (getToken()->type != TokenType::_EndOfFile) {
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

bool Parser::nextTokenIs(std::string str) const {
    return str == token_labels[peek().value()->type];
}

std::shared_ptr<ASTNode> Parser::parseFoundation() {
    // std::cout << "parse foundation" << std::endl;
    if (keyword_tokens.contains(getTokenStr())) {
        return parseControlFlowStatement();
    }
    else  {
        auto statement =  parseStatement();
        if (!tokenIs(";")) {
            handleError("Expected ';' but got " + getTokenStr(), getToken()->line, getToken()->column);
        } else {
            consume();
            return statement;
        }
        return nullptr;
    }
}

std::shared_ptr<ASTNode> Parser::parseControlFlowStatement() {
    // std::cout << "parse control flow" << std::endl;
    if (peek() && peek().value()->type == TokenType::_Equals) {
        // Make sure they're not trying to use a keyword as a variable
        handleError(getTokenStr() + " is a keyword and is not allowed to be redefined", getToken()->line, getToken()->column);
    }

    std::string t_str = getTokenStr();
    if (scoped_keyword_tokens.contains(getTokenStr())) {
        TokenType keyword = getToken()->type;
        consume();
        std::shared_ptr<ASTNode> comparison = nullptr;
        if (t_str == "if" || t_str == "elif" || t_str == "while") {
            if (tokenIs("{")) {
                handleError("Syntax Error: Missing boolean expression ", getToken()->line, getToken()->column);
            }

            comparison = parseLogicalOr();
        }

        if (!tokenIs("{")) {
            handleError("Syntax Error: Expected '{' but got " + getTokenStr(), getToken()->line, getToken()->column);
        }
        consume();

        // Prevent connected elif to if outside of scope
        addIfElseScope();

        std::vector<std::shared_ptr<ASTNode>> block;
        while (!tokenIs("eof") && !tokenIs("}")) {
            block.push_back(parseFoundation());
        }

        if (tokenIs("eof")) {
            throw std::runtime_error("Syntax Error: Expected '}'.");
        }
        consume();

        removeIfElseScope();

        if (t_str == "if") {
            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword, nullptr, comparison, block);
            last_if_else.back() = keyword_node;
            return keyword_node;
        } else if (t_str == "elif") {
            if (last_if_else.back() == nullptr) {
                handleError("Missing 'if' before 'elif'", getToken()->line, getToken()->column);
            }

            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword, last_if_else.back(), comparison, block);
            last_if_else.back() = keyword_node;
            return keyword_node;
        } else if (t_str == "else") {
            if (last_if_else.back() == nullptr) {
                handleError("Missing 'if' before 'else'", getToken()->line, getToken()->column);
            }

            std::shared_ptr<ScopedNode> keyword_node = std::make_shared<ScopedNode>(keyword, last_if_else.back(), comparison, block);
            last_if_else.back() = nullptr;
            return keyword_node;
        }

        return std::make_shared<ScopedNode>(keyword, nullptr, comparison, block);
    }
    else {
        // Token not, and, or
        if (t_str == "not" || t_str == "and" || t_str == "or") {
            return parseLogicalOr();
        }
        else {
            auto node = std::make_shared<KeywordNode>(getToken()->type);
            consume();
            if (tokenIs(";")) {
                consume();
                return node;
            }
            else {
                handleError("Expected ; but got " + getTokenStr(), getToken()->line, getToken()->column);
                return nullptr;
            }
        }
    }
}

std::shared_ptr<ASTNode> Parser::parseStatement() {
    // std::cout << "parse statement" << std::endl;

    if (tokenIs("ident") && peek() && (nextTokenIs("=") || nextTokenIs("+=") || nextTokenIs("-=") || nextTokenIs("*=") || nextTokenIs("/="))) {
        auto left = parseIdentifier();

        TokenType op = getToken()->type;
        consume();
        auto right = parseLogicalOr();
        return std::make_shared<BinaryOpNode>(left, op, right);
    }
    else {
        return parseLogicalOr();
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalOr() {
    // std::cout << "parse or" << std::endl;
    auto left = parseLogicalAnd();

    if (tokenIs("or")) {
        TokenType k_word = TokenType::_Or;
        consume();
        auto right = parseLogicalAnd();
        return std::make_shared<BinaryOpNode>(left, k_word, right);
    }
    else {
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalAnd() {
    // std::cout << "parse and" << std::endl;
    auto left = parseEquality();

    if (tokenIs("and")) {
        TokenType k_word = TokenType::_And;
        consume();
        auto right = parseEquality();
        return std::make_shared<BinaryOpNode>(left, k_word, right);
    }
    else {
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseEquality() {
    // std::cout << "parse equality" << std::endl;
    auto left = parseRelation();

    while (tokenIs("==") || tokenIs("!=")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseRelation();
        left = std::make_shared<BinaryOpNode>(left, op, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseRelation() {
    // std::cout << "parse relation" << std::endl;
    auto left = parseExpression();

    while (tokenIs("<") || tokenIs("<=") || tokenIs(">") || tokenIs(">=")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseExpression();
        left = std::make_shared<BinaryOpNode>(left, op, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseExpression() {
    // std::cout << "parse expression" << std::endl;
    auto left = parseTerm();

    while (tokenIs("+") || tokenIs("-")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseTerm();
        left = std::make_shared<BinaryOpNode>(left, op, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseTerm() {
    // std::cout << "parse term" << std::endl;
    auto left = parseFactor();

    while (tokenIs("*") || tokenIs("/") || tokenIs("//") || tokenIs("%")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseFactor();
        left = std::make_shared<BinaryOpNode>(left, op, right);
    }

    return left;
}

std::shared_ptr<ASTNode> Parser::parseFactor() {
    // std::cout << "parse factor" << std::endl;
    if (tokenIs("+") || tokenIs("-")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parsePower();
        return std::make_shared<UnaryOpNode>(op, right);
    }
    else {
        return parsePower();
    }
}

std::shared_ptr<ASTNode> Parser::parsePower() {
    // std::cout << "parse power" << std::endl;
    auto left = parseLogicalNot();

    if (tokenIs("^") || tokenIs("**")) {
        TokenType op = getToken()->type;
        consume();
        auto right = parseFactor();
        return std::make_shared<BinaryOpNode>(left, op, right);
    }
    else {
        return left;
    }
}

std::shared_ptr<ASTNode> Parser::parseLogicalNot() {
    // std::cout << "parse not" << std::endl;
    if (tokenIs("not") || tokenIs("!")) {
        TokenType k_word = TokenType::_Not;
        consume();
        auto right = parsePrimary();
        return std::make_shared<UnaryOpNode>(k_word, right);
    }
    else {
        return parsePrimary();
    }
}

std::shared_ptr<ASTNode> Parser::parsePrimary() {
    // std::cout << "parse primary" << std::endl;
    if (tokenIs("(")) {
        consume();
        auto comp = parseLogicalOr();
        if (!tokenIs(")")) {
            handleError("Expected ')'", getToken()->line, getToken()->column);
        }
        consume();
        return std::make_shared<ParenthesisOpNode>(comp);
    }
    else if (tokenIs("ident")) {
        return parseIdentifier();
    }
    else {
        return parseAtom();
    }
}

std::shared_ptr<ASTNode> Parser::parseAtom() {
    // std::cout << "parse atom" << std::endl;
    if (tokenIs("int") || tokenIs("float") || tokenIs("bool") || tokenIs("string")) {
        const Token* token = getToken();
        if (auto int_value = std::get_if<int>(&token->value)) {
            consume();
            return std::make_shared<AtomNode>(*int_value);
        }
        else if (auto float_value = std::get_if<double>(&token->value)) {
            consume();
            return std::make_shared<AtomNode>(*float_value);
        }
        else if (auto bool_value = std::get_if<bool>(&token->value)) {
            consume();
            return std::make_shared<AtomNode>(*bool_value);
        }
        else if (auto string_value = std::get_if<std::string>(&token->value)) {
            consume();
            return std::make_shared<AtomNode>(*string_value);
        }
    }
    else {
        handleError(std::format("Expected atom but got {}", getTokenStr()), getToken()->line, getToken()->column);
    }
    return nullptr;
}

std::shared_ptr<ASTNode> Parser::parseIdentifier() {
    if (tokenIs("ident")) {
        const Token* token = getToken();
        consume();
        if (auto ident_value = std::get_if<std::string>(&token->value)) {
            return std::make_shared<IdentifierNode>(*ident_value);
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