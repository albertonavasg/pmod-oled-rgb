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

    // Check that file exists
    if (!std::filesystem::exists(configFile)) {
        throw std::runtime_error("Config file does not exist: " + configFile);
    }

    // Open file
    std::ifstream file(configFile);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file: " + configFile);
    }

    json j;
    file >> j;

    // Check for top-level key "screens" and check that json is an array
    if (!j.contains("screens") || !j["screens"].is_array()) {
        throw std::runtime_error("Config file missing 'screens' array");
    }

    const auto &screens = j["screens"];
    m_screens.reserve(screens.size());

    for (const auto &s : screens) {

        const std::string id = s.at("id").get<std::string>();
        const std::string uio = s.at("uio").get<std::string>();

        m_screens.emplace_back(uio);
        m_screenIndex[id] = m_screens.size() - 1;
        Screen &screen = m_screens.back();

        const auto& textCursor = s.at("textCursor");

        screen.setTextCursor        (static_cast<uint8_t>(textCursor[0].get<int>()), static_cast<uint8_t>(textCursor[1].get<int>()) );
        screen.setSpiDelay          (std::chrono::nanoseconds(s.at("spiDelay").get<int>()));
        screen.setScreenOrientation (parseOrientation(s.at("orientation").get<std::string>()));
        screen.enableFillRectangle  (s.at("fillRectangle").get<bool>());
        screen.enableReverseCopy    (s.at("reverseCopy").get<bool>());
    }
}

Screen& Service::screen(const std::string& id) {

    return m_screens.at(m_screenIndex.at(id));
}

screen::Orientation Service::parseOrientation(const std::string &s) {
    if (s == "Horizontal_0")   return screen::Orientation::Horizontal_0;
    if (s == "Vertical_90")    return screen::Orientation::Vertical_90;
    if (s == "Horizontal_180") return screen::Orientation::Horizontal_180;
    if (s == "Vertical_270")   return screen::Orientation::Vertical_270;

    throw std::runtime_error("Invalid orientation value: " + s);
}