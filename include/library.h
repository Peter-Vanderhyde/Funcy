#pragma once
#include <variant>
#include <sstream>
#include "pch.h"
#include "parser.h"

struct Value;

using BuiltInFunctionReturn = std::optional<std::shared_ptr<Value>>;

std::string readSourceCodeFromFile(const std::string& filename);

BuiltInFunctionReturn print(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn intConverter(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn floatConverter(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn boolConverter(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn stringConverter(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn listConverter(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn input(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn getType(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn range(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn map(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
// BuiltInFunctionReturn all(const std::vector<std::shared_ptr<Value>>& args);
// BuiltInFunctionReturn any(const std::vector<std::shared_ptr<Value>>& args);


BuiltInFunctionReturn listSize(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn listAppend(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn listPop(const std::vector<std::shared_ptr<Value>>& args);

BuiltInFunctionReturn stringLower(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn stringUpper(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn stringStrip(const std::vector<std::shared_ptr<Value>>& args);
BuiltInFunctionReturn stringIsDigit(const std::vector<std::shared_ptr<Value>>& args);