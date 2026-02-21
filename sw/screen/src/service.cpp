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
#include <cstring>      // snprintf
#include <string_view>  // string_view
#include <cmath>        // sin, cos

#include <nlohmann/json.hpp>

#include "paths.h"
#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "service.h"
#include "test.h"

#define PI 3.14159265

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
        std::this_thread::sleep_for(100ms);
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

    const bool force = ctx.enteringNewMode;

    if (force || m_dateHasChanged) {
        renderDateString(ctx);
    }
    if (force || m_timeHasChanged) {
        renderTimeString(ctx, force);
    }
    if (force || m_netHasChanged){
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

    const bool force = ctx.enteringNewMode;

    if (force) {
        renderAnalogClockFace(ctx);
    }
    if (force || m_timeHasChanged) {
        renderAnalogClockHands(ctx, force);
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

bool Service::renderTextBlock(Screen &s, const service::TextBlock &block, std::string_view text){

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

    // Format date
    static const char* months[] = {
        "Jan","Feb","Mar","Apr","May","Jun",
        "Jul","Aug","Sep","Oct","Nov","Dec"
    };

    char dateBuf[16];
    std::snprintf(dateBuf, sizeof(dateBuf),
                "%s %u %u",
                months[m_date.month - 1],
                m_date.day,
                m_date.year);

    renderTextBlock(s, service::InfoDateBlock, dateBuf);
}

void Service::renderTimeString(service::ScreenContext &ctx, const bool forceFullRender) {

    Screen &s = *ctx.screen;

    // Buffers
    std::array<char, 3> hoursBuf{};
    std::array<char, 3> minutesBuf{};
    std::array<char, 3> secondsBuf{};

    // Manual conversion to "HH", "MM", "SS"
    hoursBuf[0] = '0' + m_time.hour / 10;
    hoursBuf[1] = '0' + m_time.hour % 10;
    hoursBuf[2] = '\0';

    minutesBuf[0] = '0' + m_time.minute / 10;
    minutesBuf[1] = '0' + m_time.minute % 10;
    minutesBuf[2] = '\0';

    secondsBuf[0] = '0' + m_time.second / 10;
    secondsBuf[1] = '0' + m_time.second % 10;
    secondsBuf[2] = '\0';

    // Seconds tick char
    std::string_view tickBuf = (m_time.second % 2) ? "." : " ";

    // Render hours
    if (forceFullRender || m_time.hour != m_prevTime.hour) {
        renderTextBlock(s, service::InfoHoursBlock, std::string_view(hoursBuf.data(), 2));
    }
    // Render first colon
    if (forceFullRender) {
        renderTextBlock(s, service::InfoFirstColonBlock, ":");
    }
    // Render minutes
    if (forceFullRender || m_time.minute != m_prevTime.minute) {
        renderTextBlock(s, service::InfoMinutesBlock, std::string_view(minutesBuf.data(), 2));
    }
    // Render second colon
    if (forceFullRender && ctx.subMode == service::ScreenSubMode::HourMinuteSecond) {
        renderTextBlock(s, service::InfoSecondColonBlock, ":");
    }
    // Render seconds or tick
    switch(ctx.subMode) {
        case service::ScreenSubMode::HourMinute:
            break;
        case service::ScreenSubMode::HourMinuteSecond:
            if (forceFullRender || m_time.second != m_prevTime.second) {
                renderTextBlock(s, service::InfoSecondsBlock, std::string_view(secondsBuf.data(), 2));
            }
            break;
        case service::ScreenSubMode::HourMinuteTick:
            if (forceFullRender || m_time.second != m_prevTime.second) {
                renderTextBlock(s, service::InfoTickBlock, tickBuf);
            }
            break;
        default:
            std::cout << "Unknown sub mode" << std::endl;
            break;
    }
}

void Service::renderIpString(service::ScreenContext &ctx) {

    Screen &s = *ctx.screen;

    if (!m_net.interfaceUp) {
        renderTextBlock(s, service::InfoIpBlock, "Interface down");
        renderTextBlock(s, service::InfoMaskBlock, "");
    }
    else if (!m_net.hasCarrier) {
        renderTextBlock(s, service::InfoIpBlock, "No carrier");
        renderTextBlock(s, service::InfoMaskBlock, "");
    }
    else if (!m_net.isIPv4) {
        renderTextBlock(s, service::InfoIpBlock, "No IP");
        renderTextBlock(s, service::InfoMaskBlock, "");
    }
    else {
        renderTextBlock(s, service::InfoIpBlock, formatIPv4(m_net.ip));
        renderTextBlock(s, service::InfoMaskBlock, formatIPv4(m_net.netmask));
    }
}

void Service::renderAnalogClockFace(service::ScreenContext &ctx) {

    Screen &s = *ctx.screen;
    uint8_t x_coord = (screen::Geometry::Columns / 2) - (screen::Geometry::Rows / 2);
    uint8_t y_coord = 0;
    uint8_t d = screen::Geometry::Rows;
    screen::Color c = screen::StandardColor::White;
    s.drawCircle(x_coord, y_coord, d, c);
    std::this_thread::sleep_for(1ms);

    for (int quarter = 0; quarter < 4; quarter++) {
        float baseAngle = 0 + (quarter * (PI / 2));
        bool upperHalf = (quarter == 0 || quarter == 3) ? true : false;
        bool rightHalf = (quarter == 0 || quarter == 1) ? true : false;
        for (int tick = 0; tick < 4; tick++) {
            float drawAngle = baseAngle + (tick * (PI / 6));
            uint8_t cx = (screen::Geometry::Columns / 2) + static_cast<uint8_t>(rightHalf);
            uint8_t cy = (screen::Geometry::Rows / 2) - static_cast<uint8_t>(upperHalf);
            uint8_t x1 = cx + (25 * sin(drawAngle));
            uint8_t y1 = cy - (25 * cos(drawAngle));
            uint8_t x2 = cx + (31 * sin(drawAngle));
            uint8_t y2 = cy - (31 * cos(drawAngle));
            s.drawLine(x1, y1, x2, y2, c);
            std::this_thread::sleep_for(1ms);
        }
    }
}

void Service::renderAnalogClockHands(service::ScreenContext &ctx, const bool forceFullRender) {

    Screen &s = *ctx.screen;

    screen::Color c = screen::StandardColor::White;

    // Clock hands
    if (forceFullRender) {
        service::Line h = calcHourLine(m_time);
        service::Line m = calcMinuteLine(m_time);
        s.drawLine(h.x1, h.y1, h.x2, h.y2, c);
        std::this_thread::sleep_for(1ms);
        s.drawLine(m.x1, m.y1, m.x2, m.y2, c);
        std::this_thread::sleep_for(1ms);
    } else if (m_time.hour != m_prevTime.hour || m_time.minute != m_prevTime.minute) {
        service::Line h_prev = calcHourLine(m_prevTime);
        service::Line m_prev = calcMinuteLine(m_prevTime);
        s.clearWindow(h_prev.x1, h_prev.y1, h_prev.x2, h_prev.y2);
        std::this_thread::sleep_for(1ms);
        s.clearWindow(m_prev.x1, m_prev.y1, m_prev.x2, m_prev.y2);
        std::this_thread::sleep_for(1ms);
        service::Line h = calcHourLine(m_time);
        service::Line m = calcMinuteLine(m_time);
        s.drawLine(h.x1, h.y1, h.x2, h.y2, c);
        std::this_thread::sleep_for(1ms);
        s.drawLine(m.x1, m.y1, m.x2, m.y2, c);
        std::this_thread::sleep_for(1ms);
    }

    // Auxiliar seconds or tick
    // Seconds buffer
    std::array<char, 3> secondsBuf{};
    secondsBuf[0] = '0' + m_time.second / 10;
    secondsBuf[1] = '0' + m_time.second % 10;
    secondsBuf[2] = '\0';
    // Seconds tick char
    std::string_view tickBuf = (m_time.second % 2) ? "." : " ";
    // Render seconds or tick
    switch(ctx.subMode) {
        case service::ScreenSubMode::HourMinute:
            break;
        case service::ScreenSubMode::HourMinuteSecond:
            if (forceFullRender || m_time.second != m_prevTime.second) {
                renderTextBlock(s, service::AnalogClockSecondsBlock, std::string_view(secondsBuf.data(), 2));
            }
            break;
        case service::ScreenSubMode::HourMinuteTick:
            if (forceFullRender || m_time.second != m_prevTime.second) {
                renderTextBlock(s, service::AnalogClockTickBlock, tickBuf);
            }
            break;
        default:
            std::cout << "Unknown sub mode" << std::endl;
            break;
    }
}

service::Line Service::calcHourLine(const service::Time &t) {

    service::Line hourLine{};

    uint8_t hour12 = (t.hour >= 12) ? (t.hour - 12) : t.hour ;

    bool hourUpperHalf = (hour12 <= 3 || hour12 > 9) ? true : false;
    bool hourRightHalf = (hour12 > 0 && hour12 <= 6) ? true : false;

    float hourAngle = ((hour12 / 12.0) * 2 * PI) + ((t.minute / 60.0) * (2 * PI / 12.0));

    hourLine.x1 = (screen::Geometry::Columns / 2) + static_cast<uint8_t>(hourRightHalf);
    hourLine.y1 = (screen::Geometry::Rows / 2) - static_cast<uint8_t>(hourUpperHalf);
    hourLine.x2 = hourLine.x1 + (service::AnalogClockHourHandLength * sin(hourAngle));
    hourLine.y2 = hourLine.y1 - (service::AnalogClockHourHandLength * cos(hourAngle));

    return hourLine;
}

service::Line Service::calcMinuteLine(const service::Time &t) {

    service::Line minuteLine{};

    bool minuteUpperHalf = (t.minute <= 15 || t.minute > 45) ? true : false;
    bool minuteRightHalf = (t.minute > 0 && t.minute <= 30) ? true : false;

    float minuteAngle = (t.minute / 60.0) * 2 * PI;

    minuteLine.x1 = (screen::Geometry::Columns / 2) + static_cast<uint8_t>(minuteRightHalf);
    minuteLine.y1 = (screen::Geometry::Rows / 2) - static_cast<uint8_t>(minuteUpperHalf);
    minuteLine.x2 = minuteLine.x1 + (service::AnalogClockMinuteHandLength * sin(minuteAngle));
    minuteLine.y2 = minuteLine.y1 - (service::AnalogClockMinuteHandLength * cos(minuteAngle));

    return minuteLine;
}


void Service::updateDateAndTime() {

    std::time_t now = std::time(nullptr);
    std::tm *tm = std::localtime(&now);

    service::Date newDate {
        .year  = static_cast<uint16_t>(tm->tm_year + 1900),
        .month = static_cast<uint8_t>(tm->tm_mon + 1),
        .day   = static_cast<uint8_t>(tm->tm_mday)
    };

    service::Time newTime {
        .hour   = static_cast<uint8_t>(tm->tm_hour),
        .minute = static_cast<uint8_t>(tm->tm_min),
        .second = static_cast<uint8_t>(tm->tm_sec)
    };

    bool dateChanged =
        newDate.year   != m_date.year   ||
        newDate.month  != m_date.month  ||
        newDate.day    != m_date.day;

    bool timeChanged =
        newTime.hour   != m_time.hour   ||
        newTime.minute != m_time.minute ||
        newTime.second != m_time.second;

    if (dateChanged) {
        m_prevDate = m_date;
        m_date = newDate;
    }
    if (timeChanged) {
        m_prevTime = m_time;
        m_time = newTime;
    }

    m_dateHasChanged = dateChanged;
    m_timeHasChanged = timeChanged;
}

void Service::updateIpAndMask() {

    service::Network newNet{};
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

    bool changed = std::memcmp(&newNet, &m_net, sizeof(service::Network)) != 0;

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