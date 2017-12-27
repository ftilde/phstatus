#pragma once
#include "plugin.h"

#include <thread>
#include <memory>
#include <condition_variable>

struct TsState {
    TsState();
    bool connected_;
    bool microphoneMuted_;
    bool headphonesMuted_;
    bool talking_;
};

class TsDisplay : public Plugin {
public:
    TsDisplay(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
    virtual ~TsDisplay();

    void updateLoop();

    void update();
    bool print(BarOutput& output) const;
private:
    TsState currentTsState_;

    std::unique_ptr<std::thread> updateThread_;
    std::condition_variable quitUpdateThread_;

    std::string microphoneMutedStr_;
    std::string headphonesMutedStr_;
    std::string bothMutedStr_;
    std::string talkingStr_;
    std::string silentStr_;
};

extern "C" {
    Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters);
}
