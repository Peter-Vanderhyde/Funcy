// Peter Vanderhyde
// November 2024
// Project: Funcy Language 2.0

#include <iostream>
#include <vector>
#include "library.h"
#include "lexer.h"
#include "parser.h"
#include "context.h"



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
        runtimeError("File " + filename + " is empty or could not be read.");
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
    env.addScope();

    env.addFunction("print", std::make_shared<Value>(std::make_shared<BuiltInFunction>(print)));
    env.addFunction("int", std::make_shared<Value>(std::make_shared<BuiltInFunction>(intConverter)));
    env.addFunction("float", std::make_shared<Value>(std::make_shared<BuiltInFunction>(floatConverter)));
    env.addFunction("bool", std::make_shared<Value>(std::make_shared<BuiltInFunction>(boolConverter)));
    env.addFunction("str", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringConverter)));
    env.addFunction("list", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listConverter)));
    env.addFunction("dict", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictConverter)));
    env.addFunction("type", std::make_shared<Value>(std::make_shared<BuiltInFunction>(getType)));
    env.addFunction("range", std::make_shared<Value>(std::make_shared<BuiltInFunction>(range)));
    env.addFunction("map", std::make_shared<Value>(std::make_shared<BuiltInFunction>(map)));
    env.addFunction("all", std::make_shared<Value>(std::make_shared<BuiltInFunction>(all)));
    env.addFunction("any", std::make_shared<Value>(std::make_shared<BuiltInFunction>(any)));
    env.addFunction("read", std::make_shared<Value>(std::make_shared<BuiltInFunction>(read)));
    env.addFunction("input", std::make_shared<Value>(std::make_shared<BuiltInFunction>(input)));
    env.addFunction("zip", std::make_shared<Value>(std::make_shared<BuiltInFunction>(zip)));
    env.addFunction("enumerate", std::make_shared<Value>(std::make_shared<BuiltInFunction>(enumerate)));
    env.addFunction("time", std::make_shared<Value>(std::make_shared<BuiltInFunction>(currentTime)));

    env.addMember(ValueType::List, "size", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listSize)));
    env.addMember(ValueType::List, "append", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listAppend)));
    env.addMember(ValueType::List, "pop", std::make_shared<Value>(std::make_shared<BuiltInFunction>(listPop)));

    env.addMember(ValueType::Dictionary, "get", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictGet)));
    env.addMember(ValueType::Dictionary, "items", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictItems)));
    env.addMember(ValueType::Dictionary, "keys", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictKeys)));
    env.addMember(ValueType::Dictionary, "values", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictValues)));
    env.addMember(ValueType::Dictionary, "pop", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictPop)));
    env.addMember(ValueType::Dictionary, "update", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictUpdate)));
    env.addMember(ValueType::Dictionary, "size", std::make_shared<Value>(std::make_shared<BuiltInFunction>(dictSize)));

    env.addMember(ValueType::String, "lower", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringLower)));
    env.addMember(ValueType::String, "upper", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringUpper)));
    env.addMember(ValueType::String, "strip", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringStrip)));
    env.addMember(ValueType::String, "split", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringSplit)));
    env.addMember(ValueType::String, "isDigit", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringIsDigit)));
    env.addMember(ValueType::String, "length", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringLength)));
    env.addMember(ValueType::String, "replace", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringReplace)));
    env.addMember(ValueType::String, "join", std::make_shared<Value>(std::make_shared<BuiltInFunction>(stringJoin)));


    for (auto statement : statements) {
        try {
            auto result = statement->evaluate(env);
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
        catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
    }

    popExecutionContext();

    return 0;
}