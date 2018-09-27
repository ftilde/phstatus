#include "time.h"
#include "config_parsing.h"

#include <iostream>
#include <iomanip>
#include <ctime>


Time::Time(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , text_("")
    , format_(readOr(parameters["format"], std::string("%a %d.%m.%Y %H:%M:%S")))
{
}

Time::~Time() {
}

void Time::update() {
    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::ostringstream ss;
    ss << std::put_time(&tm, format_.c_str()) ;

    text_ = ss.str();
}

bool Time::print(BarOutput& output) const {
    output.put(text_);
    return true;
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new Time(baseConstructionData, parameters);
}
