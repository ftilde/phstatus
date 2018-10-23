#include "lemonbar.h"
#include "config_parsing.h"
#include <iostream>
#include <chrono>
#include <regex>

namespace {

struct Geometry {
    size_t x;
    size_t y;
    size_t w;
    size_t h;
};

Geometry* getGeometry(const std::string& outputName) {
    redi::ipstream xrandrProc("xrandr --current");
    std::string line;
    std::smatch match;
    std::regex reg ("^" + outputName + " (?:\\w+ )?\\w+ (.+?)x(.+?)\\+(.+?)\\+(.+?).*");
    while (std::getline(xrandrProc, line)) {
        if(std::regex_search(line, match, reg)) {
            std::string wstr = match[1].str();
            size_t w = std::atoi(wstr.c_str());
            std::string hstr = match[2].str();
            size_t h = std::atoi(hstr.c_str());
            std::string xstr = match[3].str();
            size_t x = std::atoi(xstr.c_str());
            std::string ystr = match[4].str();
            size_t y = std::atoi(ystr.c_str());
            return new Geometry {
                x, y, w, h
            };
        }
    }

    return nullptr;
}

} // Anonymous namespace


std::string LemonFormater::addColor(const std::string& input, const Color& color) const {
    return "%{F" + color.toString() + "}" + input +  "%{F-}";
}
std::string LemonFormater::addAction(const std::string& input, int mouseButton, const char* action) const {
    return "%{A" + std::to_string(mouseButton) + ":" + action + ":}" + input + "%{A}";
}
const Formater& LemonBar::getFormater() const {
    return formater_;
}
LemonBar::LemonBar(YAML::Node node, ColorMap& colorMap)
    : formater_()
    , barProcess_()
{
    /*
    std::vector<std::string> argv;
    argv.push_back("lemonbar");
    argv.push_back("-B");
    argv.push_back(colorMap[node["backgroundColor"].int_value(ColorIndex::BLACK)].toString());
    argv.push_back("-F");
    argv.push_back(colorMap[node["foregroundColor"].int_value(ColorIndex::BRIGHT_WHITE)].toString());
    argv.push_back("-f");
    argv.push_back(node["font"].string_value("DejaVu-8"));
    std::unique_ptr<Geometry> g = std::unique_ptr<Geometry>(getGeometry(node["output"].string_value("")));
    argv.push_back("-g");
    argv.push_back(std::to_string(g->w) + "x" + std::to_string(node["height"].int_value(10)) + "+" + std::to_string(g->x) + "+0");
    if(node["bottom"]) {
        argv.push_back("-b");
    }
    barProcess_.open("leonbar", argv);
    */
    std::string argv;
    argv.append("lemonbar");
    argv.append(" ");
    argv.append("-B");
    argv.append(" \"");
    argv.append(colorMap[readOr<int>(node["backgroundColor"], ColorIndex::BLACK)].toString());
    argv.append("\" ");
    argv.append("-F");
    argv.append(" \"");
    argv.append(colorMap[readOr<int>(node["foregroundColor"], ColorIndex::BRIGHT_WHITE)].toString());
    argv.append("\" ");
    argv.append("-f");
    argv.append(" ");
    argv.append(readOr(node["font"], std::string("DejaVu-8")));
    argv.append(" ");
    std::unique_ptr<Geometry> g = std::unique_ptr<Geometry>(getGeometry(readOr(node["output"], std::string(""))));
    argv.append("-g");
    argv.append(" ");
    argv.append(std::to_string(g->w) + "x" + std::to_string(readOr(node["height"], 10)) + "+" + std::to_string(g->x) + "+0");
    argv.append(" ");
    argv.append("-a");
    argv.append("20");
    argv.append(" ");
    if(node["bottom"]) {
        argv.append("-b");
        argv.append(" ");
    }
    barProcess_.open(argv + " | sh > /dev/null");
}
LemonBar::~LemonBar() {
    //barProcess_.kill();
}
BarOutput& LemonBar::beginLeftOuput() {
    put("%{l}");
    return *this;
}
BarOutput& LemonBar::endLeftAndBeginRightOutput() {
    put("%{r}");
    return *this;
}
void LemonBar::endRightOutput() {
    put("\n");
    barProcess_.flush();
}

void LemonBar::put(const char* text) {
    barProcess_ << text;
}
void LemonBar::put(const std::string& text) {
    barProcess_ << text;
}
