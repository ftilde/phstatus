#include "testplugin.h"

TestPlugin::TestPlugin(const PluginBaseConstructionData& baseConstructionData, const ucl::Ucl&)
    : Plugin(baseConstructionData)
    , numUpdates_(0)
{
}
void TestPlugin::update() {
    ++numUpdates_;
}
bool TestPlugin::print(BarOutput& output) const {
    output.put(std::to_string(numUpdates_));
    return true;
}

Plugin* CREATE_PLUGIN(const PluginBaseConstructionData& baseConstructionData, const ucl::Ucl& parameters) {
    return new TestPlugin(baseConstructionData, parameters);
}
