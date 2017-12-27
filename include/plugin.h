#ifdef CREATE_PLUGIN
#undef CREATE_PLUGIN
#endif

#ifdef BUILTIN_PLUGIN
    #ifdef PLUGIN_NAME
        #ifndef CONCAT
            #define CONCAT(x, y) CONCAT2(x, y)
            #define CONCAT2(x, y) x ## y
        #endif
        #define CREATE_PLUGIN CONCAT(createPlugin_, PLUGIN_NAME)
    #else
        #error No PLUGIN_NAME specified.
    #endif
#else
    #define CREATE_PLUGIN createPlugin
#endif

#pragma once

#include "bar.h"

#include <functional>
#include <mutex>

extern "C"{
#include <i3ipc-glib/i3ipc-glib.h>
}
#include <yaml-cpp/yaml.h>

enum UpdateEvent {
    EVT_NO_EVENT         = 0,
    EVT_WORKSPACE        = (1 << 0),
    EVT_OUTPUT           = (1 << 1),
    EVT_WINDOW           = (1 << 2),
    EVT_MODE             = (1 << 3),
    EVT_BINDING          = (1 << 4),
    EVT_BARCONFIG_UPDATE = (1 << 5),
};
typedef int UpdateEvents;

struct StaticPluginBaseConstructionData {
    const Formater& formater_;
    i3ipcConnection* i3con_;
    const ColorMap& colorMap_;
    std::mutex& updateMutex_;

    std::function<void()> requestUpdate_;
};
struct PluginBaseConstructionData {
    int updateInterval_;
    UpdateEvents updateHooks_;

    const StaticPluginBaseConstructionData staticData_;
};

class Plugin {
public:
    explicit Plugin(const PluginBaseConstructionData& constructionData);
    virtual ~Plugin() {}
    virtual void update() = 0;
    virtual bool print(BarOutput& output) const = 0;

    int getUpdateInterval() const;
    UpdateEvents getUpdateEvents() const;

protected:
    const Formater& getFormater() const;
    i3ipcConnection* getI3Con();
    const ColorMap& getColorMap() const;
    std::mutex& getUpdateMutex();

    void requestUpdate() const;

private:
    PluginBaseConstructionData data_;
};
