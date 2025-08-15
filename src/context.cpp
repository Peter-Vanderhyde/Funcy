#include "context.h"
#include "values.h"

// Thread-local storage for execution context
thread_local std::stack<std::string> parsing_context;
thread_local std::stack<std::string> execution_context;
thread_local std::stack<std::pair<std::string, std::string>> function_context;

thread_local int debug_tabs = 0;

void pushParsingContext(std::string filename) {
    parsing_context.push(filename);
}

void pushExecutionContext(std::string filename) {
    execution_context.push(filename);
}

void pushFunctionContext(std::string func_name, std::string filename) {
    function_context.push(std::make_pair(func_name, filename));
    pushExecutionContext(filename);
}

void popParsingContext() {
    if (!parsing_context.empty()) {
        parsing_context.pop();
    }
}

void popExecutionContext() {
    if (!execution_context.empty()) {
        execution_context.pop();
    }
}

void popFunctionContext() {
    if (!function_context.empty()) {
        function_context.pop();
    }
    popExecutionContext();
}

std::string currentParsingContext() {
    return parsing_context.empty() ? "<unknown file>" : parsing_context.top();
}

std::string currentExecutionContext() {
    return execution_context.empty() ? "<unknown file>" : execution_context.top();
}

std::pair<std::string, std::string> currentFunctionContext() {
    return function_context.empty() ? std::make_pair("", "") : function_context.top();
}