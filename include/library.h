#pragma once
#include <string>
#include <variant>
#include <vector>
#include <memory>

struct Style {
    std::string red = "\033[31m";
    std::string green = "\033[32m";
    std::string orange = "\033[38;5;214m";
    std::string blue = "\033[34m";
    std::string light_blue = "\033[38;5;81m";
    std::string purple = "\033[38;5;129m";
    std::string reset = "\033[0m";
    std::string underline = "\033[4m";
};

std::string readSourceCodeFromFile(const std::string& filename);

[[noreturn]] void handleError(std::string message, int line, int column, std::string prefix);

void printValue(const std::variant<int, double, bool, std::string>& value);

std::vector<std::variant<int, double>> transformNums(std::shared_ptr<std::variant<int, double, bool, std::string>> first,
                                                    std::shared_ptr<std::variant<int, double, bool, std::string>> second);