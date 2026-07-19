#include "environment.h"
#include "library.h"
#include <format>
#include <iostream>
#include "errorDefs.h"
#include "values.h"

bool DETECT_RECURSION;

Scope::Scope(const std::shared_ptr<Scope> parent)
    : parent{parent} {}

void Scope::set(const std::string name,
        const std::shared_ptr<Value> value,
        Scope& original_scope,
        std::unordered_map<std::string, std::shared_ptr<Value>>& original_scope_variables) {
    
    for (auto& pair : variables) {
        if (pair.first == name) {
            variables[name] = value;
            return;
        }
    }

    if (assigned_class && assigned_class->contains(name, false)) {
        assigned_class->setValue(name, value, false);
        return;
    }

    if (hasThis() && getThis()->contains(name, false)) {
        getThis()->set(name, value, false);
        return;
    }

    if (!parent) {
        if (original_scope.hasClassAssigned()) {
            original_scope.getAssignedClass()->setValue(name, value, false);
        } else {
            original_scope_variables[name] = value;
        }
        return;
    }

    parent->set(name, value, original_scope, original_scope_variables);
}

void Scope::set(const std::string name, const std::shared_ptr<Value> value, const bool member_variable) {
    if (member_variable) {
        if (hasThis()) {
            getThis()->set(name, value);
        }
        else if (assigned_class) {
            assigned_class->setValue(name, value);
            return;
        }

        if (!parent) {
            throwError(ErrorType::Runtime, "Attempted to set member variable '" + name + "' while outside of class");
        }

        parent->set(name, value, member_variable);
    } else {
        if (!assigned_class) {
            // Behave normally
            set(name, value, *this, variables);
        } else {
            // It's a private class variable
            assigned_class->setValue(name, value, false);
        }
    }
}

std::shared_ptr<Value> Scope::get(const std::string name, const bool member_variable) const {
    if (member_variable) {
        if (hasThis()) {
            return getThis()->get(name);
        }
        else if (assigned_class) {
            return assigned_class->copyValue(name);
        }

        if (!parent) {
            throwError(ErrorType::Runtime, "Attempted to access a member variable while outside of an instance");
        }

        return parent->get(name, member_variable);
    } else {
        for (auto& pair : variables) {
            if (pair.first == name) {
                return pair.second;
            }
        }

        if (assigned_class && assigned_class->contains(name, false)) {
            return assigned_class->copyValue(name, false);
        }

        if (hasThis() && getThis()->contains(name, false)) {
            return getThis()->get(name, false);
        }

        if (!parent) {
            throwError(ErrorType::Runtime, std::format("Bad environment access with key '{}'", name));
        }

        return parent->get(name);
    }
}

bool Scope::find(const std::string name, const bool member_variable) const {
    if (member_variable) {
        if (hasThis()) {
            return getThis()->contains(name);
        }
        else if (assigned_class) {
            return assigned_class->contains(name);
        }

        if (!parent) {
            return false;
        }

        return parent->find(name, member_variable);
    } else {
        for (auto& pair : variables) {
            if (pair.first == name) {
                return true;
            }
        }

        if (assigned_class && assigned_class->contains(name, false)) {
            return true;
        }

        if (!parent) {
            return false;
        }

        return parent->find(name);
    }
}

bool Scope::contains(const std::string name, const bool member_variable) const {
    if (member_variable) {
        if (hasThis()) {
            return getThis()->contains(name);
        }
        else if (assigned_class) {
            return assigned_class->contains(name);
        }

        return false;
    } else {
        for (auto& pair : variables) {
            if (pair.first == name) {
                return true;
            }
        }

        if (assigned_class) {
            return assigned_class->contains(name, false);
        }

        return false;
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

void Scope::assignClass(const std::shared_ptr<Class> defined_class) {
    assigned_class = defined_class;
}

bool Scope::hasClassAssigned() const {
    return (assigned_class != nullptr);
}

std::shared_ptr<Class> Scope::getAssignedClass() const {
    return assigned_class;
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
        throwError(ErrorType::Runtime, "Attempted to access instance outside of instance");
    }
}

bool Scope::hasThis() const {
    return (this_ref != nullptr);
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