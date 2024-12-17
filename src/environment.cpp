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
    for (const auto scope : scopes) {
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

void Environment::removeScope() {
    scopes.pop_back();
}

void Environment::display() const {
    for (auto scope : scopes) {
        scope.display();
    }
}