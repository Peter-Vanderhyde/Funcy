#pragma once
#include <string>


struct Style {
    std::string red = "\033[31m";
    std::string green = "\033[32m";
    std::string orange = "\033[38;5;214m";
    std::string blue = "\033[34m";
    std::string light_blue = "\033[38;5;81m";
    std::string purple = "\033[38;5;129m";
    std::string grey = "\033[38;5;235m";
    std::string reset = "\033[0m";
    std::string underline = "\033[4m";
};

enum class ErrorType { Runtime, Thrown, StackOverflow, ZeroDivision, Syntax, ArityMismatch };

class ErrorException : public std::exception {
public:
    ErrorException(ErrorType error_type, std::string message)
        : error_type{error_type}, message{message} {}
    
    ErrorType error_type = ErrorType::Runtime;
    std::string message = "";
};

std::string buildError(ErrorType error_type, std::string message, int line, int column, std::string filename="");

[[noreturn]] void throwError(ErrorType error_type, std::string message, int line=0, int column=0, std::string filename="");