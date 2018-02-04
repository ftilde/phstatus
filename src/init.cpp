#include "init.h"
#include "plugin.h"
#include "pluginloader.h"
#include "../bars/lemonbar.h"
#include "config_parsing.h"

#include <iostream>
#include <functional>
#include <fstream>
#include <chrono>
#include <thread>
#include <cmath>

#include <yaml-cpp/yaml.h>

Status::Status(Bar* bar, i3ipcConnection* i3con, i3ipcConnection* i3conEvents, std::unique_ptr<ColorMap> colorMap, const std::string& separator, unsigned int tickDurationInMs)
    : bar_(bar)
    , i3con_(i3con)
    , i3conEvents_(i3conEvents)
    , colorMap_(std::move(colorMap))
    , separator_(separator)
    , tickDuration_(tickDurationInMs)
    , unresolvedUpdateEvents_(EVT_NO_EVENT)
{
}
void Status::mainLoop() {
    registerI3IPCCallbacks();
    std::thread i3ipcthread(std::bind(i3ipc_connection_main, i3conEvents_));
    using clock = std::chrono::steady_clock;
    clock::time_point nextUpdate = clock::now();
    {
        std::unique_lock<std::mutex> lock(updateMutex_);
        long cycle = 0;
        updatePlugins();
        while(/*cycle<20*/ true) {
            if(updateRequested_.wait_until(lock, nextUpdate) == std::cv_status::timeout) {
                nextUpdate += getTickDuration();
                updatePluginsCycle(cycle);
                ++cycle;
            } else {
                updatePluginsEvent();
                unresolvedUpdateEvents_ = EVT_NO_EVENT;
            }
            updateBar();
        }
    }
    // This might be not thread safe...
    i3ipc_connection_main_quit(i3conEvents_);
    i3ipcthread.join();
}
void Status::updateBar() {
    BarOutput& output = bar_->beginLeftOuput();
    output.put("  ");
    for(auto pluginIt = leftAlignedPlugins_.begin(); pluginIt != leftAlignedPlugins_.end(); ++pluginIt) {
        Plugin* plugin = *pluginIt;
        bool wroteOutput = plugin->print(output);
        if(wroteOutput && pluginIt+1 != leftAlignedPlugins_.end()) {
            output.put(separator_);
        }
    }
    output = bar_->endLeftAndBeginRightOutput();
    for(auto pluginIt = rightAlignedPlugins_.begin(); pluginIt != rightAlignedPlugins_.end(); ++pluginIt) {
        Plugin* plugin = *pluginIt;
        bool wroteOutput = plugin->print(output);
        if(wroteOutput && pluginIt+1 != rightAlignedPlugins_.end()) {
            output.put(separator_);
        }
    }
    output.put("  ");
    bar_->endRightOutput();
}
void Status::updatePluginsCycle(unsigned long cycle) {
    for(Plugin* plugin : cycleUpdatedPlugins_) {
        if(cycle % plugin->getUpdateInterval() == 0) {
            plugin->update();
        }
    }
}
void Status::updatePluginsEvent() {
    for(std::unique_ptr<Plugin>& plugin : plugins_) {
        if(plugin->getUpdateEvents() & unresolvedUpdateEvents_) {
            plugin->update();
        }
    }
}
void Status::updatePlugins() {
    for(std::unique_ptr<Plugin>& plugin : plugins_) {
        plugin->update();
    }
}
void Status::requestUpdate(UpdateEvent hook) {
    {
        std::unique_lock<std::mutex> lock(updateMutex_);
        unresolvedUpdateEvents_ |= hook;
    }
    updateRequested_.notify_one();
}

std::chrono::milliseconds Status::getTickDuration() const {
    return tickDuration_;
}

Status::~Status() {
    g_object_unref(i3con_);
    g_object_unref(i3conEvents_);
}

void Status::addPluginLeft(Plugin* plugin) {
    plugins_.emplace_back(plugin);
    leftAlignedPlugins_.push_back(plugin);
    if(plugin->getUpdateInterval()) {
        cycleUpdatedPlugins_.push_back(plugin);
    }
}

void Status::addPluginRight(Plugin* plugin) {
    plugins_.emplace_back(plugin);
    rightAlignedPlugins_.push_back(plugin);
    if(plugin->getUpdateInterval()) {
        cycleUpdatedPlugins_.push_back(plugin);
    }
}

static Plugin* loadPluginFromConfig(const PluginLoader& pLoader, const StaticPluginBaseConstructionData& pluginInitInfo, const YAML::Node& pluginNode) {
    std::string name = pluginNode[std::string("name")].as<std::string>();
    PluginBaseConstructionData constructionData {
        0,
        EVT_NO_EVENT,
        pluginInitInfo
    };
    constructionData.updateInterval_ = readOr(pluginNode["updatePeriod"],0);
    for(auto updateHookNode : pluginNode["updateHooks"]) {
        std::string updateHook = updateHookNode.as<std::string>();
        if(updateHook == "workspace") {
            constructionData.updateHooks_ |= EVT_WORKSPACE;
        } else if(updateHook == "output") {
            constructionData.updateHooks_ |= EVT_OUTPUT;
        } else if(updateHook == "window") {
            constructionData.updateHooks_ |= EVT_WINDOW;
        } else if(updateHook == "mode") {
            constructionData.updateHooks_ |= EVT_MODE;
        } else if(updateHook == "binding") {
            constructionData.updateHooks_ |= EVT_BINDING;
        } else if(updateHook == "barconfig_update") {
            constructionData.updateHooks_ |= EVT_BARCONFIG_UPDATE;
        } else {
            std::cerr << "Invalid hook: " << updateHook << std::endl;
        }
    }
    return pLoader.loadPlugin(name, constructionData, pluginNode["parameters"]);
}

Status* Status::loadFromConfig(const std::string& configPath) {

    YAML::Node rootNode;
    try {
        rootNode = YAML::LoadFile(configPath);
    } catch(YAML::ParserException e) {
        std::cerr << "Could not parse config file " << configPath << std::endl;
        std::cerr << e.what() << std::endl;
        //TODO show some errors or something
        return nullptr;
    } catch(YAML::BadFile e) {
        std::cerr << "Could not open config file " << configPath << std::endl;
        return nullptr;
    }

    std::unique_ptr<ColorMap> colorMap = std::make_unique<ColorMap>(rootNode["colors"]);
    Bar* bar = new LemonBar(rootNode["bar"], *colorMap);

    std::string separator = bar->getFormater().addColor(readOr(rootNode["separator"], std::string("|")), (*colorMap)[readOr<unsigned int>(rootNode["separatorColor"], ColorIndex::BRIGHT_BLACK)]);
    int tickLength = static_cast<int>(std::round(1000*rootNode["tickLength"].as<float>())); //Convert to ms

    GError* error = nullptr;
    i3ipcConnection* i3con;
    i3con = i3ipc_connection_new(NULL, &error);
    if(error) {
        std::cerr << "Could not create i3connection: " << error->message << std::endl;
        return nullptr;
    }

    i3ipcConnection* i3conEvents;
    i3conEvents = i3ipc_connection_new(NULL, &error);
    if(error) {
        std::cerr << "Could not create i3connection: " << error->message << std::endl;
        return nullptr;
    }

    Status* status = new Status(bar, i3con, i3conEvents, std::move(colorMap), separator, tickLength);

    const StaticPluginBaseConstructionData pluginInitInfo {
        status->bar_->getFormater(),
        status->i3con_,
        *status->colorMap_,
        status->updateMutex_,
        std::bind(&Status::updateBar, status)
    };

    std::vector<std::string> binaryPluginPaths;
    std::vector<std::string> shellPluginPaths;
    PluginLoader pLoader(std::move(binaryPluginPaths), std::move(shellPluginPaths));

    auto leftAlignedPluginList = rootNode["leftAlignedPlugins"];
    if(leftAlignedPluginList) {
        for(const auto& pluginNode : leftAlignedPluginList) {
            Plugin* plugin = loadPluginFromConfig(pLoader, pluginInitInfo, pluginNode);
            if(plugin) {
                status->addPluginLeft(plugin);
            }
        }
    }

    auto rightAlignedPluginList = rootNode["rightAlignedPlugins"];
    if(rightAlignedPluginList) {
        for(const auto& pluginNode : rightAlignedPluginList) {
            Plugin* plugin = loadPluginFromConfig(pLoader, pluginInitInfo, pluginNode);
            if(plugin) {
                status->addPluginRight(plugin);
            }
        }
    }

    return status;
}

template<UpdateEvent hook>
void hookCallback(i3ipcConnection* con, i3ipcGenericEvent* evt, gpointer pstatus) {
    Status* status = (Status*)pstatus;
    status->requestUpdate(hook);
}

void Status::registerI3IPCCallbacks() {
    GError* error = NULL;
    UpdateEvents activeEvents = EVT_NO_EVENT;
    for(auto& plugin : plugins_) {
        activeEvents = activeEvents | plugin->getUpdateEvents();
    }
    if(       activeEvents & EVT_WORKSPACE) {
        GClosure* callback = g_cclosure_new(GCallback(hookCallback<EVT_WORKSPACE>), gpointer(this), NULL);
        i3conEvents_ = i3ipc_connection_on(i3conEvents_, "workspace", callback, &error);
    }
    if(activeEvents & EVT_OUTPUT) {
        GClosure* callback = g_cclosure_new(GCallback(hookCallback<EVT_OUTPUT>), gpointer(this), NULL);
        i3conEvents_ = i3ipc_connection_on(i3conEvents_, "output", callback, &error);
    }
    if(activeEvents & EVT_WINDOW) {
        GClosure* callback = g_cclosure_new(GCallback(hookCallback<EVT_WINDOW>), gpointer(this), NULL);
        i3conEvents_ = i3ipc_connection_on(i3conEvents_, "window", callback, &error);
    }
    if(activeEvents & EVT_MODE) {
        GClosure* callback = g_cclosure_new(GCallback(hookCallback<EVT_MODE>), gpointer(this), NULL);
        i3conEvents_ = i3ipc_connection_on(i3conEvents_, "mode", callback, &error);
    }
    if(activeEvents & EVT_BINDING) {
        GClosure* callback = g_cclosure_new(GCallback(hookCallback<EVT_BINDING>), gpointer(this), NULL);
        i3conEvents_ = i3ipc_connection_on(i3conEvents_, "binding", callback, &error);
    }
    if(activeEvents & EVT_BARCONFIG_UPDATE) {
        GClosure* callback = g_cclosure_new(GCallback(hookCallback<EVT_BINDING>), gpointer(this), NULL);
        i3conEvents_ = i3ipc_connection_on(i3conEvents_, "barconfig_update", callback, &error);
    }

    if(error != NULL) {
        std::cerr << "error connection_on: " << error->message << std::endl;
        exit(1);
    }
}

