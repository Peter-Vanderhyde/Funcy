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

void printValue(const std::shared_ptr<Value> value, bool error = false, bool drawing = false);

std::vector<std::variant<int, double>> transformNums(std::shared_ptr<Value> first,
                                                    std::shared_ptr<Value> second);

std::shared_ptr<Scope> buildStartingEnvironment();


BuiltInFunctionReturn absoluteValue(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn all(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn any(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn appendFile(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn boolConverter(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn callable(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn currentTime(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictConverter(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn divMod(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn draw(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn enumerate(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn floatConverter(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn getType(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn globals(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn input(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn intConverter(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn length(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn listConverter(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn locals(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn map(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn max(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn min(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn print(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn randChoice(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn randInt(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn randShuffle(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn range(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn readFile(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn reversed(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn roundVal(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn showCursor(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn sleep(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringConverter(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn sum(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn writeFile(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn zip(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn getKey(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);

BuiltInFunctionReturn floatIsInt(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);

BuiltInFunctionReturn listAppend(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn listClear(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn listCopy(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn listIndex(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn listInsert(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn listPop(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn listRemove(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn listReverse(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn listSize(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);

BuiltInFunctionReturn dictClear(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictCopy(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictGet(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictItems(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictKeys(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictPop(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictSetDefault(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictSize(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictUpdate(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn dictValues(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);

BuiltInFunctionReturn stringCapitalize(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringEndsWith(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringFind(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringIsAlpha(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringIsAlphaNum(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringIsDigit(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringIsSpace(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringIsWhitespace(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringJoin(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringLength(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringLower(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringReplace(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringSplit(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringStrip(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringToJson(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn stringUpper(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);

BuiltInFunctionReturn instanceDel(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn instanceGet(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn instanceHas(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);
BuiltInFunctionReturn instanceSet(const std::vector<std::shared_ptr<Value>>& args, Scope& scope);