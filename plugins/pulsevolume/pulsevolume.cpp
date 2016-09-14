#include "pulsevolume.h"

#include <sstream>
#include <regex>
#include <mutex>
#include <cmath>

#include "build_config.h"


namespace {

} // Anonymous namespace

PulseVolumePlugin::PulseVolumePlugin(const PluginBaseConstructionData& baseConstructionData, const ucl::Ucl& parameters)
    : Plugin(baseConstructionData)
    , mainLoop_(nullptr)
    , context_(nullptr)
    , output_("")
    , sinkName_(parameters["sinkName"].string_value("@DEFAULT_SINK@"))
    , portSymbols_()
{
    pulseInit();
    for(auto& elem : parameters["portSymbols"]) {
        portSymbols_[elem.key()] = elem.string_value();
    }
}
PulseVolumePlugin::~PulseVolumePlugin() {
    pa_context_disconnect(context_);
    //Somehow memory is leaked here. Not sure how to properly destroy the context.
    pa_threaded_mainloop_stop(mainLoop_);
    pa_threaded_mainloop_free(mainLoop_);
}

void PulseVolumePlugin::writeVolumeInfo(const pa_sink_info i) {
    pa_volume_t vol = pa_cvolume_avg(&i.volume);

    std::string output;
    if(portSymbols_.find(i.active_port->name) != portSymbols_.end()) {
        output = portSymbols_[i.active_port->name];
    } else {
        output = "Vol:";
    }
    output += " ";
    if(i.mute) {
        output += "off";
    } else {
        unsigned int intvol = static_cast<unsigned int>(std::round(static_cast<float>(vol) / PA_VOLUME_NORM * 100));
        output += std::to_string(intvol) + "%";
    }
    setOutputThreaded(output);
}
static void writeVolumeSinkInfo(pa_context* c, const pa_sink_info* i, int eol, void* userdata) {
    PulseVolumePlugin* plugin = static_cast<PulseVolumePlugin*>(userdata);
    if(eol > 0) {
        return;
    }
    if(!i) {
        plugin->setOutputThreaded("Error: Could not get sink info.");
        return;
    }
    if(eol < 0) {
        plugin->showPAErrorThreaded();
        return;
    }
    plugin->writeVolumeInfo(*i);
}

void PulseVolumePlugin::update() {
    pa_operation *o = pa_context_get_sink_info_by_name(context_, sinkName_.c_str(), writeVolumeSinkInfo, this);
    if (o) {
        pa_operation_unref(o);
    } else {
        showPAError();
    }
}
void PulseVolumePlugin::updateThreaded() {
    //TODO make sink configurable
    pa_operation *o = pa_context_get_sink_info_by_name(context_, sinkName_.c_str(), writeVolumeSinkInfo, this);
    if (o) {
        pa_operation_unref(o);
    } else {
        showPAErrorThreaded();
    }
}
void PulseVolumePlugin::showError(const std::string& err) {
    output_ = "Error: " + err;
    std::cerr << output_ << std::endl;
}
void PulseVolumePlugin::showErrorThreaded(const std::string& err) {
    setOutputThreaded("Error: " + err);
    std::cerr << output_ << std::endl;
}
void PulseVolumePlugin::showPAError() {
    showError(pa_strerror(pa_context_errno(context_)));
}
void PulseVolumePlugin::showPAErrorThreaded() {
    showErrorThreaded(pa_strerror(pa_context_errno(context_)));
}

static void pulse_event_callback(pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *userdata) {
    PulseVolumePlugin* plugin = static_cast<PulseVolumePlugin*>(userdata);
    plugin->updateThreaded();
}

static void context_state_callback(pa_context *c, void *userdata) {
    PulseVolumePlugin* plugin = static_cast<PulseVolumePlugin*>(userdata);
    switch (pa_context_get_state(c)) {

    case PA_CONTEXT_UNCONNECTED:
    case PA_CONTEXT_CONNECTING:
    case PA_CONTEXT_AUTHORIZING:
    case PA_CONTEXT_SETTING_NAME:
    case PA_CONTEXT_TERMINATED:
    default:
        break;

    case PA_CONTEXT_READY:
    {
        plugin->updateThreaded();

        pa_context_set_subscribe_callback(c, pulse_event_callback, plugin);
        pa_operation* o = pa_context_subscribe(c, PA_SUBSCRIPTION_MASK_SINK, nullptr, nullptr);
        if (o) {
            pa_operation_unref(o);
        } else {
            plugin->showPAErrorThreaded();
            return;
        }
    }
    break;

    case PA_CONTEXT_FAILED:
        plugin->showPAErrorThreaded();
        break;
    }
}

void PulseVolumePlugin::pulseInit() {
    const std::string applicationName = APPLICATION_NAME;
    const std::string applicationVersion = APPLICATION_VERSION;
    //TODO get via cmake

    if(!mainLoop_) {
        mainLoop_ = pa_threaded_mainloop_new();
        if(!mainLoop_) {
            showError("Could not create mainLoop");
            return;
        }
    }

    pa_mainloop_api* api = pa_threaded_mainloop_get_api(mainLoop_);
    if(!api) {
        showError("Could not get api");
        return;
    }

    pa_proplist *proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, applicationName.c_str());
    //pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, APP_ID);
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, applicationVersion.c_str());
    context_ = pa_context_new_with_proplist(api, applicationName.c_str(), proplist);
    pa_proplist_free(proplist);

    if(!context_) {
        showError("Could not create context");
        return;
    }
    //pa_context_set_subscribe_callback(context, pulse_callback, nullptr);
    pa_context_set_state_callback(context_, context_state_callback, this);
    if (pa_context_connect(context_, nullptr, PA_CONTEXT_NOFAIL/* | PA_CONTEXT_NOAUTOSPAWN*/, nullptr) < 0) {
        showPAError();
        //TODO check if needed to disconnect
        return;
    }
    if (pa_threaded_mainloop_start(mainLoop_) < 0) {
        showPAError();
        mainLoop_ = nullptr;
        return;
    }
}
void PulseVolumePlugin::setOutputThreaded(const std::string& str) {
    {
        std::unique_lock<std::mutex> lock(getUpdateMutex());
        output_ = str;
    }
    requestUpdate();
}
bool PulseVolumePlugin::print(BarOutput& output) const {
    output.put(output_);
    return !output_.empty();
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const ucl::Ucl& parameters) {
    return new PulseVolumePlugin(baseConstructionData, parameters);
}
