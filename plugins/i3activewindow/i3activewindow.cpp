#include "i3activewindow.h"
#include "config_parsing.h"

#include <map>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <i3ipc-glib/i3ipc-glib.h>

I3ActiveWindow::I3ActiveWindow(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , output_("")
    , maxChars_(readOr(parameters["maxChars"], 100))
{
}
void I3ActiveWindow::update() {
    GError* error = nullptr;
    i3ipcCon* node = i3ipc_connection_get_tree(getI3Con(), &error);
    if(error) {
        std::cerr << "Could not get workspaces: " << error->message << std::endl;
        output_ = error->message;
        return;
    }
    if(!node) {
        std::cerr << "No tree node" << std::endl;
        output_ = "";
        return;
    }
    i3ipcCon* nextNode = i3ipc_con_find_focused(node);
    while(nextNode) {
        node = nextNode;
        nextNode = i3ipc_con_find_focused(node);
    }
    const char* fullNamePtr = i3ipc_con_get_name(node);
    if(!fullNamePtr) {
        std::cerr << "i3ipc_con_get_name returned nullptr" << std::endl;
        output_ = "";
        return;
    }
    std::string fullName(fullNamePtr);
    if(fullName.size() <= maxChars_) {
        output_ = fullName;
    } else {
        output_  = fullName.substr(0, (maxChars_-3)/2);
        output_ += "...";
        output_ += fullName.substr(fullName.size()-(maxChars_-2)/2);
    }
    //TODO does anything need to be freed here?
}
bool I3ActiveWindow::print(BarOutput& output) const {
    output.put(output_);
    return !output_.empty();
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new I3ActiveWindow(baseConstructionData, parameters);
}
