#pragma once
#include <string>
#include <variant>

using TokenValue = std::variant<int, double, std::string>;

std::string readSourceCodeFromFile(const std::string& filename);