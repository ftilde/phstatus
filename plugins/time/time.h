#pragma once
#include "plugin.h"

class Time : public Plugin {
public:
    Time(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
    virtual ~Time();

    void updateLoop();

    void update();
    bool print(BarOutput& output) const;
private:

    std::string text_;
    std::string format_;
};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
