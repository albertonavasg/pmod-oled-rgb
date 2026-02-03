#include <iostream>   // cout, endl
#include <fstream>    // ifstream
#include <stdexcept>  // runtime_error
#include <filesystem> // filesystem

#include <nlohmann/json.hpp>

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "service.h"

using json = nlohmann::json;

Service::Service(const std::string &configFile) {

    // Load config file
    json j = loadJson(configFile);

    const auto &screens = j.at("screens");

    m_screens.reserve(screens.size());

    for (const auto &s : screens) {
        const std::string id  = s.at("id").get<std::string>();
        const std::string uio = s.at("uio").get<std::string>();

        m_screens.emplace_back(uio);
        m_screenIndex[id] = m_screens.size() - 1;
    }

    applyConfig(configFile);
}

void Service::applyConfig(const std::string &configFile) {

    // Load config file
    json j = loadJson(configFile);

    // Check for top-level key "screens" and check that json is an array
    if (!j.contains("screens") || !j["screens"].is_array()) {
        throw std::runtime_error("Config file missing 'screens' array");
    }

    const auto &screens = j.at("screens");

    for (const auto &s : screens) {

        const std::string id = s.at("id").get<std::string>();

        auto it = m_screenIndex.find(id);
        if (it == m_screenIndex.end()) {
            throw std::runtime_error("Unknown screen id in config: " + id);
        }

        Screen &screen = m_screens[it->second];

        const auto &textCursor = s.at("textCursor");

        screen.setTextCursor(static_cast<uint8_t>(textCursor.at(0).get<int>()), static_cast<uint8_t>(textCursor.at(1).get<int>()));
        screen.setSpiDelay(std::chrono::nanoseconds(s.at("spiDelay").get<int>()));
        screen.setScreenOrientation(parseOrientation(s.at("orientation").get<std::string>()));
        screen.setFillRectangleEnable(s.at("fillRectangle").get<bool>());
        screen.setReverseCopyEnable(s.at("reverseCopy").get<bool>());
    }
}

Screen& Service::screen(const std::string &id) {

    return m_screens.at(m_screenIndex.at(id));
}

json Service::loadJson(const std::string &path) const {

    // Check that file exists
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("Config file does not exist: " + path);
    }

    // Open file
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Failed to open config file: " + path);
    }

    json j;
    file >> j;

    return j;
}

screen::Orientation Service::parseOrientation(const std::string &s) {
    if (s == "Horizontal_0")   return screen::Orientation::Horizontal_0;
    if (s == "Vertical_90")    return screen::Orientation::Vertical_90;
    if (s == "Horizontal_180") return screen::Orientation::Horizontal_180;
    if (s == "Vertical_270")   return screen::Orientation::Vertical_270;

    throw std::runtime_error("Invalid orientation value: " + s);
}