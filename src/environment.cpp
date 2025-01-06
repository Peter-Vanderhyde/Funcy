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

Environment::Environment(const Environment& other)
    : scopes(other.scopes), class_scopes(other.class_scopes), class_env(other.class_env),
      loop_depth(other.loop_depth), built_in_functions(other.built_in_functions),
      member_functions(other.member_functions), scoped_globals(other.scoped_globals),
      class_depth(other.class_depth), class_attrs(other.class_attrs) {}

void Environment::setClassEnv(Scope& class_scope) {
    class_env = true;
    class_scopes.push_back(class_scope);
}

bool Environment::isClassEnv() const {
    return class_env;
}

void Environment::set(std::string name, std::shared_ptr<Value> value, bool is_member_var) {
    if (scopes.empty()) {
        handleError("Attempted to access empty environment", 0, 0, "Runtime Error");
    } else {
        if (is_member_var && class_depth == 0 && class_env == false) {
            runtimeError("Unable to set class attribute '" + name + "' outside of class");
        } else if (is_member_var) {
            class_attrs.set(name, value);
            return;
        }
        if (isGlobal(name)) {
            scopes.front().set(name, value);
            return;
        }
        if (class_env == true) { // change member values before exterior environment
            for (int i = class_scopes.size() - 1; i > -1; i--) {
                if (class_scopes.at(i).contains(name)) {
                    class_scopes.at(i).set(name, value);
                    return;
                }
            }
        }
        for (int i = scopes.size() - 1; i > -1; i--) {
            if (scopes.at(i).contains(name)) {
                scopes.at(i).set(name, value);
                return;
            }
        }
    }

    if (class_env == true) {
        class_scopes.back().set(name, value);
        return;
    }
    scopes.back().set(name, value);
}

std::shared_ptr<Value> Environment::get(std::string name, bool is_member_var) const {
    if (scopes.empty()) {
        handleError("Attempted to access empty environment", 0, 0, "Runtime Error");
    }
    if (is_member_var && class_depth == 0 && class_env == false) {
        runtimeError("Unable to get class attribute '" + name + "' outside of class");
    } else if (is_member_var) {
        return class_attrs.get(name);
    }
    if (isGlobal(name)) {
        if (scopes.front().contains(name)) {
            return scopes.front().get(name);
        } else {
            runtimeError("Unrecognized variable " + name);
        }
    }

    if (class_env == true) {
        for (int i = class_scopes.size() - 1; i >= 0; i--) {
            const auto& scope = class_scopes.at(i);
            if (scope.contains(name)) {
                return scope.get(name);
            }
        }
    }
    
    for (int i = scopes.size() - 1; i >= 0; i--) {
        const auto& scope = scopes.at(i);
        if (scope.contains(name)) {
            return scope.get(name);
        }
    }

    runtimeError(std::format("Unrecognized variable {}", name));
}


bool Environment::contains(std::string name, bool is_member_var) const {
    if (scopes.empty()) {
        handleError("Attempted to access empty environment", 0, 0, "Runtime Error");
    }
    if (class_env == true || class_depth != 0) {
        if (is_member_var) {
            return class_attrs.contains(name);
        }
        for (const auto scope : class_scopes) {
            if (scope.contains(name)) {
                return true;
            }
        }
    }
    for (const auto scope : scopes) {
        if (scope.contains(name)) {
            return true;
        }
    }

    return false;
}

int Environment::scopeDepth() const {
    if (class_env == true) {
        return class_scopes.size();
    }
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
    if (class_env == true) {
        class_scopes.push_back(Scope());
        resetGlobals();
        return;
    }
    scopes.push_back(Scope());
    resetGlobals();
}

void Environment::addScope(Scope& scope) {
    if (class_env == true) {
        class_scopes.push_back(scope);
        resetGlobals();
        return;
    }
    scopes.push_back(scope);
    resetGlobals();
}

void Environment::addClassScope() {
    class_depth += 1;
    class_attrs = Scope();
    if (class_env == true) {
        class_scopes.push_back(Scope());
        resetGlobals();
        return;
    }
    scopes.push_back(Scope());
    resetGlobals();
}

Scope Environment::getScope() {
    if (class_env == true) {
        return class_scopes.back();
    }
    return scopes.back();
}

void Environment::removeScope() {
    if (class_env == true) {
        class_scopes.pop_back();
        removeGlobalScope();
        return;
    }
    scopes.pop_back();
    removeGlobalScope();
}

void Environment::removeClassScope() {
    class_depth -= 1;
    class_attrs = Scope();
    if (class_env == true) {
        class_scopes.pop_back();
        removeGlobalScope();
        return;
    }
    scopes.pop_back();
    removeGlobalScope();
}

std::vector<Scope> Environment::copyScopes() const {
    if (class_env == true) {
        std::vector<Scope> new_scopes = scopes;
        for (auto scope : class_scopes) {
            new_scopes.push_back(scope);
        }
        return new_scopes;
    }
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
    if (class_env == true && type == ValueType::Instance) {
        return class_attrs.get(name);
        runtimeError("Class has no attribute: " + name);
    }
    else {
        auto members = member_functions.find(type);
        if (members != member_functions.end()) {
            auto func = members->second.find(name);
            if (func != members->second.end()) {
                return func->second;
            }
        }
    }

    runtimeError("Unrecognized member function: " + name);
}

bool Environment::hasMember(ValueType type, const std::string& name) const {
    if (class_env == true && type == ValueType::Instance) {
        return class_attrs.contains(name);
    }
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

Scope& Environment::getClassScope() {
    return class_scopes.front();
}

void Environment::setClassScope(const Scope& class_scope) {
    class_scopes[0] = class_scope;
}

Scope& Environment::getClassAttrs() {
    return class_attrs;
}

void Environment::setClassAttrs(Scope& scope) {
    class_attrs = scope;
}

void Environment::display(bool show_attrs) const {
    int i = 1;
    for (auto scope : scopes) {
        std::cout << "Scope:" << i << std::endl;
        scope.display();
        i += 1;
    }
    if (class_env || show_attrs) {
        std::cout << "CLASS" << std::endl;
        std::cout << "ATTRS\n";
        class_attrs.display();
        i = 1;
        for (auto scope : class_scopes) {
            std::cout << "Class Scope:" << i << std::endl;
            scope.display();
            i += 1;
        }
    }
    std::cout << std::endl;
}