// global_context.h

#ifndef GLOBAL_CONTEXT_H
#define GLOBAL_CONTEXT_H

#include <string>
#include <vector>
#include "parser.h"

class GlobalContext {
public:
    static GlobalContext& instance();

    void setFilename(const std::string& filename);
    const std::string& getFilename() const;
    void removeFilename();

    void addFunction(const std::string& name, std::shared_ptr<Value> func);
    std::shared_ptr<Environment> getEnvironment();

private:
    GlobalContext() = default;
    std::vector<std::string> filename_list;
    std::shared_ptr<Environment> environment = std::make_shared<Environment>();
};

#endif // GLOBAL_CONTEXT_H
