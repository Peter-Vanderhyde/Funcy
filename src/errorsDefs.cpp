#include "errorDefs.h"
#include <string>
#include <format>
#include "context.h"
#include <fstream>


std::string getLine(const std::string& filename, int line) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throwError(ErrorType::Runtime, "Could not open file: " + filename);
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
        return "<Line number out of range in file: " + filename + ">";
    }
    return "";
}

std::string buildError(ErrorType error_type, std::string message, int line, int column, std::string filename) {
    if (filename == "") {
        filename = currentExecutionContext();
    }

    std::map<ErrorType, std::string> error_type_str{
        {ErrorType::Runtime, "Runtime Error"},
        {ErrorType::Thrown, "Thrown Exception"},
        {ErrorType::StackOverflow, "Stack Overflow Warning"},
        {ErrorType::ZeroDivision, "Zero Division Error"},
        {ErrorType::Syntax, "Syntax Error"},
        {ErrorType::ArityMismatch, "Arity Mismatch Error"}
    };

    Style style{};
    std::string error;
    if (line == 0 && column == 0) {
        error = std::format("{}{}:{} File {}{}{}:\n\n{}{}",
                            style.red, error_type_str[error_type], style.reset, style.green, filename, style.reset,
                            style.orange, message);
        if (!message.ends_with(style.reset)) {
            error = std::format("{}.{}", error, style.reset);
        }
    } else {
        error = std::format("{}{}:{} File {}{}{} at {}{}line {} column {}{}:\n",
                                        style.red, error_type_str[error_type], style.reset, style.green, filename, style.reset,
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

    return error;
}

void throwError(ErrorType error_type, std::string message, int line, int column, std::string filename) {
    if (filename == "") {
        filename = currentExecutionContext();
    }

    std::string error_message = buildError(error_type, message, line, column, filename);
    throw ErrorException(error_type, error_message);
}