// Peter Vanderhyde
// November 2024
// Project: Funcy Language 2.0

#include <iostream>
#include <vector>
#include "library.h"
#include "lexer.h"
#include "parser.h"
#include "context.h"
#include "errorDefs.h"



int main(int argc, char* argv[]) {

    bool TESTING = false;
    if (!TESTING && argc < 2) {
        runtimeError("Program usage: Funcy <program_path>");
        return 0;
    }

    std::string filename = ""; // Replace with your file name
    if (TESTING) {
        filename = "../scripts/test.fy";
    } else {
        filename = argv[1];
    }
    std::string source_code = readSourceCodeFromFile(filename);

    if (source_code.empty()) {
        try {
            runtimeError("File " + filename + " is empty or could not be read");
        }
        catch (const std::exception& e) {
            std::cerr << e.what();
            return 1;
        }
    }

    pushExecutionContext(filename);

    Lexer lexer{source_code};
    std::vector<Token> tokens;
    try {
        tokens = lexer.tokenize();
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    Parser parser{tokens};
    std::vector<std::shared_ptr<ASTNode>> statements;
    try {
        statements = parser.parse();
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    Environment env = buildStartingEnvironment();

    try {
        for (auto statement : statements) {
            try {
                auto result = statement->evaluate(env);
            }
            catch (const ReturnException) {
                runtimeError("Return was used outside of function");
            }
            catch (const BreakException) {
                runtimeError("Break was used outside of loop");
            }
            catch (const ContinueException) {
                runtimeError("Continue was used outside of loop");
            }
            catch (const StackOverflowException) {
                handleError("Maximum recursion depth exceeded", 0, 0, "StackOverflowError");
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    popExecutionContext();

    return 0;
}