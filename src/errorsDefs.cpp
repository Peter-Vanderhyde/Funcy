#include "errorDefs.h"
#include <string>
#include <format>
#include "context.h"
#include <fstream>


std::string getLine(const std::string& filename, int line) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string currentLine;
    int currentLineNum = 1;

    while (std::getline(file, currentLine)) {
        if (currentLineNum == line) {
            return currentLine;
        }
        currentLineNum++;
    }

    if (currentLineNum < line) {
        throw std::runtime_error("Line number out of range in file: " + filename);
    }
    return "";
}

void handleError(std::string message, int line, int column, std::string prefix) {
    std::string filename = currentExecutionContext();
    Style style{};
    std::string error;
    if (line == 0 && column == 0) {
        error = std::format("{}{}:{} File {}{}{}:\n\n{}{}",
                            style.red, prefix, style.reset, style.green, filename, style.reset,
                            style.orange, message);
        if (!message.ends_with(style.reset)) {
            error = std::format("{}.{}", error, style.reset);
        }
    } else {
        error = std::format("{}{}:{} File {}{}{} at {}{}line {} column {}{}:\n",
                                        style.red, prefix, style.reset, style.green, filename, style.reset,
                                        style.purple, style.underline, line, column, style.reset);
        error += std::format("        {}\n", getLine(filename, line));
        std::string spaces = "        ";
        for (int i = 0; i < column - 1; i++) {
            spaces += " ";
        }
        spaces += style.orange + "^\n";
        error += spaces;
        error += message;
        if (!message.ends_with(style.reset)) {
            error = std::format("{}.{}", error, style.reset);
        }
    }
    throw std::runtime_error(error);
}

void runtimeError(std::string message, int line, int column) {
    handleError(message, line, column, "Runtime Error");
}

void runtimeError(std::string message) {
    handleError(message, 0, 0, "Runtime Error");
}