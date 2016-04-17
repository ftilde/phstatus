#include "shellplugin.h"
#include <pstreams/pstream.h>

ShellPlugin::ShellPlugin(const PluginBaseConstructionData& baseConstructionData, const std::string& scriptPath, const ucl::Ucl& parameters)
    : Plugin(baseConstructionData)
    , output_("")
    , scriptPath_(scriptPath)
    , argv_()
{
    argv_.push_back(scriptPath_);
    for(auto& node : parameters) {
        argv_.push_back(node.string_value());
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
