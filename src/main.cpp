#include "lexer.h"
#include <fstream>
#include <sstream>
#include <iostream>


std::string readSourceCodeFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the file's contents into the buffer

    return buffer.str(); // Return the contents as a std::string
}

int main() {
    std::string filename = "../scripts/test.func"; // Replace with your file name
    std::string source_code = readSourceCodeFromFile(filename);

    if (source_code.empty()) {
        throw std::runtime_error("File is empty or could not be read.");
    }

    Lexer lexer{source_code};
    std::vector<Token> tokens = lexer.tokenize();
    for (Token t : tokens) {
        std::cout << t << std::endl;
    }
}