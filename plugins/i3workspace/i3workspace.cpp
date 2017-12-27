#include "i3workspace.h"

#include <map>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <i3ipc-glib/i3ipc-glib.h>

namespace {
bool comparei3ipcWorkspaceReply(i3ipcWorkspaceReply* r1, i3ipcWorkspaceReply* r2) {
    return strcmp(r1->name, r2->name) < 0;
}
}

I3Workspace::I3Workspace(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , unfocusedColor_(getColorMap()[ColorIndex::WHITE])
    , focusedInactiveColor_(getColorMap()[ColorIndex::BLUE])
    , focusedColor_(getColorMap()[ColorIndex::BRIGHT_BLUE])
    , urgentColor_(getColorMap()[ColorIndex::YELLOW])
    , separatorColor_(getColorMap()[ColorIndex::BRIGHT_BLACK])
    , output_("")
    , monitorSeparator_("  |  ")
    , workspacePadding_(" ")
    , monitorNames_()
{
    for(const auto& monitor : parameters["monitorNames"]) {
        monitorNames_.push_back(monitor.as<std::string>());
    }
}
void I3Workspace::update() {
    GError* error = nullptr;
    GSList* workspaceList = i3ipc_connection_get_workspaces(getI3Con(), &error);
    if(error) {
        std::cerr << "Could not get workspaces: " << error->message << std::endl;
        return;
    }
    output_ = "";
    std::map<std::string, std::vector<i3ipcWorkspaceReply*>> monitorWorkspaceMap;
    for(GSList* workspace = workspaceList; workspace; workspace = workspace->next) {
        i3ipcWorkspaceReply* reply = static_cast<i3ipcWorkspaceReply*>(workspace->data);
        monitorWorkspaceMap[reply->output].push_back(reply);
    }
    for(auto monitorName = monitorNames_.begin(); monitorName != monitorNames_.end(); ++monitorName) {
        std::vector<i3ipcWorkspaceReply*>& workspaces = monitorWorkspaceMap[*monitorName];
        std::sort(workspaces.begin(), workspaces.end(), comparei3ipcWorkspaceReply);
        for(auto workspace = workspaces.begin(); workspace != workspaces.end(); ++workspace) {
            output_ += workspacePadding_;
            std::string wsname = getFormater().addAction((*workspace)->name, 1, (std::string("i3-msg workspace number ") + std::to_string((*workspace)->num)).c_str());
            if((*workspace)->urgent) {
                output_ += getFormater().addColor(wsname, urgentColor_);
            } else if((*workspace)->focused) {
                output_ += getFormater().addColor(wsname, focusedColor_);
            } else if((*workspace)->visible) {
                output_ += getFormater().addColor(wsname, focusedInactiveColor_);
            } else {
                output_ += getFormater().addColor(wsname, unfocusedColor_);
            }
            output_ += workspacePadding_;
        }
        if(!workspaces.empty() && (monitorName+1) != monitorNames_.end()) {
            output_ += getFormater().addColor(monitorSeparator_, separatorColor_);
        }
    }
    g_slist_free_full(workspaceList, [](void* p) {i3ipc_workspace_reply_free(static_cast<i3ipcWorkspaceReply*>(p));});
}
bool I3Workspace::print(BarOutput& output) const {
    output.put(output_);
    return !output_.empty();
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new I3Workspace(baseConstructionData, parameters);
}
