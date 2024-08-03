#include "ast_printer.h"
#include <typeinfo>

// Helper function to print spaces for indentation
std::string indent(int depth) {
    return std::string(depth * 2, ' ');
}

void ASTPrinter::print(const std::vector<std::unique_ptr<ASTNode>>& nodes) const {
    for (const auto& node : nodes) {
        if (node) {
            printNode(*node, 0);
        }
    }
}

void ASTPrinter::printNode(const ASTNode& node, int depth) const {
    std::cout << indent(depth) << "Node: " << typeid(node).name() << "\n";

    // Use dynamic_cast to downcast to specific node types and print accordingly
    if (const auto* numNode = dynamic_cast<const NumberNode*>(&node)) {
        std::cout << indent(depth + 1) << "Number: ";
        if (numNode->isInteger()) {
            std::cout << numNode->getInteger() << "\n";
        } else {
            std::cout << numNode->getFloat() << "\n";
        }
    } else if (const auto* identNode = dynamic_cast<const IdentifierNode*>(&node)) {
        std::cout << indent(depth + 1) << "Identifier: ";
        std::cout << identNode->value << '\n';
    } else if (const auto* binOpNode = dynamic_cast<const BinaryOpNode*>(&node)) {
        std::cout << indent(depth + 1) << "Operator: " << token_labels[binOpNode->op] << "\n";
        if (binOpNode->left) printNode(*binOpNode->left, depth + 1);
        if (binOpNode->right) printNode(*binOpNode->right, depth + 1);
    } else if (const auto* unOpNode = dynamic_cast<const UnaryOpNode*>(&node)) {
        std::cout << indent(depth + 1) << "Operator: " << token_labels[unOpNode->op] << "\n";
        if (unOpNode->right) printNode(*unOpNode->right, depth + 1);
    } else if (const auto* parenOpNode = dynamic_cast<const ParenthesisOpNode*>(&node)) {
        std::cout << indent(depth + 1) << "Parenthesis: ( ... )" << "\n";
        if (parenOpNode->expr) printNode(*parenOpNode->expr, depth + 1);
    }
}
