#pragma once
#include "plugin.h"
#include "upower.h"

#include <thread>
#include <memory>
#include <condition_variable>

class Battery : public Plugin {
public:
    Battery(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
    virtual ~Battery();

    void updateLoop();

    void update();
    bool print(BarOutput& output) const;
private:

    UpClient* upClient_;
    std::string text_;

};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
