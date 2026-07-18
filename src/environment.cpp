#include "environment.h"
#include "library.h"
#include <format>
#include <iostream>
#include "errorDefs.h"
#include "values.h"

bool DETECT_RECURSION;

Scope::Scope(const std::shared_ptr<Scope> parent)
    : parent{parent} {}

void Scope::set(const std::string name, const std::shared_ptr<Value> value, const bool member_variable) {
    if (member_variable) {
        if (assigned_class) {
            assigned_class->setValue(name, value);
            return;
        }

        if (!parent) {
            throwError(ErrorType::Runtime, "Attempted to set member variable '" + name + "' while outside of class");
        }

        parent->set(name, value, member_variable);
    } else {
        variables[name] = value;
    }
}

std::shared_ptr<Value> Scope::get(const std::string name) const {
    for (auto& pair : variables) {
        if (pair.first == name) {
            return pair.second;
        }
    }

    if (!parent) {
        throwError(ErrorType::Runtime, std::format("Bad environment access with key '{}'", name));
    }

    return parent->get(name);
}

bool Scope::contains(const std::string name, const bool member_variable) const {
    if (member_variable) {
        if (assigned_class) {
            return assigned_class->contains(name);
        }

        if (!parent) {
            return false;
        }

        return parent->contains(name, member_variable);
    } else {
        for (auto& pair : variables) {
            if (pair.first == name) {
                return true;
            }
        }

        if (!parent) {
            return false;
        }

        return parent->contains(name);
    }
}

void Scope::remove(std::string name) {
    if (contains(name)) {
        variables.erase(name);
    } else {
        if (!parent) {
            throwError(ErrorType::Runtime, "Could not find '" + name + "' to remove it");
        }

        parent->remove(name);
    }
}


void Scope::addFunction(const std::string name, const std::shared_ptr<Value> func) {
    if (!parent) {
        built_in_functions[name] = func;
        return;
    }

    parent->addFunction(name, func);
}

std::shared_ptr<Value> Scope::getFunction(const std::string name) const {
    if (!parent) {
        auto func = built_in_functions.find(name);
        if (func != built_in_functions.end()) {
            return func->second;
        }

        throwError(ErrorType::Runtime, "Unrecognized built-in function '" + name + "'");
    } else {
        return parent->getFunction(name);
    }
}

bool Scope::hasFunction(const std::string name) const {
    if (!parent) {
        auto func = built_in_functions.find(name);
        if (func != built_in_functions.end()) {
            return true;
        }
        
        return false;
    } else {
        return parent->hasFunction(name);
    }
}

void Scope::addMember(const ValueType value_type, const std::string name, const std::shared_ptr<Value> func) {
    if (!parent) {
        type_members[value_type][name] = func;
    } else {
        parent->addMember(value_type, name, func);
    }
}

std::shared_ptr<Value> Scope::getMember(const ValueType value_type, const std::string name) const {
    if (!parent) {
        auto members = type_members.find(value_type);
        if (members != type_members.end()) {
            auto func = members->second.find(name);
            if (func != members->second.end()) {
                return func->second;
            }
        }

        throwError(ErrorType::Runtime, "Unrecognized member function '" + name + "' for type '" + getTypeStr(value_type) + "'");
    } else {
        return parent->getMember(value_type, name);
    }
}

bool Scope::hasMember(const ValueType value_type, const std::string name) const {
    if (!parent) {
        auto members = type_members.find(value_type);
        if (members != type_members.end()) {
            auto func = members->second.find(name);
            if (func != members->second.end()) {
                return true;
            }

            return false;
        }

        return false;
    } else {
        return parent->hasMember(value_type, name);
    }
}


std::shared_ptr<Scope> Scope::getGlobalScope() {
    if (!parent) {
        return std::make_shared<Scope>(*this);
    }

    return parent->getGlobalScope();
}

std::shared_ptr<Scope> Scope::enterScope() {
    Scope new_scope{std::make_shared<Scope>(*this)};
    return std::make_shared<Scope>(new_scope);
}

std::shared_ptr<Scope> Scope::exitScope() {
    return parent;
}

void Scope::assignClass(const std::shared_ptr<Class>& defined_class) {
    assigned_class = defined_class;
}

void Scope::addGlobal(const std::string name) {
    if (!parent) {
        set(name, std::shared_ptr<Value>());
    } else {
        parent->addGlobal(name);
    }
}

void Scope::setThis(const std::shared_ptr<Instance> instance) {
    this_ref = instance;
}

std::shared_ptr<Instance> Scope::getThis() const {
    if (this_ref) {
        return this_ref;
    } else {
        if (!parent) {
            throwError(ErrorType::Runtime, "Attempted to access instance outside of instance");
        }

        if (assigned_class) {
            throwError(ErrorType::Runtime, "No reference to instance found");
        }

        return parent->getThis();
    }
}

void Scope::addLoop() {
    loop_depth += 1;
}

void Scope::removeLoop() {
    loop_depth -= 1;
}

bool Scope::inLoop() const {
    return loop_depth > 0;
}

void Scope::resetLoop() {
    loop_depth = 0;
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