#pragma once
#include <variant>
#include <string>
#include <unordered_map>
#include <memory>

using Value = std::variant<int, double, bool, std::string>;

class Environment {
public:
    Environment();
    void add(std::string name, std::shared_ptr<Value> value);
    bool contains(std::string name) const;
    std::shared_ptr<Value> get(std::string name);
private:
    std::unordered_map<std::string, std::shared_ptr<Value>> env;
};