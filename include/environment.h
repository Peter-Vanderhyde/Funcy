#pragma once
#include <variant>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>
#include <optional>
#include "values.h"

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
    Environment(Environment& env);
    void set(std::string name, std::shared_ptr<Value> value);
    bool contains(std::string name) const;
    std::shared_ptr<Value> get(std::string name) const;

    int scopeDepth() const;
    void addScope();
    void addScope(Scope& scope);
    void removeScope();
    std::vector<Scope> copyScopes() const;
    void addLoop();
    void removeLoop();
    bool inLoop() const;
    void resetLoop();

    void display() const;
private:
    std::vector<Scope> scopes;
    int loop_depth;
};

class BreakException : public std::exception {};
class ContinueException : public std::exception {};
class ReturnException : public std::exception {
public:
    ReturnException(std::optional<std::shared_ptr<Value>> value)
        : value{value} {}

    std::optional<std::shared_ptr<Value>> value;
};