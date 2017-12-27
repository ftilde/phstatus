#include "shellplugin.h"
#include <pstreams/pstream.h>

ShellPlugin::ShellPlugin(const PluginBaseConstructionData& baseConstructionData, const std::string& scriptPath, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , output_("")
    , scriptPath_(scriptPath)
    , argv_()
{
    argv_.push_back(scriptPath_);
    for(const auto& node : parameters) {
        argv_.push_back(node.as<std::string>());
    }
}
void ShellPlugin::update() {
    redi::ipstream shellProc(scriptPath_, argv_);
    std::string line;
    output_ = "";
    while (std::getline(shellProc, line)) {
        output_ += line;
    }
}
bool ShellPlugin::print(BarOutput& output) const {
    output.put(output_);
    return !output_.empty();
}
