#include "context.h"

// Thread-local storage for execution context
thread_local std::stack<std::string> execution_context;

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
