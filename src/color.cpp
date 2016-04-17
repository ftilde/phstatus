#include "color.h"

#include <limits>
#include <memory>
#include <sstream>
#include <iomanip>

Color Color::parse(ucl::Ucl node, const Color& defaultColor) {
    ucl::Ucl rNode = node["r"];
    ucl::Ucl gNode = node["g"];
    ucl::Ucl bNode = node["b"];
    if(rNode.type() == UCL_FLOAT && gNode.type() == UCL_FLOAT && bNode.type() == UCL_FLOAT) {
        double r = std::max(0.0, std::min(1.0, rNode.number_value()));
        double g = std::max(0.0, std::min(1.0, gNode.number_value()));
        double b = std::max(0.0, std::min(1.0, bNode.number_value()));
        return Color(r, g, b);
    } else if(rNode.type() == UCL_INT && gNode.type() == UCL_INT && bNode.type() == UCL_INT) {
        uint8_t r = std::max(0l, std::min(0xffl, rNode.int_value()));
        uint8_t g = std::max(0l, std::min(0xffl, gNode.int_value()));
        uint8_t b = std::max(0l, std::min(0xffl, bNode.int_value()));
        return Color(r, g, b);
    } else {
        //TODO error output?
        return defaultColor;
    }
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

ColorMap::ColorMap(ucl::Ucl mapNode)
    : colors_()
{
    int i=0;
    for(auto& colorNode : mapNode) {
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
