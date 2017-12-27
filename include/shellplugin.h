#pragma once
#include "plugin.h"

#include <vector>
#include <string>

class ShellPlugin : public Plugin {
public:
    ShellPlugin(const PluginBaseConstructionData& baseConstructionData, const std::string& scriptPath, const YAML::Node&);
    void update();
    bool print(BarOutput& output) const;
private:
    std::string output_;
    std::string scriptPath_;
    std::vector<std::string> argv_;
};
