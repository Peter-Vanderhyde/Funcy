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
    if (const auto* atom_node = dynamic_cast<const AtomNode*>(&node)) {
        std::cout << indent(depth + 1) << "Atom: ";
        if (atom_node->isInteger()) {
            std::cout << atom_node->getInteger() << "\n";
        } else  if (atom_node->isFloat()) {
            std::cout << atom_node->getFloat() << "\n";
        } else if (atom_node->isBool()) {
            std::cout << std::boolalpha << atom_node->getBool() << '\n';
        } else if (atom_node->isString()) {
            std::cout << '"' << atom_node->getString() << '"' << '\n';
        }
    } else if (const auto* ident_node = dynamic_cast<const IdentifierNode*>(&node)) {
        std::cout << indent(depth + 1) << "Identifier: ";
        std::cout << ident_node->value << '\n';
    } else if (const auto* bin_op_node = dynamic_cast<const BinaryOpNode*>(&node)) {
        std::cout << indent(depth + 1) << "Operator: " << token_labels[bin_op_node->op] << "\n";
        if (bin_op_node->left) printNode(*bin_op_node->left, depth + 1);
        if (bin_op_node->right) printNode(*bin_op_node->right, depth + 1);
    } else if (const auto* un_op_node = dynamic_cast<const UnaryOpNode*>(&node)) {
        std::cout << indent(depth + 1) << "Operator: " << token_labels[un_op_node->op] << "\n";
        if (un_op_node->right) printNode(*un_op_node->right, depth + 1);
    } else if (const auto* paren_op_node = dynamic_cast<const ParenthesisOpNode*>(&node)) {
        std::cout << indent(depth + 1) << "Parenthesis: ( ... )" << "\n";
        if (paren_op_node->expr) printNode(*paren_op_node->expr, depth + 1);
    } else if (const auto* keyword_node = dynamic_cast<const KeywordNode*>(&node)) {
        std::cout << indent(depth + 1) << "Keyword: " << token_labels[keyword_node->keyword] << '\n';
        if (keyword_node->comparison) printNode(*keyword_node->comparison, depth + 1);
        for (int i = 0; i < keyword_node->statements_block.size(); i++) {
            printNode(*keyword_node->statements_block.at(i), depth + 1);
        }
    }
}
