#pragma once
#include "plugin.h"

#include <vector>
#include <map>
#include <memory>
#include <condition_variable>

#include <pulse/pulseaudio.h>

class PulseVolumePlugin : public Plugin {
public:
    PulseVolumePlugin(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
    ~PulseVolumePlugin();
    void update();
    void updateThreaded();
    bool print(BarOutput& output) const;

    void showPAErrorThreaded();
    void setOutputThreaded(const std::string& str);

    void writeVolumeInfo(const pa_sink_info i);
private:
    void pulseInit();


    void showError(const std::string& err);
    void showErrorThreaded(const std::string& err);
    void showPAError();

    //TODO use unique_ptr?
    pa_threaded_mainloop* mainLoop_;
    pa_context* context_;
    std::string output_;

    const std::string sinkName_;
    std::map<std::string, std::string> portSymbols_;
};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
