#pragma once
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include "environment.h"
#include "values.h"
#include <optional>


using BuiltInFunctionReturn = std::optional<std::shared_ptr<Value>>;


std::string readSourceCodeFromFile(const std::string& filename);

void printValue(const std::shared_ptr<Value> value);

std::vector<std::variant<int, double>> transformNums(std::shared_ptr<Value> first,
                                                    std::shared_ptr<Value> second);


BuiltInFunctionReturn print(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn intConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn floatConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn boolConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn getType(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn range(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn map(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn all(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn any(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn read(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn input(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn zip(const std::vector<std::shared_ptr<Value>>& args, Environment& env);



BuiltInFunctionReturn listSize(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listAppend(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listPop(const std::vector<std::shared_ptr<Value>>& args, Environment& env);

BuiltInFunctionReturn dictGet(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictItems(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictKeys(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictValues(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictPop(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictUpdate(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictSize(const std::vector<std::shared_ptr<Value>>& args, Environment& env);

BuiltInFunctionReturn stringLower(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringUpper(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringStrip(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringSplit(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringIsDigit(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringLength(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringReplace(const std::vector<std::shared_ptr<Value>>& args, Environment& env);