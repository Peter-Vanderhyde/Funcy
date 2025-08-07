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

bool TESTING = false;


#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
void enableAnsiEscapeCodes() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}
#endif


int main(int argc, char* argv[]) {
    enableAnsiEscapeCodes();

    bool ignore_overflow = false;
    if (!TESTING && argc < 2) {
        runtimeError("Program Usage: Funcy <program_path> [-IgnoreOverflow]", "");
        return 0;
    }

    std::string filename = ""; // Replace with your file name
    if (TESTING) {
        filename = "../scripts/test.fy";
    } else {
        filename = argv[1];
    }

    if (argc == 3) {
        std::string flag = argv[2];
        if (flag == "-IgnoreOverflow") {
            ignore_overflow = true;
        } else {
            runtimeError("Program Usage: Unrecognized flag " + flag, "");
        }
    }
    std::string source_code = readSourceCodeFromFile(filename);

    if (source_code.empty()) {
        try {
            runtimeError("File " + filename + " is empty or could not be read", "");
        }
        catch (const std::exception& e) {
            // Throwing and then catching the error allows for proper error formating
            std::cerr << e.what();
            return 1;
        }
    }

    pushExecutionContext(filename); // Keeps the current running code's file on top of the stack

    Lexer lexer{source_code};
    std::vector<Token> tokens;
    for (int i = 0; i < tokens.size(); i++) {
        tokens[i].display();
    }
    try {
        tokens = lexer.tokenize();

        Parser parser{tokens};
        std::vector<std::shared_ptr<ASTNode>> statements;
        statements = parser.parse();

        Environment env = buildStartingEnvironment(); // Create environment and inject the global builtin functions
        DETECT_RECURSION = !ignore_overflow; // Suppress recursion warning if flag disables it
        for (auto statement : statements) {
            try {
                auto result = statement->evaluate(env);
            }
            catch (const ReturnException) {
                runtimeError("Return was used outside of function", "");
            }
            catch (const BreakException) {
                runtimeError("Break was used outside of loop", "");
            }
            catch (const ContinueException) {
                runtimeError("Continue was used outside of loop", "");
            }
            catch (const StackOverflowException) {
                handleError("Excessive recursion depth reached. (Add the -IgnoreOverflow flag to the end of \
the program execution to ignore this warning)", 0, 0, "StackOverflowWarning", "");
            }
            catch (const ErrorException& e) {
                printValue(e.value, true);
                return 1;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    popExecutionContext();

    return 0;
}