#pragma once
#include <string>
#include <stack>
#include <thread>

// Thread-local storage for execution context
extern thread_local std::stack<std::string> execution_context;

void pushExecutionContext(const std::string& filename);

void popExecutionContext();

std::string currentExecutionContext();