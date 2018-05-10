#include "memory.h"
#include "config_parsing.h"

#include <iostream>


#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <err.h>
#include <sys/types.h>
#include <sys/sysctl.h>
static bool getFreeMemPercentage(int& result) {
    int result;
    size_t result_size = sizeof(result);

    if(sysctlbyname("vm.stats.vm.v_page_count", &result, &result_size, NULL, 0)) {
        return false;
    }
    int totalPageCount = result;

    if(sysctlbyname("vm.stats.vm.v_free_count", &result, &result_size, NULL, 0)) {
        return false;
    }
    int freePageCount = result;

    result = 100*(totalPageCount-freePageCount)/totalPageCount;
    return true;
}
#elif defined(linux)
#include <unistd.h>
static bool getFreeMemPercentage(int& result) {
    long totalPageCount = sysconf(_SC_PHYS_PAGES);
    if(totalPageCount < 0) {
        return false;
    }

    long freePageCount = sysconf(_SC_AVPHYS_PAGES);
    if(freePageCount < 0) {
        return false;
    }

    result = 100*(totalPageCount-freePageCount)/totalPageCount;
    return true;
}
#else
#error Plattform not supported
#endif


Memory::Memory(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , text_("")
    , lowColor_(getColorMap()[ColorIndex::BRIGHT_WHITE])
    , mediumColor_(getColorMap()[ColorIndex::BRIGHT_YELLOW])
    , highColor_(getColorMap()[ColorIndex::BRIGHT_RED])
    , mediumThreshold_(60)
    , highThreshold_(80)
{
}

Memory::~Memory() {
}

void Memory::update() {
    int memUsage;
    if(!getFreeMemPercentage(memUsage)) {
        text_ = "Failed to get memory info";
        return;
    }

    const Color* color = nullptr;
    if(memUsage > highThreshold_) {
        color = &highColor_;
    } else if(memUsage > mediumThreshold_) {
        color = &mediumColor_;
    } else {
        color = &lowColor_;
    }

    std::ostringstream ss;
    ss << "µ "<< memUsage << "%";

    text_ = getFormater().addColor(ss.str(), *color);
}

bool Memory::print(BarOutput& output) const {
    output.put(text_);
    return true;
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new Memory(baseConstructionData, parameters);
}
