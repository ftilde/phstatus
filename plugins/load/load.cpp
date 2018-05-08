#include "load.h"

#include <iostream>
#include <stdlib.h>
#include <unistd.h>


#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(LINUX)
#include <err.h>
#include <sys/types.h>
#include <sys/sysctl.h>
static float getCurrentLoad() {
    double loadavg[3];

    if (getloadavg(loadavg, 3) == -1)
        return -1;

    return loadavg[0];
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
    , mediumThreshold_(0.2)
    , highThreshold_(0.8)
{
}

Load::~Load() {
}

void Load::update() {
    float load = getCurrentLoad();
    auto numProcs = sysconf(_SC_NPROCESSORS_ONLN);
    float relativeLoad = load/numProcs;

    const Color* color = nullptr;
    if(relativeLoad > highThreshold_) {
        color = &highColor_;
    } else if(relativeLoad > mediumThreshold_) {
        color = &mediumColor_;
    } else {
        color = &lowColor_;
    }

    std::ostringstream ss;
    ss.precision(2);
    ss << "↺ "<< (relativeLoad*100)<< "%";

    text_ = getFormater().addColor(ss.str(), *color);
}

bool Load::print(BarOutput& output) const {
    output.put(text_);
    return true;
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new Load(baseConstructionData, parameters);
}
