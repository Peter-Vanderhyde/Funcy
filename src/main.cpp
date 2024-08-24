#include "lexer.h"
#include "library.h"
#include "ast_printer.h"
#include "pch.h"
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
    env.addFunction("print", std::make_shared<Value>(std::make_shared<BuiltInFunction>(print)));
    env.addFunction("int", std::make_shared<Value>(std::make_shared<BuiltInFunction>(intConverter)));
    env.addFunction("float", std::make_shared<Value>(std::make_shared<BuiltInFunction>(floatConverter)));
    env.addFunction("bool", std::make_shared<Value>(std::make_shared<BuiltInFunction>(boolConverter)));
    env.addFunction("string", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringConverter)));
    env.addFunction("list", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listConverter)));
    env.addFunction("input", std::make_shared<Value>(std::make_shared<BuiltInFunction>(input)));
    env.addFunction("type", std::make_shared<Value>(std::make_shared<BuiltInFunction>(getType)));

    env.addMember(ValueType::List, "size", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listSize)));
    env.addMember(ValueType::List, "append", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listAppend)));
    env.addMember(ValueType::List, "pop", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listPop)));

    env.addScope();
    int stmnt_num = 0;
    for (auto statement : statements) {
        try {
            stmnt_num += 1;
            std::optional<std::shared_ptr<Value>> result = statement->evaluate(env);
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