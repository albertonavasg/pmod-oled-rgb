#include <iostream>     // cout, endl
#include <fstream>      // ifstream
#include <stdexcept>    // runtime_error
#include <filesystem>   // filesystem
#include <functional>   // reference_wrapper
#include <chrono>       // chrono
#include <ctime>        // time, localtime
#include <sstream>      // ostringstream
#include <iomanip>      // put_time
#include <ifaddrs.h>    // ifaddrs
#include <arpa/inet.h>  // inet_ntop
#include <net/if.h>     // IFF_UP, IFF_LOOPBACK
#include <netinet/in.h> // sockaddr_in
#include <cstring>      // strcmp

#include <nlohmann/json.hpp>

#include "paths.h"
#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "service.h"
#include "test.h"

using json = nlohmann::json;

using namespace std::chrono_literals;

Service::Service(const std::string &configFile) {

    // Load config file
    json j = loadJson(configFile);

    const json &screens = j.at("screens");

    m_screens.reserve(screens.size());
    m_screenModes.reserve(screens.size());
    m_screenSubModes.reserve(screens.size());

    for (const json &s : screens) {
        const std::string id  = s.at("id").get<std::string>();
        const std::string uio = s.at("uio").get<std::string>();

        m_screens.emplace_back(uio);
        m_screenModes.emplace_back(parseScreenMode(s.at("mode").get<std::string>()));
        m_screenSubModes.emplace_back(parseScreenSubMode(s.at("subMode").get<std::string>()));
        m_screenIndex[id] = m_screens.size() - 1;
    }

    applyConfig(configFile);
}

Service::~Service() {
    stop();
}

void Service::applyConfig(const std::string &configFile) {

    // Load config file
    json j = loadJson(configFile);

    // Check for top-level key "screens" and check that json is an array
    if (!j.contains("screens") || !j["screens"].is_array()) {
        throw std::runtime_error("Config file missing 'screens' array");
    }

    const json &screens = j.at("screens");

    for (const json &s : screens) {

        // Find the screen by its ID
        const std::string id = s.at("id").get<std::string>();

        std::unordered_map<std::string, std::size_t>::iterator iterator = m_screenIndex.find(id);
        if (iterator == m_screenIndex.end()) {
            throw std::runtime_error("Unknown screen id in config: " + id);
        }

        const size_t index = iterator->second;

        Screen &screen = m_screens[index];

        // Apply the settings
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

    std::this_thread::sleep_for(100ms);

    while (m_running) {
        for (size_t i = 0; i < m_screens.size(); i++) {
            updateMode(i);
        }
        std::this_thread::sleep_for(100ms);
    }
}

void Service::stop() {

    if (!m_running.exchange(false, std::memory_order_relaxed)) {
        return; // Already stopped
    }
}

void Service::enterMode(size_t index) {

    Screen &screen = m_screens[index];
    service::ScreenMode &mode  = m_screenModes[index];

    screen.clearScreen();

    switch (mode) {
        case service::ScreenMode::None:
            enterNoneMode(screen, index);
            break;
        case service::ScreenMode::Info:
            enterInfoMode(screen);
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

void Service::enterNoneMode(Screen &s, size_t index) {

    std::string id;
    for (std::pair<const std::string, std::size_t> &pair : m_screenIndex) {
        if (pair.second == index) {
            id = pair.first;
            break;
        }
    }

    s.drawString(("Screen " + id).c_str(), 0, 0, screen::Font8x8, screen::StandardColor::White);
}

void Service::enterInfoMode(Screen &s) {

    bool timeChanged = updateDateAndTime();
    if (timeChanged) {
        service::TextBlock dateBlock {0, 0, 96, 8, &screen::Font8x8, screen::StandardColor::White};
        service::TextBlock timeBlock {0, 16, 96, 8, &screen::Font8x8, screen::StandardColor::White};

        renderTextBlock(s, dateBlock, m_date);
        renderTextBlock(s, timeBlock, m_time);
    }
    bool ipChanged = updateIpAndMask();
    if (ipChanged) {
        service::TextBlock ipBlock {0, 32, 96, 8, &screen::Font6x8, screen::StandardColor::White};
        service::TextBlock maskBlock {0, 48, 96, 8, &screen::Font6x8, screen::StandardColor::White};

        renderTextBlock(s, ipBlock, m_ip);
        renderTextBlock(s, maskBlock, m_mask);
    }
}

void Service::enterDigitalClockMode(Screen &s) {

    s.drawString("DigitalClock", 0, 0, screen::Font8x8, screen::StandardColor::White);
}

void Service::enterAnalogClockMode(Screen &s) {

    s.drawString("AnalogClock", 0, 0, screen::Font8x8, screen::StandardColor::White);
}

void Service::updateMode(size_t index) {

    Screen &screen = m_screens[index];
    service::ScreenMode &mode  = m_screenModes[index];

    switch (mode) {
        case service::ScreenMode::None:
            updateNoneMode(screen);
            break;
        case service::ScreenMode::Info:
            updateInfoMode(screen);
            break;
        case service::ScreenMode::DigitalClock:
            updateDigitalClockMode(screen);
            break;
        case service::ScreenMode::AnalogClock:
            updateAnalogClockMode(screen);
            break;
        default:
            std::cout << "Unknown mode" << std::endl;
            break;
    }
}

void Service::updateNoneMode(Screen &s) {

}

void Service::updateInfoMode(Screen &s) {

    bool timeChanged = updateDateAndTime();
    if (timeChanged) {
        service::TextBlock dateBlock {0, 0, 96, 8, &screen::Font8x8, screen::StandardColor::White};
        service::TextBlock timeBlock {0, 16, 96, 8, &screen::Font8x8, screen::StandardColor::White};

        renderTextBlock(s, dateBlock, m_date);
        renderTextBlock(s, timeBlock, m_time);
    }
    bool ipChanged = updateIpAndMask();
    if (ipChanged) {
        service::TextBlock ipBlock {0, 32, 96, 8, &screen::Font6x8, screen::StandardColor::White};
        service::TextBlock maskBlock {0, 48, 96, 8, &screen::Font6x8, screen::StandardColor::White};

        renderTextBlock(s, ipBlock, m_ip);
        renderTextBlock(s, maskBlock, m_mask);
    }
}

void Service::updateDigitalClockMode(Screen &s) {

}

void Service::updateAnalogClockMode(Screen &s) {

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
    if (s == "Info")         return service::ScreenMode::Info;
    if (s == "DigitalClock") return service::ScreenMode::DigitalClock;
    if (s == "AnalogClock")  return service::ScreenMode::AnalogClock;

    throw std::runtime_error("Invalid Screen Mode value: " + s);
}

service::ScreenSubMode Service::parseScreenSubMode(const std::string &s) {

    if (s == "None")             return service::ScreenSubMode::None;
    if (s == "HourMinute")       return service::ScreenSubMode::HourMinute;
    if (s == "HourMinuteSecond") return service::ScreenSubMode::HourMinuteSecond;
    if (s == "HourMinuteTick")   return service::ScreenSubMode::HourMinuteTick;

    throw std::runtime_error("Invalid Screen SubMode value: " + s);
}

screen::Orientation Service::parseOrientation(const std::string &s) {

    if (s == "Horizontal_0")   return screen::Orientation::Horizontal_0;
    if (s == "Vertical_90")    return screen::Orientation::Vertical_90;
    if (s == "Horizontal_180") return screen::Orientation::Horizontal_180;
    if (s == "Vertical_270")   return screen::Orientation::Vertical_270;

    throw std::runtime_error("Invalid Orientation value: " + s);
}

bool Service::renderTextBlock(Screen &s, const service::TextBlock &block, const std::string text){

    if (text.empty() || !block.font) {
        return false;
    }

    uint8_t charWidth  = block.font->width;
    // uint8_t charHeight = block.font->charHeight;

    // Maximum chars that fit horizontally
    size_t maxChars = block.width / charWidth;

    // Not render text if too long
    if (text.size() > maxChars) {
        return false;
    }

    s.clearWindow(block.x, block.y, block.x + block.width - 1, block.y + block.height - 1);
    std::this_thread::sleep_for(1ms);
    s.drawString(text, block.x, block.y, *(block.font), block.color);

    return true;
}

bool Service::updateDateAndTime() {

    std::string oldDate = m_date;
    std::string oldTime = m_time;

    // Get current system time
    std::time_t now = std::time(nullptr);
    std::tm *tm = std::localtime(&now);

    // Format date (Month Day Year)
    char buf[16];
    std::strftime(buf, sizeof(buf), "%b %-d %Y", tm);
    m_date = buf;

    // Format time (Hour:Minute)
    std::strftime(buf, sizeof(buf), "%H:%M", tm);
    m_time = buf;

    // Return true if visible state changed
    return (m_date != oldDate || m_time != oldTime);
}

bool Service::updateIpAndMask() {

    std::string oldIp   = m_ip;
    std::string oldMask = m_mask;

    struct ifaddrs *ifaddr;
    if (getifaddrs(&ifaddr) == -1) {
        return false; // Failed to read interface list
    }

    bool ipFound    = false;
    bool anyUp      = false;
    bool anyCarrier = false;

    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {

        if (!ifa->ifa_addr)
            continue; // Ignore interface entry without address info
        if (ifa->ifa_flags & IFF_LOOPBACK)
            continue; // Ignore loopback
        if (!(ifa->ifa_flags & IFF_UP))
            continue; // Ignore interface DOWN

        anyUp = true;

        if (!hasCarrier(ifa->ifa_name))
            continue; // Ignore interface with no carrier

        anyCarrier = true;

        if (ifa->ifa_addr->sa_family != AF_INET)
            continue; // Ignore interface if not IPv4

        char ipbuf[INET_ADDRSTRLEN];
        char maskbuf[INET_ADDRSTRLEN];

        struct sockaddr_in *addr    = (struct sockaddr_in *)ifa->ifa_addr;
        struct sockaddr_in *netmask = (struct sockaddr_in *)ifa->ifa_netmask;

        inet_ntop(AF_INET, &addr->sin_addr, ipbuf, sizeof(ipbuf));
        inet_ntop(AF_INET, &netmask->sin_addr, maskbuf, sizeof(maskbuf));

        m_ip   = ipbuf;
        m_mask = maskbuf;

        ipFound  = true;

        break; // Stop after first usable interface
    }

    freeifaddrs(ifaddr);

    if (!anyUp) {
        m_ip   = "Interface down";
        m_mask = "";
    } else if (!anyCarrier) {
        m_ip   = "No carrier";
        m_mask = "";
    } else if (!ipFound) {
        m_ip   = "No IP";
        m_mask = "No mask";
    }

    // Return true if visible state changed
    return (m_ip != oldIp || m_mask != oldMask);
}

bool Service::hasCarrier(const char *iface) {

    std::string path = std::string("/sys/class/net/") + iface + "/carrier";
    std::ifstream f(path);
    int carrier = 0;
    f >> carrier;
    return carrier == 1;
}