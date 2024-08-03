#include "lexer.h"
#include "parser.h"
#include "ast_printer.h"
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

    Parser parser{tokens};
    std::vector<std::unique_ptr<ASTNode>> statements = parser.parse();

    Environment env;
    env.addScope();
    int stmnt_num = 0;
    for (const auto& statement : statements) {
        stmnt_num += 1;
        std::optional<std::shared_ptr<Value>> result = statement->evaluate(env);
        if (result.has_value()) {
            auto result_value = result.value();
            if (auto bool_value = std::get_if<bool>(result.value().get())) {
                std::cout << std::format("Result {}: ", stmnt_num) << std::boolalpha << *bool_value << std::endl;
            }
            else if (auto int_value = std::get_if<int>(result.value().get())) {
                std::cout << std::format("Result {}: {}", stmnt_num, *int_value) << std::endl;
            }
            else if (auto double_value = std::get_if<double>(result.value().get())) {
                std::cout << std::format("Result {}: {}", stmnt_num, *double_value) << std::endl;
            }
            else if (auto string_value = std::get_if<std::string>(result.value().get())) {
                std::cout << std::format("Result {}: {}", stmnt_num, *string_value) << std::endl;
            }
        } else {
            // std::cout << std::format("Result {}: No return.", stmnt_num) << std::endl;
            continue;
        }
    }
    // ASTPrinter printer;
    // printer.print(statements);
}