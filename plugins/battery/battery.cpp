#include "battery.h"

#include <map>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <sstream>


Battery::Battery(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , upClient_(up_client_new())
    , text_("")
{
}
Battery::~Battery() {
    g_object_unref(upClient_);
}

static void appendTime(std::ostringstream& ss, gint64 fullSeconds) {
    gint64 hours = fullSeconds/(60*60);
    gint64 minutes = (fullSeconds/60)%60;
    gint64 seconds = fullSeconds%(60);

    ss << hours << ":" << minutes << ":" << seconds;
}

void Battery::update() {
    UpDevice* device = up_client_get_display_device(upClient_);

    gdouble chargePercentage;
    UpDeviceState state;
    g_object_get(device,
            "state", &state,
            "percentage", &chargePercentage,
            NULL);

    std::ostringstream ss;
    switch(state) {
        case UP_DEVICE_STATE_UNKNOWN:
        case UP_DEVICE_STATE_LAST:
            ss << "??? " << chargePercentage << "%";
            break;

        case UP_DEVICE_STATE_PENDING_DISCHARGE:
        case UP_DEVICE_STATE_DISCHARGING:
        case UP_DEVICE_STATE_EMPTY:
            {
                gint64 secondsToEmpty;
                g_object_get(device,
                        "time-to-empty", &secondsToEmpty,
                        NULL);

                ss << "B ";
                ss << chargePercentage << "% ";
                appendTime(ss, secondsToEmpty);
                break;
            }

        case UP_DEVICE_STATE_CHARGING:
        case UP_DEVICE_STATE_PENDING_CHARGE:
            {
                gint64 secondsToFull;
                g_object_get(device,
                        "time-to-full", &secondsToFull,
                        NULL);

                ss << "C ";
                ss << chargePercentage << "% ";
                appendTime(ss, secondsToFull);
                break;
            }

        case UP_DEVICE_STATE_FULLY_CHARGED:
            ss << "F " << chargePercentage << "%";
    }

    text_ = ss.str();
    g_object_unref(device);
}

bool Battery::print(BarOutput& output) const {
    output.put(text_);
    return true;
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new Battery(baseConstructionData, parameters);
}
