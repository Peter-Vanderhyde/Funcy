#include "environment.h"
#include "library.h"
#include <format>
#include <iostream>
#include "errorDefs.h"
#include "values.h"

bool DETECT_RECURSION;

Scope::Scope(std::shared_ptr<Scope> parent)
    : parent{parent} {}

void Scope::set(const std::string name,
        const std::shared_ptr<Value> value,
        Scope& original_scope,
        std::unordered_map<std::string, std::shared_ptr<Value>>& original_scope_variables) {
    
    for (auto saved_name : global_list) {
        if (name == saved_name) {
            setGlobal(name, value);
            return;
        }
    }
    
    for (auto& pair : variables) {
        if (pair.first == name) {
            variables[name] = value;
            return;
        }
    }

    if (hasThis() && getThis()->contains(name, false)) {
        getThis()->set(name, value, false);
        return;
    }

    if (assigned_class && assigned_class->contains(name, false)) {
        assigned_class->setValue(name, value, false);
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
            return;
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
        for (auto saved_name : global_list) {
            if (name == saved_name) {
                setGlobal(name, value);
                return;
            }
        }
        
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
        for (auto saved_name : global_list) {
            if (name == saved_name) {
                return getGlobal(name);
            }
        }

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
    Scope new_scope{shared_from_this()};
    if (hasThis()) {
        new_scope.setThis(getThis());
    }
    return std::make_shared<Scope>(new_scope);
}

std::shared_ptr<Scope> Scope::exitScope() {
    return parent;
}

void Scope::assignClass(const std::shared_ptr<Class> defined_class) {
    assigned_class = defined_class;
    if (hasThis()) {
        setThis(nullptr);
    }
}

bool Scope::hasClassAssigned() const {
    return (assigned_class != nullptr);
}

std::shared_ptr<Class> Scope::getAssignedClass() const {
    return assigned_class;
}

void Scope::addGlobal(const std::string name) {
    global_list.push_back(name);
}

void Scope::setGlobal(const std::string name, const std::shared_ptr<Value> value) {
    if (!parent) {
        set(name, value, *this, variables);
    } else {
        parent->setGlobal(name, value);
    }
}

std::shared_ptr<Value> Scope::getGlobal(const std::string name) const {
    if (!parent) {
        if (!contains(name)) {
            throwError(ErrorType::Runtime, "Attempted to access global variable '" + name + "' before it was defined");
        }
        
        return get(name);
    } else {
        return parent->getGlobal(name);
    }
}

void Scope::setThis(const std::shared_ptr<Instance> instance) {
    this_ref = instance;
}

std::shared_ptr<Instance> Scope::getThis() const {
    if (this_ref) {
        return this_ref;
    } else {
        throwError(ErrorType::Runtime, "Attempted to access the current instance while outside of an instance");
    }
}

std::shared_ptr<Instance> Scope::findThis() const {
    if (this_ref) {
        return this_ref;
    }
    else if (!parent) {
        throwError(ErrorType::Runtime, "Attempted to access the current instance while outside of an instance");
    }
    else {
        return parent->findThis();
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

void Scope::display(int depth) const {
    std::string indent(depth * 4, ' ');
    std::string branch = (depth == 0) ? "> " : "\\___ ";
    
    // Header for the current scope level with nice colors[cite: 1]
    if (!parent) {
        std::cout << indent << branch << "\033[1;35m[Global Scope]\033[0m" << std::endl; //[cite: 1]
    } else if (hasThis()) {
        std::cout << indent << branch << "\033[1;36m[Scope Instance of " << getThis()->getClassName() << "]\033[0m" << std::endl; //[cite: 1]
    } else if (hasClassAssigned()) {
        std::cout << indent << branch << "\033[1;36m[Class Definition Scope: " << getAssignedClass()->getName() << "]\033[0m" << std::endl; //[cite: 1]
    } else {
        std::cout << indent << branch << "\033[1;32m[Local Scope Frame]\033[0m" << std::endl; //[cite: 1]
    }

    // Display variables inside this specific frame using safe ASCII bars[cite: 1]
    if (variables.empty()) { //[cite: 1]
        std::cout << indent << "    (no local variables)" << std::endl;
    } else {
        for (const auto& pair : variables) { //[cite: 1]
            std::cout << indent << "    +--- " << pair.first << " = " << pair.second->getPrintable(depth + 1) << std::endl; //[cite: 1, 2]
        }
    }

    std::cout << indent << "    |" << std::endl;

    // Recurse up to the parent environment[cite: 1]
    if (parent) { //[cite: 1]
        parent->display(depth + 1);
    }
}