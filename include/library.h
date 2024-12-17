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