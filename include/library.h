#pragma once
#include <variant>
#include <sstream>
#include "pch.h"
#include "parser.h"

struct Value;

using BuiltInFunctionReturn = std::optional<std::shared_ptr<Value>>;

std::string readSourceCodeFromFile(const std::string& filename);

BuiltInFunctionReturn print(const List& args, Environment& env);
BuiltInFunctionReturn intConverter(const List& args, Environment& env);
BuiltInFunctionReturn floatConverter(const List& args, Environment& env);
BuiltInFunctionReturn boolConverter(const List& args, Environment& env);
BuiltInFunctionReturn stringConverter(const List& args, Environment& env);
BuiltInFunctionReturn listConverter(const List& args, Environment& env);
BuiltInFunctionReturn input(const List& args, Environment& env);
BuiltInFunctionReturn getType(const List& args, Environment& env);
BuiltInFunctionReturn range(const List& args, Environment& env);
BuiltInFunctionReturn map(const List& args, Environment& env);
BuiltInFunctionReturn all(const List& args, Environment& env);
BuiltInFunctionReturn any(const List& args, Environment& env);


BuiltInFunctionReturn listSize(const List& args, Environment& env);
BuiltInFunctionReturn listAppend(const List& args, Environment& env);
BuiltInFunctionReturn listPop(const List& args, Environment& env);

BuiltInFunctionReturn dictGet(const List& args, Environment& env);
BuiltInFunctionReturn dictItems(const List& args, Environment& env);
BuiltInFunctionReturn dictKeys(const List& args, Environment& env);
BuiltInFunctionReturn dictValues(const List& args, Environment& env);
BuiltInFunctionReturn dictPop(const List& args, Environment& env);
BuiltInFunctionReturn dictUpdate(const List& args, Environment& env);
BuiltInFunctionReturn dictSize(const List& args, Environment& env);

BuiltInFunctionReturn stringLower(const List& args, Environment& env);
BuiltInFunctionReturn stringUpper(const List& args, Environment& env);
BuiltInFunctionReturn stringStrip(const List& args, Environment& env);
BuiltInFunctionReturn stringSplit(const List& args, Environment& env);
BuiltInFunctionReturn stringIsDigit(const List& args, Environment& env);
BuiltInFunctionReturn stringLength(const List& args, Environment& env);