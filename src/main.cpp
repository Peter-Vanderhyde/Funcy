// Peter Vanderhyde
// November 2024
// Project: Funcy Language 2.0

// Need to figure out how to allow instances to update in real time when they are manipulatd with this.var

/*
Use a shared_ptr and pass it to a function if the function needs to take ownership and needs to add to the reference counter to keep it alive.
Pass a reference to other functions that just needs to edit the class or get data from it.
When passing the shared_ptr as a reference, just dereference it with *my_pointer.
*/
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
    bool display_tokens = false;

    if (!TESTING && argc < 2) {
        try {
            throwError(ErrorType::Runtime, "Program usage: Funcy <program_path> [-IgnoreOverflow, -ShowTokens, -DebugParser, -Debug]");
        }
        catch (const std::exception& e) {
            std::cerr << e.what();
        }
        return 0;
    }

    std::string filename = ""; // Replace with your file name
    if (TESTING) {
        filename = "../scripts/test.fy";
    } else {
        filename = argv[1];
    }

    if (argc > 2) {
        for (int i = 2; i < argc; i++) {
            std::string flag = argv[i];
            if (flag == "-IgnoreOverflow") {
                ignore_overflow = true;
            } else if (flag == "-ShowTokens") {
                display_tokens = true;
            } else if (flag == "-DebugParser") {
                DEBUG_PARSER = true;
            } else if (flag == "-Debug") {
                DEBUG_AST = true;
            } else {
                try {
                    throwError(ErrorType::Runtime, "Program usage: Unrecognized flag " + flag);
                }
                catch (const std::exception& e) {
                    std::cerr << e.what();
                    return 1;
                }
            }
        }
    }

    std::string source_code;
    try {
        source_code = readSourceCodeFromFile(filename);

        if (source_code.empty()) {
            throwError(ErrorType::Runtime, "File " + filename + " is empty or could not be opened.");
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    pushExecutionContext(filename); // Keeps the current running code's file on top of the stack

    Lexer lexer{source_code};
    std::vector<Token> tokens;
    try {
        tokens = lexer.tokenize();
        if (display_tokens) {
            for (int i = 0; i < tokens.size(); i++) {
                tokens[i].display();
            }
        }

        pushParsingContext(filename);
        Parser parser{tokens};
        std::vector<std::shared_ptr<ASTNode>> statements;
        statements = parser.parse();

        std::shared_ptr<Scope> global_scope = buildStartingEnvironment(); // Create environment and inject the global builtin functions
        DETECT_RECURSION = !ignore_overflow; // Suppress recursion warning if flag disables it
        for (auto statement : statements) {
            try {
                auto result = statement->evaluate(global_scope);
            }
            catch (const ReturnException) {
                throwError(ErrorType::Runtime, "Return was used outside of function");
            }
            catch (const BreakException) {
                throwError(ErrorType::Runtime, "Break was used outside of loop");
            }
            catch (const ContinueException) {
                throwError(ErrorType::Runtime, "Continue was used outside of loop");
            }
            catch (const StackOverflowException) {
                throwError(ErrorType::StackOverflow, "Excessive recursion depth reached. (Add the -IgnoreOverflow flag to the end of \
the program execution to ignore this warning)");
            }
            catch (const ErrorException& e) {
                std::cerr << e.message;
                return 1;
            }
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what();
        return 1;
    }

    popParsingContext();
    popExecutionContext();

    return 0;
}