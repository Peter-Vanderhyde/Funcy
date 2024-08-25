// global_context.h

#ifndef GLOBAL_CONTEXT_H
#define GLOBAL_CONTEXT_H

#include <string>
#include <vector>

class GlobalContext {
public:
    static GlobalContext& instance();

    void setFilename(const std::string& filename);
    const std::string& getFilename() const;
    void removeFilename();

private:
    GlobalContext() = default;
    std::vector<std::string> filename_list;
};

#endif // GLOBAL_CONTEXT_H
