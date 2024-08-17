#include "library.h"


BuiltInFunctionReturn print(const std::vector<std::shared_ptr<Value>>& args) {
    for (const auto& arg : args) {
        printValue(arg);
    }
    std::cout << std::endl;
    return std::nullopt;
}