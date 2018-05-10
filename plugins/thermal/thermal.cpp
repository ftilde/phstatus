#include "thermal.h"
#include "config_parsing.h"

#include <iostream>

// Symbolicly set obviously invalid temperature to somehow convey

#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <err.h>
#include <sys/types.h>
#include <sys/sysctl.h>
static bool getCurrentTemperature(const char* zone, float& res) {
    int result;
    std::ostringstream full_zone;
    full_zone << "hw.acpi.thermal." << zone << ".temperature";
    size_t result_size = sizeof(result);
    if(sysctlbyname(full_zone.str().c_str(), &result, &result_size, NULL, 0)) {
        return false;
    }
    int tmpInDecikelvin = result;

    res = (tmpInDecikelvin-2731)*0.1f;
    return true;
}
#elif defined(linux)
#include <fstream>
static bool getCurrentTemperature(const char* zone, float& res) {
    std::ostringstream file_name;
    file_name << "/sys/class/thermal/" << zone << "/temp";

    std::fstream fs(file_name.str().c_str(), std::fstream::in);

    if(fs.fail()) {
        return false;
    }
    float milliCelsius;
    fs >> milliCelsius;

    res = milliCelsius/1000.0f;
    return true;
}
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
    float temperature;
    if(!getCurrentTemperature(zone_.c_str(), temperature)) {
        text_ = "Failed to get temperature";
        return;
    }

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
