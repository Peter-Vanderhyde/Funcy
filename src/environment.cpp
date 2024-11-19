#include "environment.h"
#include "library.h"
#include <format>


Environment::Environment() {}

void Environment::add(std::string name, std::shared_ptr<Value> value) {
    env[name] = value;
}

bool Environment::contains(std::string name) const {
    return static_cast<bool>(env.count(name));
}

std::shared_ptr<Value> Environment::get(std::string name) {
    if (contains(name)) {
        return env.at(name);
    } else {
        handleError(std::format("Bad environment access with key '{}'.", name), 0, 0, "Runtime Error");
    }
}