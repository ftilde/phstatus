#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/filesystem.hpp>

#include "init.h"
#include "build_config.h"


std::string getEnvString(std::string const& key) {
    char * val = getenv(key.c_str());
    return (val == nullptr) ? "" : std::string(val);
}

std::string getDefaultConfigPath() {
    std::string path;
    path = getEnvString("XDG_CONFIG_HOME") + "/" DEFAULT_CONFIG_FILE_NAME;
    if(boost::filesystem::exists(path)) {
        return path;
    }
    path = getEnvString("HOME") + "/.config/" DEFAULT_CONFIG_FILE_NAME;
    if(boost::filesystem::exists(path)) {
        return path;
    }
    return "/etc/" DEFAULT_CONFIG_FILE_NAME;
}

int main(int argc, char** argv) {
    boost::program_options::options_description description("Options");
    description.add_options()
        ("help,h", "Display this help message.")
        ("version,v", "Display version information.")
        ("config,c", boost::program_options::value<std::string>(), "Configuration file");

    boost::program_options::variables_map variablesMap;
    try {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), variablesMap);
    } catch(boost::program_options::error& e) {
        std::cout << description << std::endl;
        return 0;
    }
    boost::program_options::notify(variablesMap);

    if(variablesMap.count("help")) {
        std::cout << description << std::endl;
        return 0;
    }

    if(variablesMap.count("version")) {
        std::cout << APPLICATION_NAME << " (" << APPLICATION_VERSION << ")" << std::endl;
        return 0;
    }

    std::string configPath = variablesMap.count("config") ? variablesMap["config"].as<std::string>() : getDefaultConfigPath();
    if(!boost::filesystem::exists(configPath)) {
        std::cerr << "Config file \"" << configPath << "\" does not exist." << std::endl;
        return 1;
    }

    std::unique_ptr<Status> s(Status::loadFromConfig(configPath));
    if(s) {
        s->mainLoop();
    } else {
        std::cerr << "Could not create status" << std::endl;
        return 1;
    }
}
