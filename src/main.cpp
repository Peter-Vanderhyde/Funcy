#include <iostream>
#include <vector>
#include "library.h"
#include "lexer.h"
#include "parser.h"


int main(int argc, char* argv[]) {
    bool TESTING = true;
    if (!TESTING && argc < 2) {
        throw std::runtime_error("Program usage: Funcy <program_path>");
        return 0;
    }

    std::string filename = ""; // Replace with your file name
    if (TESTING) {
        filename = "../scripts/test.funcy";
    } else {
        filename = argv[1];
    }
    std::string source_code = readSourceCodeFromFile(filename);

    if (source_code.empty()) {
        throw std::runtime_error("File " + filename + " is empty or could not be read.");
    }

    Lexer lexer{source_code};
    std::vector<Token> tokens;
    try {
        tokens = lexer.tokenize();
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    // std::cout << "TOKENS:\n";
    // for (Token t : tokens) {
    //     std::cout << getTokenTypeLabel(t.type) << std::endl;
    // }

    Parser parser{tokens};
    std::vector<std::shared_ptr<ASTNode>> statements;
    try {
        statements = parser.parse();
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    Environment env;

    for (auto statement : statements) {
        try {
            auto result = statement->evaluate(env);
            if (result.has_value()) {
                printValue(result.value(), env);
                std::cout << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }

    return 0;
}