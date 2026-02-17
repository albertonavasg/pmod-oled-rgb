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
#include <cstring>      // strcmp, snprintf

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

    for (const json &s : screens) {
        const std::string id  = s.at("id").get<std::string>();
        const std::string uio = s.at("uio").get<std::string>();
        const bool enteringNewMode = true;

        m_screens.push_back({
            std::make_unique<Screen>(uio),
            id,
            parseScreenMode(s.at("mode").get<std::string>()),
            parseScreenSubMode(s.at("subMode").get<std::string>()),
            enteringNewMode
        });
    }

    applyConfig(configFile);
}

Service::~Service() {
    stop();
}

void Service::applyConfig(const std::string &configFile) {

    // Load config file
    json j = loadJson(configFile);

    // Check for top-level key "screens" and ensure it's an array
    if (!j.contains("screens") || !j["screens"].is_array()) {
        throw std::runtime_error("Config file missing 'screens' array");
    }

    const json &screens = j.at("screens");

    for (const json &s : screens) {
        const std::string id = s.at("id").get<std::string>();

        // Find the screen by its ID
        std::vector<service::ScreenContext>::iterator it;
        it = std::find_if(
            m_screens.begin(),
            m_screens.end(),
            [&](const service::ScreenContext &ctx) { return ctx.id == id; });

        if (it == m_screens.end()) {
            throw std::runtime_error("Unknown screen id in config: " + id);
        }

        service::ScreenContext &ctx = *it;

        Screen &screen = *ctx.screen;

        // Apply the settings
        screen.setSpiDelay(std::chrono::nanoseconds(s.at("spiDelay").get<int>()));
        screen.setScreenOrientation(parseOrientation(s.at("orientation").get<std::string>()));
        screen.setFillRectangleEnable(s.at("fillRectangle").get<bool>());
        screen.setReverseCopyEnable(s.at("reverseCopy").get<bool>());
    }
}

void Service::runTests() {

    // Create a vector of references to all screens
    std::vector<std::reference_wrapper<Screen>> screenRefs;
    screenRefs.reserve(m_screens.size());

    for (service::ScreenContext &ctx : m_screens) {
        screenRefs.push_back(*ctx.screen);
    }

    // Create a Test object
    Test tester(screenRefs);

    std::cout << "Running tests..." << std::endl;
    tester.full();
    std::cout << "All tests completed." << std::endl;
}

void Service::run() {

    while (m_running) {
        updateDateAndTime();
        updateIpAndMask();
        // Update all screens
        for (service::ScreenContext &ctx : m_screens) {
            updateMode(ctx);
        }
        std::this_thread::sleep_for(400ms);
    }
}

void Service::stop() {

    if (!m_running.exchange(false, std::memory_order_relaxed)) {
        return; // Already stopped
    }
}

void Service::updateMode(service::ScreenContext &ctx) {

    if (ctx.enteringNewMode) {
        Screen &screen = *ctx.screen;
        screen.clearScreen();
    }

    switch (ctx.mode) {
        case service::ScreenMode::None:
            updateNoneMode(ctx);
            break;
        case service::ScreenMode::Info:
            updateInfoMode(ctx);
            break;
        case service::ScreenMode::DigitalClock:
            updateDigitalClockMode(ctx);
            break;
        case service::ScreenMode::AnalogClock:
            updateAnalogClockMode(ctx);
            break;
        default:
            std::cout << "Unknown mode" << std::endl;
            break;
    }

    if (ctx.enteringNewMode) {
        ctx.enteringNewMode = false;
    }
}

void Service::updateNoneMode(service::ScreenContext &ctx) {

    if (ctx.enteringNewMode) {
        Screen &s = *ctx.screen;
        const std::string &id = ctx.id;
        s.drawString(("Screen " + id).c_str(), 0, 0, screen::Font8x8, screen::StandardColor::White);
    }
}

void Service::updateInfoMode(service::ScreenContext &ctx) {

    // Time
    if (m_timeHasChanged)
    {
        renderDateString(ctx);
        renderTimeString(ctx);
    }

    // Network
    if (m_netHasChanged)
    {
        renderIpString(ctx);
    }
}

void Service::updateDigitalClockMode(service::ScreenContext &ctx) {

    if (ctx.enteringNewMode) {
        Screen &s = *ctx.screen;
        s.drawString("DigitalClock", 0, 0, screen::Font8x8, screen::StandardColor::White);
    }
}

void Service::updateAnalogClockMode(service::ScreenContext &ctx) {

    if (ctx.enteringNewMode) {
        Screen &s = *ctx.screen;
        s.drawString("AnalogClock", 0, 0, screen::Font8x8, screen::StandardColor::White);
    }
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

    uint8_t charWidth  = block.font.width;
    // uint8_t charHeight = block.font.charHeight;

    // Maximum chars that fit horizontally
    size_t maxChars = block.width / charWidth;

    // Not render text if too long
    if (text.size() > maxChars) {
        return false;
    }

    s.clearWindow(block.x, block.y, block.x + block.width - 1, block.y + block.height - 1);
    std::this_thread::sleep_for(1ms);

    // Nothing to draw, just cleared the screen
    if (text.empty()) {
        return true;
    }

    s.drawString(text, block.x, block.y, block.font, block.color);

    return true;
}

void Service::renderDateString(service::ScreenContext &ctx) {

    Screen &s = *ctx.screen;

    service::TextBlock dateBlock {0, 0, 96, 8, screen::Font8x8, screen::StandardColor::White};

    // ---- Format date ----
    static const char* months[] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };

    char dateBuf[16];
    std::snprintf(dateBuf, sizeof(dateBuf),
                "%s %u %u",
                months[m_time.month - 1],
                m_time.day,
                m_time.year);

    renderTextBlock(s, dateBlock, dateBuf);
}

void Service::renderTimeString(service::ScreenContext &ctx) {

    Screen &s = *ctx.screen;

    service::TextBlock timeBlock {0, 16, 96, 8, screen::Font8x8, screen::StandardColor::White};

    // ---- Format time HH:MM ----
    char timeBuf[6];
    std::snprintf(timeBuf, sizeof(timeBuf),
                "%02hhu:%02hhu",
                m_time.hour,
                m_time.minute);

    renderTextBlock(s, timeBlock, timeBuf);
}

void Service::renderIpString(service::ScreenContext &ctx) {

    Screen &s = *ctx.screen;

    service::TextBlock ipBlock   {0, 32, 96, 8, screen::Font6x8, screen::StandardColor::White};
    service::TextBlock maskBlock {0, 48, 96, 8, screen::Font6x8, screen::StandardColor::White};

    if (!m_net.interfaceUp) {
        renderTextBlock(s, ipBlock, "Interface down");
        renderTextBlock(s, maskBlock, "");
    }
    else if (!m_net.hasCarrier) {
        renderTextBlock(s, ipBlock, "No carrier");
        renderTextBlock(s, maskBlock, "");
    }
    else if (!m_net.isIPv4) {
        renderTextBlock(s, ipBlock, "No IP");
        renderTextBlock(s, maskBlock, "");
    }
    else {
        renderTextBlock(s, ipBlock, formatIPv4(m_net.ip));
        renderTextBlock(s, maskBlock, formatIPv4(m_net.netmask));
    }
}

void Service::updateDateAndTime() {

    std::time_t now = std::time(nullptr);
    std::tm *tm = std::localtime(&now);

    service::TimeData newTime{};
    newTime.year   = tm->tm_year + 1900;
    newTime.month  = tm->tm_mon + 1;
    newTime.day    = tm->tm_mday;
    newTime.hour   = tm->tm_hour;
    newTime.minute = tm->tm_min;
    newTime.second = tm->tm_sec;

    bool changed =
        newTime.year   != m_time.year   ||
        newTime.month  != m_time.month  ||
        newTime.day    != m_time.day    ||
        newTime.hour   != m_time.hour   ||
        newTime.minute != m_time.minute ||
        newTime.second != m_time.second;

    if (changed) {
        m_prevTime = m_time;
        m_time = newTime;
    }

    m_timeHasChanged = changed;
}

void Service::updateIpAndMask() {

    service::NetworkData newNet{};
    struct ifaddrs* ifaddr;

    if (getifaddrs(&ifaddr) == -1) {
        return; // Failed to read interface list
    }

    for (struct ifaddrs *ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {

        if (!ifa->ifa_addr)
            continue; // Ignore interface entry without address info
        if (ifa->ifa_flags & IFF_LOOPBACK)
            continue; // Ignore loopback
        if (!(ifa->ifa_flags & IFF_UP))
            continue; // Ignore interface DOWN

        newNet.interfaceUp = true;

        if (!hasCarrier(ifa->ifa_name))
            continue; // Ignore interface with no carrier

        newNet.hasCarrier = true;

        if (ifa->ifa_addr->sa_family != AF_INET)
            continue; // Ignore interface if not IPv4

        newNet.isIPv4 = true;

        struct sockaddr_in *addr    = (struct sockaddr_in *)ifa->ifa_addr;
        struct sockaddr_in *netmask = (struct sockaddr_in *)ifa->ifa_netmask;

        newNet.ip      = ntohl(addr->sin_addr.s_addr);
        newNet.netmask = ntohl(netmask->sin_addr.s_addr);

        break;
    }

    freeifaddrs(ifaddr);

    bool changed = std::memcmp(&newNet, &m_net, sizeof(service::NetworkData)) != 0;

    if (changed) {
        m_prevNet = m_net;
        m_net = newNet;
    }

    m_netHasChanged = changed;
}

bool Service::hasCarrier(const char *iface) {

    std::string path = std::string("/sys/class/net/") + iface + "/carrier";
    std::ifstream f(path);
    int carrier = 0;
    f >> carrier;
    return carrier == 1;
}

std::string Service::formatIPv4(uint32_t ip) {

    struct in_addr addr;
    addr.s_addr = htonl(ip);

    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr, buf, sizeof(buf));

    return buf;
}