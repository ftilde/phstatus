#pragma once
#include "plugin.h"

#include <vector>

class I3Workspace : public Plugin {
public:
    I3Workspace(const PluginBaseConstructionData& baseConstructionData, const ucl::Ucl& parameters);
    void update();
    bool print(BarOutput& output) const;
private:
    const Color& unfocusedColor_;
    const Color& focusedInactiveColor_;
    const Color& focusedColor_;
    const Color& urgentColor_;
    const Color& separatorColor_;

    std::string output_;
    const std::string monitorSeparator_;
    const std::string workspacePadding_;
    std::vector<std::string> monitorNames_;
};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const ucl::Ucl& parameters);
}
