#pragma once

#include <string>

#include "color.h"

class BarOutput {
public:
    virtual ~BarOutput() {}
    virtual void put(const char* text) = 0;
    virtual void put(const std::string& text) = 0;
};
class Formater {
public:
    virtual ~Formater() {}
    virtual std::string addColor(const std::string& input, const Color& color) const = 0;
    virtual std::string addAction(const std::string& input, int mouseButton, const char* action) const = 0;
};
class Bar {
public:
    virtual ~Bar() {}
    virtual const Formater& getFormater() const = 0;
    virtual BarOutput& beginLeftOuput() = 0;
    virtual BarOutput& endLeftAndBeginRightOutput() = 0;
    virtual void endRightOutput() = 0;
};
