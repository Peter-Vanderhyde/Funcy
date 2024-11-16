#include <iostream>
#include <vector>
#include "library.h"
#include "lexer.h"

int main() {
    // if (argc < 2) {
    //     throw std::runtime_error("Program usage: Funcy <program_path>");
    //     return 0;
    // }

    std::string filename = "../scripts/test.funcy"; // Replace with your file name
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

    std::cout << "TOKENS:\n";
    for (Token t : tokens) {
        std::cout << getTokenTypeLabel(t.type) << std::endl;
    }

    return 0;
}