#include "environment.h"
#include "library.h"
#include <format>
#include <iostream>
#include "errorDefs.h"
#include "values.h"

bool DETECT_RECURSION;

Scope::Scope() {}

void Scope::set(std::string name, std::shared_ptr<Value> value) {
    variables[name] = value;
}

bool Scope::contains(std::string name) const {
    return static_cast<bool>(variables.count(name));
}

void Scope::remove(std::string name) {
    variables.erase(name);
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
        throwError(ErrorType::Runtime, std::format("Bad environment access with key '{}'", name));
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
    : scopes(other.scopes), class_env(other.class_env),
      loop_depth(other.loop_depth), built_in_functions(other.built_in_functions),
      member_functions(other.member_functions), scoped_globals(other.scoped_globals),
      class_depth(other.class_depth), class_attrs(other.class_attrs), is_top_scope(other.is_top_scope) {}

void Environment::setClassEnv() {
    class_env = true;
}

bool Environment::isClassEnv() const {
    return class_env;
}

void Environment::set(std::string name, std::shared_ptr<Value> value, bool is_member_var) {
    if (scopes.empty()) {
        throwError(ErrorType::Runtime, "Attempted to access empty environment");
    } else {
        if (is_member_var && class_depth == 0 && class_env == false) {
            throwError(ErrorType::Runtime, "Unable to set class attribute '" + name + "' outside of class");
        } else if (is_member_var) {
            class_attrs.set(name, value);
            return;
        }
        if (isGlobal(name)) {
            setGlobalValue(name, value);
            return;
        }
        for (int i = scopes.size() - 1; i > -1; i--) {
            if (scopes.at(i).contains(name)) {
                scopes.at(i).set(name, value);
                return;
            }
        }

        scopes.back().set(name, value);
    }
}

std::shared_ptr<Value> Environment::get(std::string name, bool is_member_var) const {
    if (scopes.empty()) {
        throwError(ErrorType::Runtime, "Attempted to access empty environment");
    }
    if (is_member_var && class_depth == 0 && class_env == false) {
        throwError(ErrorType::Runtime, "Unable to get class attribute '" + name + "' outside of class");
    } else if (is_member_var) {
        return class_attrs.get(name);
    }
    if (isGlobal(name)) {
        if (scopes.front().contains(name)) {
            return scopes.front().get(name);
        } else {
            throwError(ErrorType::Runtime, "Unrecognized variable " + name);
        }
    }
    
    for (int i = scopes.size() - 1; i >= 0; i--) {
        const auto& scope = scopes.at(i);
        if (scope.contains(name)) {
            return scope.get(name);
        }
    }

    throwError(ErrorType::Runtime, "Unrecognized variable " + name);
}


bool Environment::contains(std::string name, bool is_member_var) const {
    if (scopes.empty()) {
        throwError(ErrorType::Runtime, "Attempted to access emtpy environment");
    }
    if (class_env == true || class_depth != 0) {
        if (is_member_var) {
            return class_attrs.contains(name);
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

void Environment::addClassScope() {
    class_depth += 1;
    scopes.push_back(Scope());
    resetGlobals();
}

Scope Environment::getScope() {
    return scopes.back();
}

int Environment::classDepth() {
    return class_depth;
}

void Environment::removeScope() {
    scopes.pop_back();
    removeGlobalScope();
}

void Environment::removeClassScope(Scope& previous_attrs) {
    class_depth -= 1;
    class_attrs = previous_attrs;
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

    throwError(ErrorType::Runtime, "Unrecognized built-in function '" + name + "'");
}

bool Environment::hasFunction(const std::string& name) const {
    auto func = built_in_functions.find(name);
    return func != built_in_functions.end();
}

void Environment::addMember(ValueType type, const std::string& name, std::shared_ptr<Value> func) {
    member_functions[type][name] = func;
}

void Environment::addMember(const std::string& name, std::shared_ptr<Value> value) {
    class_attrs.set(name, value);
}

std::shared_ptr<Value> Environment::getMember(ValueType type, const std::string& name) const {
    auto members = member_functions.find(type);
    if (members != member_functions.end()) {
        auto func = members->second.find(name);
        if (func != members->second.end()) {
            return func->second;
        }
    }

    throwError(ErrorType::Runtime, "Unrecognized member function '" + name + "'");
}

std::shared_ptr<Value> Environment::getMember(const std::string& name) const {
    return class_attrs.get(name);
}

bool Environment::hasMember(ValueType type, const std::string& name) const {
    auto members = member_functions.find(type);
    if (members != member_functions.end()) {
        auto func = members->second.find(name);
        return func != members->second.end();
    }
    return false;
}

bool Environment::hasMember(const std::string& name) const {
    return class_attrs.contains(name);
}

void Environment::delMember(const std::string& name) {
    class_attrs.remove(name);
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

Scope& Environment::getClassAttrs() {
    return class_attrs;
}

void Environment::setClassAttrs(Scope& scope) {
    class_attrs = scope;
}

void Environment::setThis(std::shared_ptr<Value> inst_ref) {
    this_ref = inst_ref;
}

std::shared_ptr<Value> Environment::getThis() {
    if (!this_ref) {
        throwError(ErrorType::Runtime, "'this' may only be used inside a member function");
        return std::make_shared<Value>();
    }
    return this_ref;
}

void Environment::display(bool show_attrs) const {
    if (class_env || show_attrs) {
        std::cout << "ATTRS\n";
        class_attrs.display();
    }
    int i = 1;
    for (auto scope : scopes) {
        std::cout << "Scope:" << i << std::endl;
        scope.display();
        i += 1;
    }
    std::cout << std::endl;
}