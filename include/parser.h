#pragma once
#include <vector>
#include "token.h"
#include <memory>
#include "nodes.h"


class Parser {
private:
    bool debug = false;
    const std::vector<Token>& tokens;
    size_t current_index;
public:
    explicit Parser(const std::vector<Token>& tokens);
    [[noreturn]] void parsingError(std::string message, int line, int column) const;
    // noreturn used so compiler doesn't complain about functions not returning when
    // calling this error function
    std::optional<const Token*> peekToken(int ahead = 1) const;
    const Token& getToken() const;
    const Token& consumeToken();
    std::string getTokenStr() const;
    bool tokenIs(std::string str) const;
    bool nextTokenIs(std::string str, int ahead = 1) const;

    std::vector<std::shared_ptr<ASTNode>> parse();
    std::shared_ptr<ASTNode> parseFoundation();
    std::shared_ptr<ASTNode> parseStatement();
    std::shared_ptr<ASTNode> parseEquality();
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseTerm();
    std::shared_ptr<ASTNode> parseFactor();
    std::shared_ptr<ASTNode> parsePower();
    std::shared_ptr<ASTNode> parseCollection();
    std::shared_ptr<ASTNode> parseAtom();
    std::shared_ptr<ASTNode> parseIdentifier();
};