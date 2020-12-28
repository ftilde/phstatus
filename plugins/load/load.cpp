#include "load.h"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>


#if defined(__FreeBSD__) || defined(linux)
#include <err.h>
#include <sys/types.h>
#if !defined(linux)
#include <sys/sysctl.h>
#endif
static bool getCurrentLoad(float& result) {
    double loadavg[3];

    if (getloadavg(loadavg, 3) == -1)
        return false;

    result =  loadavg[0];
    return true;
}
#else
#error Plattform not supported
#endif


Load::Load(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , text_("")
    , lowColor_(getColorMap()[ColorIndex::BRIGHT_WHITE])
    , mediumColor_(getColorMap()[ColorIndex::BRIGHT_YELLOW])
    , highColor_(getColorMap()[ColorIndex::BRIGHT_RED])
    , mediumThreshold_(0.5)
    , highThreshold_(1.0)
{
}

Load::~Load() {
}

void Load::update() {
    float load;
    if(!getCurrentLoad(load)) {
        text_ = "Failed to get load";
        return;
    }

    const Color* color = nullptr;
    if(load > highThreshold_) {
        color = &highColor_;
    } else if(load > mediumThreshold_) {
        color = &mediumColor_;
    } else {
        color = &lowColor_;
    }

    std::ostringstream ss;
    ss.precision(2);
    ss << "↺ " << std::fixed << load;

    text_ = getFormater().addColor(ss.str(), *color);
}

bool Load::print(BarOutput& output) const {
    output.put(text_);
    return true;
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new Load(baseConstructionData, parameters);
}
