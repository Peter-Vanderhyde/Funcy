#include "environment.h"
#include "library.h"
#include <format>
#include <iostream>
#include "errorDefs.h"


Scope::Scope() {}

void Scope::set(std::string name, std::shared_ptr<Value> value) {
    variables[name] = value;
}

bool Scope::contains(std::string name) const {
    return static_cast<bool>(variables.count(name));
}

std::shared_ptr<Value> Scope::get(std::string name) const {
    if (contains(name)) {
        return variables.at(name);
    } else {
        handleError(std::format("Bad environment access with key '{}'.", name), 0, 0, "Runtime Error");
    }
}

void Scope::display() const {
    for (const std::pair<const std::string, std::shared_ptr<Value>> pair : variables) {
        const std::string& name = pair.first;
        std::shared_ptr<Value> value = pair.second;

        std::cout << name << " = ";
        printValue(value);
        std::cout << std::endl;
    }
}


Environment::Environment() {}

void Environment::set(std::string name, std::shared_ptr<Value> value) {
    if (scopes.empty()) {
        handleError("Attempted to access empty environment.", 0, 0, "Runtime Error");
    } else {
        for (int i = scopes.size() - 1; i > -1; i--) {
            if (scopes.at(i).contains(name)) {
                scopes.at(i).set(name, value);
                return;
            }
        }
    }

    scopes.back().set(name, value);
}

std::shared_ptr<Value> Environment::get(std::string name) const {
    if (scopes.empty()) {
        handleError("Attempted to access empty environment.", 0, 0, "Runtime Error");
    }
    for (int i = scopes.size() - 1; i >= 0; i--) {
        const auto scope = scopes.at(i);
        if (scope.contains(name)) {
            return scope.get(name);
        }
    }

    handleError(std::format("Unrecognized variable {}.", name), 0, 0, "Runtime Error");
}

bool Environment::contains(std::string name) const {
    if (scopes.empty()) {
        handleError("Attempted to access empty environment.", 0, 0, "Runtime Error");
    }
    for (const auto scope : scopes) {
        if (scope.contains(name)) {
            return true;
        }
    }

    return false;
}

int Environment::scopeDepth() const {
    return scopes.size();
}

void Environment::addLoop() {
    loop_depth += 1;
}

void Environment::removeLoop() {
    loop_depth -= 1;
}

bool Environment::inLoop() const {
    return loop_depth > 0;
}

void Environment::resetLoop() {
    loop_depth = 0;
}

void Environment::addScope() {
    scopes.push_back(Scope());
}

void Environment::addScope(Scope& scope) {
    scopes.push_back(scope);
}

void Environment::removeScope() {
    scopes.pop_back();
}

std::vector<Scope> Environment::copyScopes() const {
    return scopes;
}

void Environment::addFunction(const std::string& name, std::shared_ptr<Value> func) {
    built_in_functions[name] = func;
    built_in_names[func] = name;
}

std::shared_ptr<Value> Environment::getFunction(const std::string& name) const {
    auto func = built_in_functions.find(name);
    if (func != built_in_functions.end()) {
        return func->second;
    }

    throw std::runtime_error("Unrecognized built-in function: " + name);
}

std::string Environment::getName(const std::shared_ptr<Value> func) const {
    auto name = built_in_names.find(func);
    if (name != built_in_names.end()) {
        return name->second;
    }

    throw std::runtime_error("Unrecognized built-in name.");
}

bool Environment::hasFunction(const std::string& name) const {
    auto func = built_in_functions.find(name);
    return func != built_in_functions.end();
}

void Environment::addMember(ValueType type, const std::string& name, std::shared_ptr<Value> func) {
    member_functions[type][name] = func;
    built_in_names[func] = getTypeStr(type) + "." + name;
}

std::shared_ptr<Value> Environment::getMember(ValueType type, const std::string& name) const {
    auto members = member_functions.find(type);
    if (members != member_functions.end()) {
        auto func = members->second.find(name);
        if (func != members->second.end()) {
            return func->second;
        }
    }

    throw std::runtime_error("Unrecognized member function: " + name);
}

bool Environment::hasMember(ValueType type, const std::string& name) const {
    auto members = member_functions.find(type);
    if (members != member_functions.end()) {
        auto func = members->second.find(name);
        return func != members->second.end();
    }
    return false;
}

void Environment::display() const {
    for (auto scope : scopes) {
        scope.display();
    }
    std::cout << std::endl;
}