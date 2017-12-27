#pragma once

#include <vector>
#include <memory>

#include <yaml-cpp/yaml.h>

class Color {
    friend class ColorMap;
public:
    static Color parse(const YAML::Node& node, const Color& defaultColor);
    Color(double r, double g, double b);
    uint8_t r() const;
    uint8_t g() const;
    uint8_t b() const;
    std::string toString() const;
private:
    typedef uint8_t CHANNEL_PRECISION;

    Color(CHANNEL_PRECISION r, CHANNEL_PRECISION g, CHANNEL_PRECISION b);

    CHANNEL_PRECISION r_;
    CHANNEL_PRECISION g_;
    CHANNEL_PRECISION b_;
};

enum ColorIndex {
    BLACK          = 0,
    RED            = 1,
    GREEN          = 2,
    YELLOW         = 3,
    BLUE           = 4,
    MAGENTA        = 5,
    CYAN           = 6,
    WHITE          = 7,
    BRIGHT_BLACK   = 8,
    BRIGHT_RED     = 9,
    BRIGHT_GREEN   = 10,
    BRIGHT_YELLOW  = 11,
    BRIGHT_BLUE    = 12,
    BRIGHT_MAGENTA = 13,
    BRIGHT_CYAN    = 14,
    BRIGHT_WHITE   = 15
};

class ColorMap {
public:
    ColorMap(const YAML::Node& node);
    ColorMap(ColorMap&) = delete;
    ColorMap() = delete;
    ~ColorMap();
    const Color& operator[](size_t i) const;
private:
    static Color DEFAULT_COLORS[];
    std::vector<Color> colors_;
};
