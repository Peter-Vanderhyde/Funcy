#pragma once
#include <string>
#include <stack>
#include <thread>
#include <memory>
#include <map>

class Value;

// Thread-local storage for execution context
extern thread_local std::stack<std::string> execution_context;

extern std::map<std::shared_ptr<Value>, std::string> function_contexts;

void pushExecutionContext(const std::string& filename);

void popExecutionContext();

std::string currentExecutionContext();

void setFuncContext(std::shared_ptr<Value> func);

std::string getFuncContext(std::shared_ptr<Value> func);