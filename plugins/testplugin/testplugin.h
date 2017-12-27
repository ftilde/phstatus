#pragma once
#include "plugin.h"

class TestPlugin : public Plugin {
public:
    TestPlugin(const PluginBaseConstructionData& baseConstructionData, const YAML::Node&);
    void update();
    bool print(BarOutput& output) const;
private:
    unsigned int numUpdates_;
};

extern "C" {
    Plugin* CREATE_PLUGIN(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
