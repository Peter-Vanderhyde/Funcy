#pragma once
#include <string>
#include <variant>
#include <vector>
#include <memory>
#include "environment.h"
#include "values.h"

std::string readSourceCodeFromFile(const std::string& filename);

void printValue(const std::shared_ptr<Value> value);

std::vector<std::variant<int, double>> transformNums(std::shared_ptr<Value> first,
                                                    std::shared_ptr<Value> second);