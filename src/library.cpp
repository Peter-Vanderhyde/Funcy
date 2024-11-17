#include <library.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <format>
#include <iomanip>

std::string readSourceCodeFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf(); // Read the file's contents into the buffer

    return buffer.str(); // Return the contents as a std::string
}

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

void printValue(const std::variant<int, double, bool, std::string>& value) {
    if (std::holds_alternative<int>(value)) {
        std::cout << std::get<int>(value);
    } else if (std::holds_alternative<double>(value)) {
        std::cout << std::fixed << std::setprecision(1) << std::get<double>(value);
    } else if (std::holds_alternative<bool>(value)) {
        std::cout << std::boolalpha << std::get<bool>(value);
    } else if (std::holds_alternative<std::string>(value)) {
        std::cout << std::get<std::string>(value);
    }
}