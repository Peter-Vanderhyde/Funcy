#include <library.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <format>
#include <iomanip>
#include <vector>
#include <memory>
#include "errorDefs.h"

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

void printValue(const std::shared_ptr<Value> value) {
    Style style{};
    switch(value->getType()) {
        case ValueType::Integer: {
            int int_value = value->get<int>();
            std::cout << style.light_blue << int_value << style.reset;
            return;
        }
        case ValueType::Float: {
            double float_value = value->get<double>();
            if (float_value == static_cast<int>(float_value)) {
                std::cout << style.light_blue << float_value << ".0" << style.reset;
            } else {
                std::cout << style.light_blue << float_value << style.reset;
            }
            return;
        }
        case ValueType::Boolean: {
            bool bool_value = value->get<bool>();
            std::cout << style.purple << std::boolalpha << bool_value << style.reset;
            return;
        }
        case ValueType::String: {
            std::string string_value = value->get<std::string>();
            std::cout << style.green << "'" << string_value << "'" << style.reset;
            return;
        }
        case ValueType::List: {
            std::shared_ptr<List> list_value = value->get<std::shared_ptr<List>>();
            std::cout << "[";
            bool first = true;
            for (int i = 0; i < list_value->size(); i++) {
                auto item = list_value->at(i);
                if (!first) {
                    std::cout << ", ";
                } else {
                    first = false;
                }
                printValue(item);
            }
            std::cout << "]";
            return;
        }
        case ValueType::None: {
            std::cout << style.blue << "Null" << style.reset;
            return;
        }
        default:
            return;
    }
}

std::vector<std::variant<int, double>> transformNums(std::shared_ptr<Value> first, std::shared_ptr<Value> second) {
    // Takes any combination of bool/int/double and turns them into the same type for adding, dividing, etc

    // Check if either Value contains a string
    if (first->getType() == ValueType::String || second->getType() == ValueType::String) {
        handleError("Attempted to transformNum with a string", 0, 0, "Runtime Error");
    }

    std::variant<int, double> first_num, second_num;

    // Transform the first Value into int or double
    switch (first->getType()) {
        case ValueType::Boolean:
            first_num = first->get<bool>() ? 1 : 0;  // Convert bool to int
            break;
        case ValueType::Integer:
            first_num = first->get<int>();
            break;
        case ValueType::Float:
            first_num = first->get<double>();
            break;
        default:
            handleError("Unsupported type in transformNums for the first value", 0, 0, "Runtime Error");
    }

    // Transform the second Value into int or double
    switch (second->getType()) {
        case ValueType::Boolean:
            second_num = second->get<bool>() ? 1 : 0;  // Convert bool to int
            break;
        case ValueType::Integer:
            second_num = second->get<int>();
            break;
        case ValueType::Float:
            second_num = second->get<double>();
            break;
        default:
            handleError("Unsupported type in transformNums for the second value", 0, 0, "Runtime Error");
    }

    // Check if either number is a double, promoting integers to doubles if necessary
    if (std::holds_alternative<double>(first_num) || std::holds_alternative<double>(second_num)) {
        if (std::holds_alternative<int>(first_num)) {
            return {static_cast<double>(std::get<int>(first_num)), std::get<double>(second_num)};
        } else if (std::holds_alternative<int>(second_num)) {
            return {std::get<double>(first_num), static_cast<double>(std::get<int>(second_num))};
        } else {
            return {std::get<double>(first_num), std::get<double>(second_num)};
        }
    } else {
        // Both are integers
        return {std::get<int>(first_num), std::get<int>(second_num)};
    }
}