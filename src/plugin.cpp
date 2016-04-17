#include "plugin.h"

Plugin::Plugin(const PluginBaseConstructionData& constructionData)
    : data_(constructionData)
{
}
int Plugin::getUpdateInterval() const {
    return data_.updateInterval_;
}
UpdateEvents Plugin::getUpdateEvents() const {
    return data_.updateHooks_;
}
const Formater& Plugin::getFormater() const {
    return data_.staticData_.formater_;
}
i3ipcConnection* Plugin::getI3Con() {
    return data_.staticData_.i3con_;
}
const ColorMap& Plugin::getColorMap() const {
    return data_.staticData_.colorMap_;
}
std::mutex& Plugin::getUpdateMutex() {
    return data_.staticData_.updateMutex_;
}

void Plugin::requestUpdate() const {
    data_.staticData_.requestUpdate_();
}
