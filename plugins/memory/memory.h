#pragma once
#include "plugin.h"

class Memory : public Plugin {
public:
    Memory(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
    virtual ~Memory();

    void updateLoop();

    void update();
    bool print(BarOutput& output) const;
private:

    std::string text_;

    const Color& lowColor_;
    const Color& mediumColor_;
    const Color& highColor_;
    const int mediumThreshold_;
    const int highThreshold_;
};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
