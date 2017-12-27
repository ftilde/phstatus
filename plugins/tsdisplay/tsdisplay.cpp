#include "tsdisplay.h"

#include <map>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <i3ipc-glib/i3ipc-glib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

const char* PIPE_PATH = "/tmp/tsDisplayPipe";

TsState::TsState()
    : connected_(false)
    , microphoneMuted_(false)
    , headphonesMuted_(false)
    , talking_(false)
{
}


TsDisplay::TsDisplay(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , currentTsState_()
    , updateThread_(nullptr)
    , quitUpdateThread_()
    , microphoneMutedStr_(getFormater().addColor("[M]", getColorMap()[ColorIndex::YELLOW]))
    , headphonesMutedStr_(getFormater().addColor("[H]", getColorMap()[ColorIndex::YELLOW]))
    , bothMutedStr_(getFormater().addColor("[MH]", getColorMap()[ColorIndex::YELLOW]))
    , talkingStr_(getFormater().addColor("[T]", getColorMap()[ColorIndex::BRIGHT_BLUE]))
    , silentStr_(getFormater().addColor("[C]", getColorMap()[ColorIndex::BRIGHT_BLACK]))
{
    updateThread_ = std::make_unique<std::thread>(std::bind(&TsDisplay::updateLoop, this));
}
TsDisplay::~TsDisplay() {
    if(updateThread_) {
        quitUpdateThread_.notify_all();
        updateThread_->join();
    }
}
void TsDisplay::updateLoop() {
    if(mkfifo(PIPE_PATH, S_IWUSR | S_IRUSR) < 0) {
        //std::cerr << "Could not create fifo" << std::endl;
    }
    while(true) {
        std::string instruction;
        std::ifstream fifo(PIPE_PATH, std::ifstream::in);
        while(getline(fifo, instruction)) {
            std::unique_lock<std::mutex> lock(getUpdateMutex());
            if(instruction == "connected") {
                currentTsState_.connected_ = true;
            } else if(instruction == "disconnected") {
                currentTsState_.connected_ = false;
            } else if(instruction == "talking") {
                currentTsState_.talking_ = true;
            } else if(instruction == "silent") {
                currentTsState_.talking_ = false;
            } else if(instruction == "microphoneMuted") {
                currentTsState_.microphoneMuted_ = true;
            } else if(instruction == "microphoneUnmuted") {
                currentTsState_.microphoneMuted_ = false;
            } else if(instruction == "headphonesMuted") {
                currentTsState_.headphonesMuted_ = true;
            } else if(instruction == "headphonesUnmuted") {
                currentTsState_.headphonesMuted_ = false;
            } else {
                std::cerr << "Invalid msg: " << instruction << std::endl;
            }
        }
        requestUpdate();
    }
}

void TsDisplay::update() {
}

bool TsDisplay::print(BarOutput& output) const {
    if(!currentTsState_.connected_) {
        return false;
    }

    if(currentTsState_.microphoneMuted_ && currentTsState_.headphonesMuted_) {
        output.put(bothMutedStr_);
    } else if(currentTsState_.microphoneMuted_) {
        output.put(microphoneMutedStr_);
    } else if(currentTsState_.headphonesMuted_) {
        output.put(headphonesMutedStr_);
    } else if(currentTsState_.talking_) {
        output.put(talkingStr_);
    } else {
        output.put(silentStr_);
    }
    return true;
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new TsDisplay(baseConstructionData, parameters);
}
