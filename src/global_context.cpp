// global_context.cpp

#include "global_context.h"

GlobalContext& GlobalContext::instance() {
    static GlobalContext instance;
    return instance;
}

void GlobalContext::setFilename(const std::string& filename) {
    filename_list.push_back(filename);
}

const std::string& GlobalContext::getFilename() const {
    return filename_list.back();
}

void GlobalContext::removeFilename() {
    filename_list.pop_back();
}