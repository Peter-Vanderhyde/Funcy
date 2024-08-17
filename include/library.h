#include "parser.h"

using BuiltInFunctionReturn = std::optional<std::shared_ptr<Value>>;

BuiltInFunctionReturn print(const std::vector<std::shared_ptr<Value>>& args);