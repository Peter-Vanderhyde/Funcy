#include "context.h"
#include "values.h"

// Thread-local storage for execution context
thread_local std::stack<std::string> execution_context;

std::map<std::shared_ptr<Value>, std::string> function_contexts;

void pushExecutionContext(const std::string& filename) {
    execution_context.push(filename);
}

void popExecutionContext() {
    if (!execution_context.empty()) {
        execution_context.pop();
    }
}

std::string currentExecutionContext() {
    return execution_context.empty() ? "<unknown file>" : execution_context.top();
}

void setFuncContext(std::shared_ptr<Value> func) {
    function_contexts[func] = currentExecutionContext();
}

std::string getFuncContext(std::shared_ptr<Value> func) {
    if (function_contexts.contains(func)) {
        return function_contexts[func];
    } else {
        throw std::runtime_error("Function definition file not found");
        return "";
    }
}