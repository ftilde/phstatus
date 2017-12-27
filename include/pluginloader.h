#pragma once

#include <vector>

#include "bar.h"
#include "plugin.h"

class PluginLoader {

public:
    PluginLoader(const std::vector<std::string>&& binaryPluginPaths, const std::vector<std::string>&& shellPluginPaths);
    virtual ~PluginLoader();

    Plugin* loadPlugin(const std::string& name, const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) const;

protected:
    Plugin* loadBuiltinPlugin(const std::string& name, const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) const;
    Plugin* loadExternalPlugin(const std::string& name, const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) const;
    Plugin* loadShellPlugin(const std::string& name, const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) const;

    std::vector<std::string> binaryPluginPaths_;
    std::vector<std::string> shellPluginPaths_;
};

