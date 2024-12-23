#include <thread>
#include <stack>
#include <string>
#include "context.h"

void pushExecutionContext(const std::string& filename) {
    executionContext.push(filename);
}

void popExecutionContext() {
    if (!executionContext.empty()) {
        executionContext.pop();
    }
}

std::string currentExecutionContext() {
    return executionContext.empty() ? "<unknown file>" : executionContext.top();
}
