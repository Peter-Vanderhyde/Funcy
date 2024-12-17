#pragma once
#include <string>


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

[[noreturn]] void handleError(std::string message, int line, int column, std::string prefix);

[[noreturn]] void runtimeError(std::string message, int line, int column);

[[noreturn]] void runtimeError(std::string message);