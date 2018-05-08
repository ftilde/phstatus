#pragma once
#include "plugin.h"

class Load : public Plugin {
public:
    Load(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
    virtual ~Load();

    void updateLoop();

    void update();
    bool print(BarOutput& output) const;
private:

    std::string text_;

    const Color& lowColor_;
    const Color& mediumColor_;
    const Color& highColor_;
    const float mediumThreshold_;
    const float highThreshold_;


};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
