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

    // for (auto token : tokens) {
    //     std::cout << token << std::endl;
    // }

    Parser parser{tokens};
    std::vector<std::shared_ptr<ASTNode>> statements = parser.parse();
    
    // ASTPrinter printer;
    // printer.print(statements);

    Environment env;
    env.addScope();
    int stmnt_num = 0;
    for (auto statement : statements) {
        try {
            stmnt_num += 1;
            std::optional<std::shared_ptr<Value>> result = statement->evaluate(env);
            if (result.has_value()) {
                printValue(result.value());
            }
            else {
                // std::cout << std::format("Result {}: No return.", stmnt_num) << std::endl;
                continue;
            }
        }
        catch (const ReturnException) {
            throw std::runtime_error("Return was used outside of function.");
        }
        catch (const BreakException) {
            throw std::runtime_error("Break was used outside of loop.");
        }
        catch (const ContinueException) {
            throw std::runtime_error("Continue was used outside of loop.");
        }
    }
}