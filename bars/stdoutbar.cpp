#include "stdoutbar.h"
#include <iostream>
#include <chrono>

void StdoutOutput::put(const char* text) {
    std::cout << text;
}
void StdoutOutput::put(const std::string& text) {
    std::cout << text;
}
std::string NopFormater::addColor(const std::string& input, uint8_t r, uint8_t g, uint8_t b) const {
    return input;
}
std::string NopFormater::addAction(const std::string& input, int mouseButton, const char* action) const {
    return input;
}
const Formater* StdoutBar::getFormater() const {
    return &formater_;
}
StdoutBar::StdoutBar()
    : formater_()
    , output_()
{
}
StdoutBar::~StdoutBar() {
}
BarOutput* StdoutBar::beginLeftOuput() {
    output_.put("\r");
    return &output_;
}
BarOutput* StdoutBar::endLeftAndBeginRightOutput() {
    output_.put(" <----> ");
    return &output_;
}
void StdoutBar::endRightOutput() {
    std::cout << std::flush;
}
