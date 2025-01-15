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

void printValue(const std::shared_ptr<Value> value, bool error = false);

std::vector<std::variant<int, double>> transformNums(std::shared_ptr<Value> first,
                                                    std::shared_ptr<Value> second);

Environment buildStartingEnvironment();


BuiltInFunctionReturn absoluteValue(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn all(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn any(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn appendFile(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn boolConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn callable(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn currentTime(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn divMod(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn enumerate(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn floatConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn getType(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn globals(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn input(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn intConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn length(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn locals(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn map(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn max(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn min(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn print(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn randChoice(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn randInt(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn range(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn readFile(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn reversed(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn roundVal(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringConverter(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn sum(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn writeFile(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn zip(const std::vector<std::shared_ptr<Value>>& args, Environment& env);

BuiltInFunctionReturn floatIsInt(const std::vector<std::shared_ptr<Value>>& args, Environment& env);

BuiltInFunctionReturn listAppend(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listClear(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listCopy(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listIndex(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listInsert(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listPop(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listRemove(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn listSize(const std::vector<std::shared_ptr<Value>>& args, Environment& env);

BuiltInFunctionReturn dictClear(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictCopy(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictGet(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictItems(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictKeys(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictPop(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictSetDefault(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictSize(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictUpdate(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn dictValues(const std::vector<std::shared_ptr<Value>>& args, Environment& env);

BuiltInFunctionReturn stringCapitalize(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringEndsWith(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringFind(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringIsAlpha(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringIsAlphaNum(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringIsDigit(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringIsSpace(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringIsWhitespace(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringJoin(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringLength(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringLower(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringReplace(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringSplit(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringStrip(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringToJson(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn stringUpper(const std::vector<std::shared_ptr<Value>>& args, Environment& env);

BuiltInFunctionReturn instanceDel(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn instanceGet(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn instanceHas(const std::vector<std::shared_ptr<Value>>& args, Environment& env);
BuiltInFunctionReturn instanceSet(const std::vector<std::shared_ptr<Value>>& args, Environment& env);