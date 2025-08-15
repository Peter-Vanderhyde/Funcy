#pragma once
#include <string>
#include <stack>
#include <thread>
#include <memory>
#include <map>

class Value;

// Thread-local storage for execution context
extern thread_local std::stack<std::string> parsing_context; // File where the currently parsing code is from
extern thread_local std::stack<std::string> execution_context; // File where the currently executing code is from
extern thread_local std::stack<std::pair<std::string, std::string>> function_context; // Function that the currently executing code is inside

extern thread_local int debug_tabs;

void pushParsingContext(std::string filename);

void pushExecutionContext(std::string filename);

void pushFunctionContext(std::string func_name, std::string filename);

void popParsingContext();

void popExecutionContext();

void popFunctionContext();

std::string currentParsingContext();

std::string currentExecutionContext();

std::pair<std::string, std::string> currentFunctionContext();