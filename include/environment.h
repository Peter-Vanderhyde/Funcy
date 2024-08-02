#include <unordered_map>
#include <variant>
#include <string>
#include <format>
#include <vector>
#include <iostream>
#include <memory>


using Value = std::variant<int, double, bool, std::string>;


std::string getValueStr(std::shared_ptr<Value> value);


class Scope {
public:
    Scope() {};

    void set(const std::string variable, std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(const std::string variable) const;
    bool has(const std::string variable) const;
    // Method to display the contents of the environment
    void display() const;
private:
    std::unordered_map<std::string, std::shared_ptr<Value>> variables;
};


class Environment {
public:
    Environment() {};

    void addScope();
    void removeScope();

    void set(const std::string variable, std::shared_ptr<Value> value);
    std::shared_ptr<Value> get(const std::string variable) const;
    bool has(const std::string variable) const;
    // Method to display the contents of the environment
    void display() const;
private:
    std::vector<Scope> scopes;
};