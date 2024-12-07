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
    std::vector<std::shared_ptr<ScopedNode>> last_if_else{nullptr};

    std::optional<const Token*> peekToken(int ahead = 1) const;
    const Token& getToken() const;
    const Token& consumeToken();
    std::string getTokenStr() const;
    bool tokenIs(std::string str) const;
    bool nextTokenIs(std::string str, int ahead = 1) const;

    std::shared_ptr<ASTNode> parseFoundation();
    std::shared_ptr<ASTNode> parseControlFlowStatement();
    std::shared_ptr<ASTNode> parseKeyword();
    std::shared_ptr<ASTNode> parseStatement(std::shared_ptr<std::string> varString = nullptr);
    std::shared_ptr<ASTNode> parseLogicalOr();
    std::shared_ptr<ASTNode> parseLogicalAnd();
    std::shared_ptr<ASTNode> parseEquality();
    std::shared_ptr<ASTNode> parseRelation();
    std::shared_ptr<ASTNode> parseExpression();
    std::shared_ptr<ASTNode> parseTerm();
    std::shared_ptr<ASTNode> parseFactor();
    std::shared_ptr<ASTNode> parsePower();
    std::shared_ptr<ASTNode> parseLogicalNot();
    std::shared_ptr<ASTNode> parseCollection();
    std::shared_ptr<ASTNode> parseAtom();
    std::shared_ptr<ASTNode> parseIdentifier(std::shared_ptr<std::string> varString = nullptr);
public:
    explicit Parser(const std::vector<Token>& tokens);
    [[noreturn]] void parsingError(std::string message, int line, int column) const;
    // noreturn used so compiler doesn't complain about functions not returning when
    // calling this error function

    std::vector<std::shared_ptr<ASTNode>> parse();

    void addIfElseScope();
    void removeIfElseScope();
};