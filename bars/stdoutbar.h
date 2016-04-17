#pragma once

#include "bar.h"
#include <string>

class StdoutOutput : public BarOutput {
public:
    void put(const char* text);
    void put(const std::string& text);
};
class NopFormater : public Formater {
public:
    std::string addColor(const std::string& input, uint8_t r, uint8_t g, uint8_t b) const;
    std::string addAction(const std::string& input, int mouseButton, const char* action) const;
};
class StdoutBar : public Bar {
public:
    StdoutBar();
    virtual ~StdoutBar();
    const Formater* getFormater() const;
    BarOutput* beginLeftOuput();
    BarOutput* endLeftAndBeginRightOutput();
    void endRightOutput();
protected:
    const NopFormater formater_;
    StdoutOutput output_;
};
