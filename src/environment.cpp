#include "environment.h"
#include "library.h"
#include <format>
#include <iostream>
#include "errorDefs.h"
#include "values.h"


Scope::Scope() {}

void Scope::set(std::string name, std::shared_ptr<Value> value) {
    variables[name] = value;
}

bool Scope::contains(std::string name) const {
    return static_cast<bool>(variables.count(name));
}

const std::vector<std::pair<std::string, std::shared_ptr<Value>>> Scope::getPairs() const {
    std::vector<std::pair<std::string, std::shared_ptr<Value>>> pairs;
    for (const auto& pair : variables) {
        pairs.push_back(pair);
    }
    return pairs;
}

std::shared_ptr<Value> Scope::get(std::string name) const {
    if (contains(name)) {
        return variables.at(name);
    } else {
        handleError(std::format("Bad environment access with key '{}'", name), 0, 0, "Runtime Error");
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


Environment::Environment() {
    class_env = false;
}

Environment::Environment(Scope class_scope, std::vector<Scope> outer_scopes)
        : class_scope{class_scope}, scopes{outer_scopes} {}

void Environment::set(std::string name, std::shared_ptr<Value> value) {
    if (scopes.empty()) {
        handleError("Attempted to access empty environment", 0, 0, "Runtime Error");
    } else {
        if (isGlobal(name)) {
            scopes.front().set(name, value);
            return;
        }
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
        handleError("Attempted to access empty environment", 0, 0, "Runtime Error");
    }

    if (isGlobal(name)) {
        if (scopes.front().contains(name)) {
            return scopes.front().get(name);
        } else {
            handleError(std::format("Unrecognized variable {}", name), 0, 0, "Runtime Error");
        }
    }
    
    for (int i = scopes.size() - 1; i >= 0; i--) {
        const auto& scope = scopes.at(i);
        if (scope.contains(name)) {
            return scope.get(name);
        }
    }

    handleError(std::format("Unrecognized variable {}", name), 0, 0, "Runtime Error");
}


bool Environment::contains(std::string name) const {
    if (scopes.empty()) {
        handleError("Attempted to access empty environment", 0, 0, "Runtime Error");
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
    resetGlobals();
}

void Environment::addScope(Scope& scope) {
    scopes.push_back(scope);
    resetGlobals();
}

Scope Environment::getScope() {
    return scopes.back();
}

void Environment::removeScope() {
    scopes.pop_back();
    removeGlobalScope();
}

std::vector<Scope> Environment::copyScopes() const {
    return scopes;
}

void Environment::addFunction(const std::string& name, std::shared_ptr<Value> func) {
    built_in_functions[name] = func;
}

std::shared_ptr<Value> Environment::getFunction(const std::string& name) const {
    auto func = built_in_functions.find(name);
    if (func != built_in_functions.end()) {
        return func->second;
    }

    runtimeError("Unrecognized built-in function: " + name);
}

bool Environment::hasFunction(const std::string& name) const {
    auto func = built_in_functions.find(name);
    return func != built_in_functions.end();
}

void Environment::addMember(ValueType type, const std::string& name, std::shared_ptr<Value> func) {
    member_functions[type][name] = func;
}

std::shared_ptr<Value> Environment::getMember(ValueType type, const std::string& name) const {
    auto members = member_functions.find(type);
    if (members != member_functions.end()) {
        auto func = members->second.find(name);
        if (func != members->second.end()) {
            return func->second;
        }
    }

    runtimeError("Unrecognized member function: " + name);
}

bool Environment::hasMember(ValueType type, const std::string& name) const {
    auto members = member_functions.find(type);
    if (members != member_functions.end()) {
        auto func = members->second.find(name);
        return func != members->second.end();
    }
    return false;
}

void Environment::addGlobal(std::string name) {
    scoped_globals[scopeDepth()].push_back(name);
}

void Environment::resetGlobals() {
    scoped_globals[scopeDepth()].clear();
}

void Environment::removeGlobalScope() {
    scoped_globals.erase(scopeDepth() + 1);
}

bool Environment::isGlobal(std::string name) const {
    for (int i = scopeDepth(); i > 0; i--) {
        if (scoped_globals.find(i) != scoped_globals.end()) {
            for (const auto& str : scoped_globals.at(i)) {
                if (str == name) {
                    return true;
                }
            }
        }
    }
    return false;
}

void Environment::setGlobalValue(std::string name, std::shared_ptr<Value> value) {
    scopes.at(0).set(name, value);
}

void Environment::display() const {
    for (auto scope : scopes) {
        scope.display();
    }
    std::cout << std::endl;
}