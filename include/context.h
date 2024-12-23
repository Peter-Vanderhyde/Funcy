#pragma once
#include <thread>
#include <stack>
#include <string>

// Thread-local storage for execution context
thread_local std::stack<std::string> executionContext;

void pushExecutionContext(const std::string& filename);

void popExecutionContext();

std::string currentExecutionContext();