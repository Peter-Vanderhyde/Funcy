#pragma once
#include <variant>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

using Value = std::variant<int, double, bool, std::string>;

class Scope {
public:
    Scope();
    void set(std::string name, std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(std::string name) const;
    bool contains(std::string name) const;
    void display() const;
private:
    std::unordered_map<std::string, std::shared_ptr<Value>> variables;
};

class Environment {
public:
    Environment();
    void set(std::string name, std::shared_ptr<Value> value);
    bool contains(std::string name) const;
    std::shared_ptr<Value> get(std::string name) const;

    int scopeDepth() const;
    void addScope();
    void removeScope();

    void display() const;
private:
    std::vector<Scope> scopes;
};