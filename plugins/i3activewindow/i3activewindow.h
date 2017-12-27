#pragma once
#include "plugin.h"

#include <vector>

class I3ActiveWindow : public Plugin {
public:
    I3ActiveWindow(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
    void update();
    bool print(BarOutput& output) const;
private:
    std::string output_;
    unsigned int maxChars_;
};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
