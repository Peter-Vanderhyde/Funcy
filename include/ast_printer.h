#pragma once
#include "parser.h"

class ASTPrinter {
public:
    void print(const std::vector<std::shared_ptr<ASTNode>>& nodes) const;

private:
    void printNode(const ASTNode& node, int depth) const;
};