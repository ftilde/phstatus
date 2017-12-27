#include<yaml-cpp/yaml.h>

// Convenience upwrapping with default value
template<typename T>
T readOr(const YAML::Node& node, T defaultValue) {
    try {
        return node.as<T>();
    } catch(...) {
        return defaultValue;
    }
}

