#pragma once
#include "plugin.h"

#include <vector>
#include <thread>
#include <memory>
#include <condition_variable>
#include <atomic>

class WeatherPlugin : public Plugin {
public:
    WeatherPlugin(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
    ~WeatherPlugin();
    void update();
    bool print(BarOutput& output) const;
private:
    std::string formatWeatherFromReply(std::stringstream& stream) const;
    void getWeather();
    void setOutputThreaded(const std::string&);

    const std::string url_;
    std::unique_ptr<std::thread> updateThread_;
    std::condition_variable updateThreadSignal_;
    std::atomic_flag threadRunning_;

    std::string output_;
    void* curl_;
    const int maxRetries_;
    const std::chrono::seconds errorWaitTime_;
};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
