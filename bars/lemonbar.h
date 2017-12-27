#pragma once

#include "bar.h"

#include <string>
#include <pstreams/pstream.h>
#include <memory>

#include <yaml-cpp/yaml.h>

class LemonFormater : public Formater {
public:
    std::string addColor(const std::string& input, const Color& color) const;
    std::string addAction(const std::string& input, int mouseButton, const char* action) const;
};
class LemonBar : public Bar, public BarOutput {
public:
    LemonBar(YAML::Node node, ColorMap& colorMap);
    virtual ~LemonBar();
    const Formater& getFormater() const;
    BarOutput& beginLeftOuput();
    BarOutput& endLeftAndBeginRightOutput();
    void endRightOutput();

    void put(const char* text);
    void put(const std::string& text);
protected:
    const LemonFormater formater_;
    redi::opstream barProcess_;
};
