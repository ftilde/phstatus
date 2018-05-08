#pragma once
#include "plugin.h"

class Thermal : public Plugin {
public:
    Thermal(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
    virtual ~Thermal();

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

    std::string zone_;

};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
