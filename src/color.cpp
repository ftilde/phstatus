#include "color.h"

#include <limits>
#include <memory>
#include <sstream>
#include <iomanip>

Color Color::parse(const YAML::Node& node, const Color& defaultColor) {
    YAML::Node rNode = node["r"];
    YAML::Node gNode = node["g"];
    YAML::Node bNode = node["b"];
    try {
        //TODO: this might not work correctly for (1,1,1) as white
        uint8_t r = std::max(0, std::min(0xff, rNode.as<int>()));
        uint8_t g = std::max(0, std::min(0xff, gNode.as<int>()));
        uint8_t b = std::max(0, std::min(0xff, bNode.as<int>()));
        return Color(r, g, b);
    } catch(...) {
    }

    try {
        double r = std::max(0.0, std::min(1.0, rNode.as<double>()));
        double g = std::max(0.0, std::min(1.0, gNode.as<double>()));
        double b = std::max(0.0, std::min(1.0, bNode.as<double>()));
        return Color(r, g, b);
    } catch(...) {
    }

    //TODO error output?
    return defaultColor;
}
Color::Color(double r, double g, double b)
    : r_(r * std::numeric_limits<CHANNEL_PRECISION>::max())
    , g_(g * std::numeric_limits<CHANNEL_PRECISION>::max())
    , b_(b * std::numeric_limits<CHANNEL_PRECISION>::max())
{
}
Color::Color(CHANNEL_PRECISION r, CHANNEL_PRECISION g, CHANNEL_PRECISION b)
    : r_(r)
    , g_(g)
    , b_(b)
{
}
uint8_t Color::r() const {
    return r_/(std::numeric_limits<CHANNEL_PRECISION>::max()/std::numeric_limits<uint8_t>::max());
}
uint8_t Color::g() const {
    return g_/(std::numeric_limits<CHANNEL_PRECISION>::max()/std::numeric_limits<uint8_t>::max());
}
uint8_t Color::b() const {
    return b_/(std::numeric_limits<CHANNEL_PRECISION>::max()/std::numeric_limits<uint8_t>::max());
}
std::string Color::toString() const {
    std::ostringstream ss;
    ss << "#";
    ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint32_t>(r());
    ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint32_t>(g());
    ss << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint32_t>(b());
    return ss.str();
}
Color ColorMap::DEFAULT_COLORS[] = {
    {0.0, 0.0, 0.0},
    {0.5, 0.0, 0.0},
    {0.0, 0.5, 0.0},
    {0.5, 0.5, 0.0},
    {0.0, 0.0, 0.5},
    {0.5, 0.0, 0.5},
    {0.0, 0.5, 0.5},
    {0.5, 0.5, 0.5},
    {0.1, 0.1, 0.1},
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {1.0, 1.0, 0.0},
    {0.0, 0.0, 1.0},
    {1.0, 0.0, 1.0},
    {0.0, 1.0, 1.0},
    {1.0, 1.0, 1.0},
};

ColorMap::ColorMap(const YAML::Node& mapNode)
    : colors_()
{
    int i=0;
    for(const auto& colorNode : mapNode) {
        colors_.push_back(Color::parse(colorNode, DEFAULT_COLORS[i%16]));
        ++i;
    }
    for(; i<16; ++i) {
        colors_.push_back(DEFAULT_COLORS[i%16]);
    }
}
ColorMap::~ColorMap() {
}
const Color& ColorMap::operator[](size_t i) const {
    return colors_[i%colors_.size()];
}
