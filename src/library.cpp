#include <library.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <format>
#include <iomanip>
#include <vector>
#include <memory>

std::string readSourceCodeFromFile(const std::string& filename) {
    if (filename.size() < 3 || filename.substr(filename.size() - 3) != ".fy") {
        throw std::runtime_error("Error: File must have a .fy extension.");
        return "";
    }

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

void printValue(const std::shared_ptr<Value> value, Environment& env) {
    Style style{};
    if (std::holds_alternative<int>(*value)) {
        auto int_value = std::get<int>(*value);
        std::cout << style.light_blue << int_value << style.reset;
    } else if (std::holds_alternative<double>(*value)) {
        auto double_value = std::get<double>(*value);
        if (double_value == static_cast<int>(double_value)) {
            std::cout << style.light_blue << double_value << ".0" << style.reset;
        } else {
            std::cout << style.light_blue << double_value << style.reset;
        }
    } else if (std::holds_alternative<bool>(*value)) {
        auto bool_value = std::get<bool>(*value);
        std::cout << style.purple << std::boolalpha << bool_value << style.reset;
    } else if (std::holds_alternative<std::string>(*value)) {
        auto string_value = std::get<std::string>(*value);
        std::cout << style.green << "'" << string_value << "'" << style.reset;
    }
}

std::vector<std::variant<int, double>> transformNums(std::shared_ptr<std::variant<int, double, bool, std::string>> first,
                                                    std::shared_ptr<std::variant<int, double, bool, std::string>> second) {
    // Takes any combination of bool/int/double and turns them into the same type for adding, dividing, etc
    if (std::holds_alternative<std::string>(*first) || std::holds_alternative<std::string>(*second)) {
        handleError("Attempted to transformNum with a string", 0, 0, "Runtime Error");
    }
    std::variant<int, double> first_num, second_num;
    if (std::holds_alternative<bool>(*first)) {
        if (std::get<bool>(*first)) {
            first_num = 1;
        } else {
            first_num = 0;
        }
    } else if (std::holds_alternative<int>(*first)) {
        first_num = std::get<int>(*first);
    } else {
        first_num = std::get<double>(*first);
    }
    if (std::holds_alternative<bool>(*second)) {
        if (std::get<bool>(*second)) {
            second_num = 1;
        } else {
            second_num = 0;
        }
    } else if (std::holds_alternative<int>(*second)) {
        second_num = std::get<int>(*second);
    } else {
        second_num = std::get<double>(*second);
    }

    if (std::holds_alternative<double>(first_num) || std::holds_alternative<double>(second_num)) {
        if (first_num.index() != second_num.index() && std::holds_alternative<int>(first_num)) {
            return {static_cast<double>(std::get<int>(first_num)), std::get<double>(second_num)};
        } else if (first_num.index() != second_num.index()) {
            return {std::get<double>(first_num), static_cast<double>(std::get<int>(second_num))};
        } else {
            return {std::get<double>(first_num), std::get<double>(second_num)};
        }
    } else {
        return {std::get<int>(first_num), std::get<int>(second_num)};
    }
}