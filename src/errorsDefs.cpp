#include "errorDefs.h"
#include <string>
#include <format>


void handleError(std::string message, int line, int column, std::string prefix) {
    Style s;
    if (line == 0 && column == 0) {
        throw std::runtime_error(std::format("{}{}:{} {}{}",
            s.red, prefix, s.orange, message, s.reset));
    } else {
        throw std::runtime_error(std::format("{}{}:{} {}{} at {}{}line {} column {}{}",
            s.red, prefix, s.orange, message, s.reset, s.underline, s.purple, line, column, s.reset));
    }
}

void runtimeError(std::string message, int line, int column) {
    handleError(message, line, column, "Runtime Error");
}

void runtimeError(std::string message) {
    handleError(message, 0, 0, "Runtime Error");
}