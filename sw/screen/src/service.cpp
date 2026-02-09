#include <iostream>   // cout, endl
#include <fstream>    // ifstream
#include <stdexcept>  // runtime_error
#include <filesystem> // filesystem
#include <functional> // reference_wrapper

#include <nlohmann/json.hpp>

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "service.h"
#include "test.h"

using json = nlohmann::json;

Service::Service(const std::string &configFile) {

    // Load config file
    json j = loadJson(configFile);

    const auto &screens = j.at("screens");

    m_screens.reserve(screens.size());
    m_modes.reserve(screens.size());

    for (const auto &s : screens) {
        const std::string id  = s.at("id").get<std::string>();
        const std::string uio = s.at("uio").get<std::string>();

        m_screens.emplace_back(uio);
        m_modes.emplace_back(parseScreenMode(s.at("mode").get<std::string>()));
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

        // Find the screen by its ID
        const std::string id = s.at("id").get<std::string>();

        auto iterator = m_screenIndex.find(id);
        if (iterator == m_screenIndex.end()) {
            throw std::runtime_error("Unknown screen id in config: " + id);
        }

        const size_t index = iterator->second;

        Screen &screen = m_screens[index];

        // Apply the settings
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

void Service::runTests() {

    // Create a vector of references to all screens
    std::vector<std::reference_wrapper<Screen>> screenRefs;
    for (Screen &s : m_screens) {
        screenRefs.push_back(s);
    }

    // Create a Test object
    Test tester(screenRefs);

    std::cout << "Running tests..." << std::endl;
    tester.full();
    std::cout << "All tests completed." << std::endl;
}

void Service::run() {

    for (size_t i = 0; i < m_screens.size(); i++) {
        enterMode(i);
    }
}

void Service::enterMode(size_t index) {

    auto &screen = m_screens[index];
    auto &state  = m_modes[index];

    screen.clearScreen();

    switch (state) {
        case service::ScreenMode::None:
            enterNoneMode(screen);
            break;
        case service::ScreenMode::Ip:
            enterIpMode(screen);
            break;

        case service::ScreenMode::DigitalClock:
            enterDigitalClockMode(screen);
            break;

        case service::ScreenMode::AnalogClock:
            enterAnalogClockMode(screen);
            break;

        default:
            std::cout << "Unknown mode" << std::endl;
            break;
    }
}

void Service::enterNoneMode(Screen &s) {

    s.drawString("None", screen::StandardColor::White);
}

void Service::enterIpMode(Screen &s) {

    s.drawString("IP", screen::StandardColor::White);
}

void Service::enterDigitalClockMode(Screen &s) {

    s.drawString("DigitalClock", screen::StandardColor::White);
}

void Service::enterAnalogClockMode(Screen &s) {

    s.drawString("AnalogClock", screen::StandardColor::White);
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

service::ScreenMode Service::parseScreenMode(const std::string &s) {

    if (s == "None")         return service::ScreenMode::None;
    if (s == "Ip")           return service::ScreenMode::Ip;
    if (s == "DigitalClock") return service::ScreenMode::DigitalClock;
    if (s == "AnalogClock")  return service::ScreenMode::AnalogClock;

    throw std::runtime_error("Invalid screen mode value: " + s);
}

screen::Orientation Service::parseOrientation(const std::string &s) {

    if (s == "Horizontal_0")   return screen::Orientation::Horizontal_0;
    if (s == "Vertical_90")    return screen::Orientation::Vertical_90;
    if (s == "Horizontal_180") return screen::Orientation::Horizontal_180;
    if (s == "Vertical_270")   return screen::Orientation::Vertical_270;

    throw std::runtime_error("Invalid orientation value: " + s);
}