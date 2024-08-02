#include "environment.h"


std::string getValueStr(std::shared_ptr<Value> value) {
    if (std::holds_alternative<int>(*value.get())) {
        return "int";
    } else if (std::holds_alternative<double>(*value.get())) {
        return "double";
    } else if (std::holds_alternative<bool>(*value.get())) {
        return "bool";
    } else if (std::holds_alternative<std::string>(*value.get())) {
        return "string";
    } else {
        throw std::runtime_error("Attempted to get string of unrecognized type.");
    }
    return "";
}

void Scope::set(const std::string variable, std::shared_ptr<Value> value) {
    variables[variable] = value;
}

std::shared_ptr<Value> Scope::get(const std::string variable) const {
    auto element = variables.find(variable);
    if (element != variables.end()) {
        return element->second;
    } else {
        throw std::runtime_error(std::format("Unrecognized variable {}.", variable));
    }
}

bool Scope::has(const std::string variable) const {
    return variables.find(variable) != variables.end();
}

// Method to display the contents of the environment
void Scope::display() const {
    for (const std::pair<const std::string, std::shared_ptr<Value>> pair : variables) {
        const std::string& name = pair.first;
        std::shared_ptr<Value> value = pair.second;

        std::cout << name << " = ";
        std::visit([](const auto& v) { std::cout << v; }, *value.get());
        std::cout << std::endl;
    }
}


void Environment::addScope() {
    scopes.push_back(Scope());
}

void Environment::removeScope() {
    scopes.pop_back();
}

void Environment::set(const std::string variable, std::shared_ptr<Value> value) {
    if (scopes.empty()) {
        throw std::runtime_error("Attempted to access empty environment.");
    }
    for (int i = scopes.size() - 1; i > -1; i--) {
        if (scopes.at(i).has(variable)) {
            scopes.at(i).set(variable, value);
            return;
        }
    }

    scopes.back().set(variable, value);
}

// Method to display the contents of the environment
void Environment::display() const {
    if (scopes.empty()) {
        throw std::runtime_error("No environment to display.");
    }

    for (auto scope : scopes) {
        scope.display();
    }
}

std::shared_ptr<Value> Environment::get(const std::string variable) const {
    if (scopes.empty()) {
        throw std::runtime_error("Attempted to access empty environment.");
    }
    for (const auto scope : scopes) {
        if (scope.has(variable)) {
            return scope.get(variable);
        }
    }

    throw std::runtime_error(std::format("Unrecognized variable {}.", variable));
}

bool Environment::has(const std::string variable) const {
    if (scopes.empty()) {
        throw std::runtime_error("Attempted to access empty environment.");
    }
    for (const auto scope : scopes) {
        if (scope.has(variable)) {
            return true;
        }
    }

    return false;
}