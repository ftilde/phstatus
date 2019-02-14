#include "weather.h"
#include "config_parsing.h"

#include <sstream>
#include <regex>
#include <mutex>

#include <curl/curl.h>
#include <curl/easy.h>

namespace {

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    std::string data((const char*) ptr, (size_t) size * nmemb);
    *((std::stringstream*) stream) << data << std::endl;
    return size * nmemb;
}

} // Anonymous namespace

WeatherPlugin::WeatherPlugin(const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters)
    : Plugin(baseConstructionData)
    , url_("http://tgftp.nws.noaa.gov/data/observations/metar/decoded/" + readOr(parameters["airportCode"], std::string("VQPR")) + ".TXT")
    , updateThread_(nullptr)
    , threadRunning_()
    , output_("")
    , curl_(nullptr)
    , maxRetries_(10)
    , errorWaitTime_(5)
{
    curl_ = curl_easy_init();
}
WeatherPlugin::~WeatherPlugin() {
    if(updateThread_) {
        threadRunning_.clear();
        updateThreadSignal_.notify_all();
        updateThread_->join();
    }
    curl_easy_cleanup(curl_);
}
void WeatherPlugin::update() {
    if(!updateThread_) {
        threadRunning_.test_and_set();
        updateThread_ = std::make_unique<std::thread>(std::bind(&WeatherPlugin::getWeather, this));
    }
    updateThreadSignal_.notify_one();
}
static const std::regex temperatureRegex("Temperature: .*\\((-?\\d+) C\\)");
static const std::regex humidityRegex("Relative Humidity: .*?(\\d+)%");
static const std::regex pressureRegex("Pressure \\(altimeter\\): .*\\((\\d+) hPa\\)");
std::string WeatherPlugin::formatWeatherFromReply(std::stringstream& stream) const {
    std::string input = stream.str();
    std::string line;
    std::smatch match;
    std::ostringstream output;

    if(std::regex_search(input, match, temperatureRegex)) {
        output <<  match[1].str();
    } else {
        output <<  "?";
    }
    output << "°C  ";

    if(std::regex_search(input, match, humidityRegex)) {
        output <<  match[1].str();
    } else {
        output <<  "?";
    }
    output << "%  "; //TODO handle this in bar

    if(std::regex_search(input, match, pressureRegex)) {
        output <<  match[1].str();
    } else {
        output <<  "?";
    }
    output << " hPa";
    return output.str();
}
void WeatherPlugin::getWeather() {
    curl_easy_setopt(curl_, CURLOPT_URL, url_.c_str());
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_ACCEPT_ENCODING, "deflate");
    curl_easy_setopt(curl_, CURLOPT_NOSIGNAL, 1); // Make curl try to avoid SIGPIPE
    std::stringstream out;
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &out);

    using clock = std::chrono::steady_clock;
    std::mutex someMutex;
    std::unique_lock<std::mutex> someLock(someMutex);
    clock::time_point nextTry;
    while(threadRunning_.test_and_set()) {
        nextTry = clock::now();
        for(int ntry = 0; ntry<maxRetries_; ++ntry) {
            /* Perform the request, res will get the return code */
            CURLcode res = curl_easy_perform(curl_);
            if (res == CURLE_OK) {
                setOutputThreaded(formatWeatherFromReply(out));
                break;
            }
            setOutputThreaded("No Internet Connection... (" + std::to_string(ntry) + ")");
            nextTry += errorWaitTime_;
            if(updateThreadSignal_.wait_until(someLock, nextTry) != std::cv_status::timeout) {
                goto loop_end;
            }
        }
        updateThreadSignal_.wait(someLock);
loop_end:
        ;
    }
}
void WeatherPlugin::setOutputThreaded(const std::string& str) {
    {
        std::unique_lock<std::mutex> lock(getUpdateMutex());
        output_ = str;
    }
    requestUpdate();
}
bool WeatherPlugin::print(BarOutput& output) const {
    output.put(output_);
    return !output_.empty();
}

Plugin* CREATE_PLUGIN (const PluginBaseConstructionData& baseConstructionData, const YAML::Node& parameters) {
    return new WeatherPlugin(baseConstructionData, parameters);
}
