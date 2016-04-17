#pragma once
#include <vector>
#include <chrono>
#include <memory>
#include <condition_variable>
#include <mutex>

extern "C" {
#include <i3ipc-glib/i3ipc-glib.h>
}

#include "plugin.h"
#include "bar.h"


class Status {
public:
    Status(Bar* bar, i3ipcConnection* i3con, i3ipcConnection* i3conEvents, std::unique_ptr<ColorMap> colorMap, const std::string& separator, unsigned int tickDurationInMs);
    virtual ~Status();
    void updateBar();
    void updatePluginsCycle(unsigned long cycle);
    void updatePluginsEvent();
    void updatePlugins();
    void requestUpdate(UpdateEvent hook);

    std::chrono::milliseconds getTickDuration() const;

    static Status* loadFromConfig(const std::string& configPath);

    void mainLoop();

protected:
    void registerI3IPCCallbacks();

    void addPluginLeft(Plugin* plugin);
    void addPluginRight(Plugin* plugin);

    std::unique_ptr<Bar> bar_;
    i3ipcConnection* i3con_;
    i3ipcConnection* i3conEvents_;
    const std::unique_ptr<ColorMap> colorMap_;
    const std::string separator_;
    const std::chrono::milliseconds tickDuration_;

    std::vector<std::unique_ptr<Plugin>> plugins_;
    std::vector<Plugin*> leftAlignedPlugins_;
    std::vector<Plugin*> rightAlignedPlugins_;
    std::vector<Plugin*> cycleUpdatedPlugins_;

    std::mutex updateMutex_;
    std::condition_variable updateRequested_;

    UpdateEvents unresolvedUpdateEvents_;
};
