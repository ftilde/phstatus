#include "thermal.h"
#include "config_parsing.h"

#include <iostream>


#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <err.h>
#include <sys/types.h>
#include <sys/sysctl.h>
static float getCurrentTemperature(const char* zone) {
    int result;
    std::ostringstream full_zone;
    full_zone << "hw.acpi.thermal." << zone << ".temperature";
    size_t result_size = sizeof(result);
    if(sysctlbyname(full_zone.str().c_str(), &result, &result_size, NULL, 0)) {
        return -1000; // obviously invalid
    }
    int tmpInDecikelvin = result;

    return (tmpInDecikelvin-2731)*0.1f;
}

#elif defined(LINUX)
#error Linux not yet supported
#else
#error Plattform not supported
#endif


Thermal::Thermal(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , text_("")
    , lowColor_(getColorMap()[ColorIndex::BRIGHT_WHITE])
    , mediumColor_(getColorMap()[ColorIndex::BRIGHT_YELLOW])
    , highColor_(getColorMap()[ColorIndex::BRIGHT_RED])
    , mediumThreshold_(60)
    , highThreshold_(80)
    , zone_(readOr(parameters["zone"], std::string("")))
{
}

Thermal::~Thermal() {
}

void Thermal::update() {
    float temperature = getCurrentTemperature(zone_.c_str());

    const Color* color = nullptr;
    if(temperature > highThreshold_) {
        color = &highColor_;
    } else if(temperature > mediumThreshold_) {
        color = &mediumColor_;
    } else {
        color = &lowColor_;
    }

    std::ostringstream ss;
    ss << "⌘ "<< temperature << "°C";

    text_ = getFormater().addColor(ss.str(), *color);
}

bool Thermal::print(BarOutput& output) const {
    output.put(text_);
    return true;
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new Thermal(baseConstructionData, parameters);
}
