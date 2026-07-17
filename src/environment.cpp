#include "environment.h"
#include "library.h"
#include <format>
#include <iostream>
#include "errorDefs.h"
#include "values.h"

bool DETECT_RECURSION;

Scope::Scope(const std::shared_ptr<Scope> parent)
    : parent{parent} {}

void Scope::set(const std::string name, const std::shared_ptr<Value> value) {
    variables[name] = value;
}

std::shared_ptr<Value> Scope::get(const std::string name) const {
    if (!contains(name)) {
        if (!parent) {
            throwError(ErrorType::Runtime, std::format("Bad environment access with key '{}'", name));
        }

        return parent->get(name);
    }

    return variables.at(name);
}

bool Scope::contains(const std::string name) const {
    for (auto& pair : variables) {
        if (pair.first == name) {
            return true;
        }
    }
    return false;
}

void Scope::remove(std::string name) {
    variables.erase(name);
}


void Scope::addFunction(const std::string name, const std::shared_ptr<Value> func) {
    built_in_functions[name] = func;
}

std::shared_ptr<Value> Scope::getFunction(const std::string name) const {
    auto func = built_in_functions.find(name);
    if (func != built_in_functions.end()) {
        return func->second;
    }

    throwError(ErrorType::Runtime, "Unrecognized built-in function '" + name + "'");
}

bool Scope::hasFunction(const std::string name) const {
    auto func = built_in_functions.find(name);
    if (func != built_in_functions.end()) {
        return true;
    }
    
    return false;
}

void Scope::addMember(const ValueType value_type, const std::string name, const std::shared_ptr<Value> func) {
    type_members[value_type][name] = func;
}

std::shared_ptr<Value> Scope::getMember(const ValueType value_type, const std::string name) const {
    auto members = type_members.find(value_type);
    if (members != type_members.end()) {
        auto func = members->second.find(name);
        if (func != members->second.end()) {
            return func->second;
        }
    }

    throwError(ErrorType::Runtime, "Unrecognized member function '" + name + "' for type '" + getTypeStr(value_type) + "'");
}

bool Scope::hasMember(const ValueType value_type, const std::string name) const {
    auto members = type_members.find(value_type);
    if (members != type_members.end()) {
        auto func = members->second.find(name);
        if (func != members->second.end()) {
            return true;
        }

        return false;
    }

    return false;
}


std::shared_ptr<Scope> Scope::getGlobalScope() const {
    if (!parent) {
        return std::make_shared<Scope>(this);
    }

    return parent->getGlobalScope();
}

void Scope::makeClassScope(const bool is_class) {
    is_class_scope = is_class;
}

std::shared_ptr<Scope> Scope::enterScope() {
    Scope new_scope{std::make_shared<Scope>(this)};
    return std::make_shared<Scope>(new_scope);
}

std::shared_ptr<Scope> Scope::exitScope() {
    return parent;
}

std::shared_ptr<Scope> Scope::enterClassScope() {
    Scope new_scope{std::make_shared<Scope>(this)};
    new_scope.makeClassScope();
    return std::make_shared<Scope>(this);
}


const std::vector<std::pair<std::string, std::shared_ptr<Value>>> Scope::getPairs() const {
    std::vector<std::pair<std::string, std::shared_ptr<Value>>> pairs;
    for (const auto& pair : variables) {
        pairs.push_back(std::make_pair(pair.first, pair.second));
    }
    return pairs;
}

void Scope::display() const {
    for (const std::pair<const std::string, const std::shared_ptr<Value>> pair : variables) {
        const std::string& name = pair.first;
        const std::shared_ptr<Value> value = pair.second;

        std::cout << name << " = ";
        printValue(value);
        std::cout << std::endl;
    }
}