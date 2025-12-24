#include <iostream> // cout, endl
#include <vector>   // vector
#include <thread>   // sleep_for
#include <chrono>   // time
#include <random>   // rand

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "test.h"

using namespace std::chrono_literals;

Test::Test(const std::vector<std::reference_wrapper<Screen>>& screens) {

    m_screens = screens;
}

void Test::display() {

    broadcast([](Screen& s){s.sendCommand(screen::Command::EntireDisplayOn);}, 500ms);
    broadcast([](Screen& s){s.sendCommand(screen::Command::EntireDisplayOff);}, 500ms);
    broadcast([](Screen& s){s.sendCommand(screen::Command::EntireDisplayOn);}, 500ms);
    broadcast([](Screen& s){s.sendCommand(screen::Command::NormalDisplay);}, 500ms);

    broadcast([](Screen& s){s.clearScreen();});
}

void Test::randomPattern() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);

    // Random pixels
    for (auto& c : colors) {
        c = {dist31(gen), dist63(gen), dist31(gen)};
    }
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::colorDepth() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);

    // Horizontal red and vertical green fade
    for (size_t i = 0; i < colors.size(); i++) {
        const size_t x = i % screen::Geometry::Columns;
        const size_t y = i / screen::Geometry::Columns;
        colors[i] = {
            static_cast<uint8_t>(x * screen::ColorLimit::R_565_MAX / (screen::Geometry::Columns - 1)),
            static_cast<uint8_t>(y * screen::ColorLimit::G_565_MAX / (screen::Geometry::Rows - 1)),
            0
        };
    }

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color256); s.applyRemapColorDepth();});
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65k); s.applyRemapColorDepth();});
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65kAlt); s.applyRemapColorDepth();});
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::addressIncrement() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);

    broadcast([](Screen& s){s.setSpiDelay(1ns);});

    // Horizontal red and vertical blue fade
    for (size_t i = 0; i < colors.size(); i++) {
        const size_t x = i % screen::Geometry::Columns;
        const size_t y = i / screen::Geometry::Columns;
        colors[i] = {
            static_cast<uint8_t>(x * screen::ColorLimit::R_565_MAX / (screen::Geometry::Columns - 1)),
            0,
            static_cast<uint8_t>(y * screen::ColorLimit::B_565_MAX / (screen::Geometry::Rows - 1))
        };
    }
    // Horizontal address increment
    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65kAlt); s.setAddressIncrement(false); s.applyRemapColorDepth();});
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    // Vertical green and horizontal blue fade
    for (size_t i = 0; i < colors.size(); i++) {
        const size_t x = i % screen::Geometry::Rows;
        const size_t y = i / screen::Geometry::Rows;
        colors[i] = {
            0,
            static_cast<uint8_t>(x * screen::ColorLimit::G_565_MAX / (screen::Geometry::Rows - 1)),
            static_cast<uint8_t>(y * screen::ColorLimit::B_565_MAX / (screen::Geometry::Columns - 1))
        };
    }
    // Vertical address increment
    broadcast([](Screen& s){s.setAddressIncrement(true); s.applyRemapColorDepth();});
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::bitmap() {

    size_t size = screen::Geometry::Pixels / 4;

    size_t c1 = (screen::Geometry::Columns / 4);
    size_t r1 = (screen::Geometry::Rows / 4);
    size_t c2 = ((screen::Geometry::Columns / 4) * 3) - 1;
    size_t r2 = ((screen::Geometry::Rows / 4) * 3) - 1;

    std::vector<screen::Color> colors(size);

    broadcast([](Screen& s){s.setSpiDelay(1ns);});

    // Horizontal red and vertical green fade with constant blue
    for (size_t i = 0; i < colors.size(); i++) {
        const size_t columns = (c2 - c1 + 1);
        const size_t rows = (r2 - r1 + 1);
        const size_t x = i % columns;
        const size_t y = i / columns;
        colors[i] = {
            static_cast<uint8_t>(x * screen::ColorLimit::R_565_MAX / (columns - 1)),
            static_cast<uint8_t>(y * screen::ColorLimit::G_565_MAX / (rows - 1)),
            screen::ColorLimit::B_565_MAX,
        };
    }
    broadcast([&colors, c1, r1, c2, r2](Screen& s){s.drawBitmap(c1, r1, c2, r2, colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::scrolling() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);
    size_t stripeWidth = 16;

    // Squares
    for (size_t i = 0; i < colors.size(); i++) {
        const size_t xBlock = (i % screen::Geometry::Columns) / stripeWidth;
        const size_t yBlock = (i / screen::Geometry::Columns) / stripeWidth;
        if ((xBlock ^ yBlock) & 1) {
            colors[i] = {
                screen::ColorLimit::R_565_MAX,
                0,
                screen::ColorLimit::B_565_MAX
            };
        } else {
            colors[i] = {
                0,
                screen::ColorLimit::G_565_MAX,
                screen::ColorLimit::B_565_MAX
            };
        }
    }
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors);}, 1s);

    // Horizontal right scroll
    broadcast([&colors](Screen& s){s.setupScrolling(1, 0, screen::Geometry::Rows, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    // Horizontal right fast scroll
    broadcast([&colors](Screen& s){s.setupScrolling(4, 0, screen::Geometry::Rows, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    // Horizontal left scroll
    broadcast([&colors](Screen& s){s.setupScrolling(95, 0, screen::Geometry::Rows, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    // Horizontal left fast scroll
    broadcast([&colors](Screen& s){s.setupScrolling(92, 0, screen::Geometry::Rows, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);

    // Horizontal partial right scroll
    broadcast([&colors](Screen& s){s.setupScrolling(1, screen::Geometry::Rows/4, screen::Geometry::Rows/2, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    broadcast([&colors](Screen& s){s.setupScrolling(63, screen::Geometry::Rows/4, screen::Geometry::Rows/2, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);

    // Vertical up scroll
    broadcast([&colors](Screen& s){s.setupScrolling(1, 0, 0, 1, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    // Vertical up fast scroll
    broadcast([&colors](Screen& s){s.setupScrolling(1, 0, 0, 4, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    // Vertical down scroll
    broadcast([&colors](Screen& s){s.setupScrolling(1, 0, 0, 63, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    // Vertical down fast scroll
    broadcast([&colors](Screen& s){s.setupScrolling(1, 0, 0, 60, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);

    // Diagonal up right scroll
    broadcast([&colors](Screen& s){s.setupScrolling(1, 0, screen::Geometry::Rows, 1, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    // Diagonal up left scroll
    broadcast([&colors](Screen& s){s.setupScrolling(95, 0, screen::Geometry::Rows, 1, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    // Diagonal down right scroll
    broadcast([&colors](Screen& s){s.setupScrolling(1, 0, screen::Geometry::Rows, 63, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);
    // Diagonal down left scroll
    broadcast([&colors](Screen& s){s.setupScrolling(95, 0, screen::Geometry::Rows, 63, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen& s){s.enableScrolling(false);}, 200ms);

    broadcast([](Screen& s){s.clearScreen();}, 200ms);
    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::line() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);
    std::uniform_int_distribution<uint8_t> dist95(0, 95);

    // Random colors and coordinates for the lines
    for (size_t i = 0; i < 30; i++) {
        screen::Color c = {dist31(gen), dist63(gen), dist31(gen)};
        uint8_t x1 = dist95(gen);
        uint8_t y1 = dist63(gen);
        uint8_t x2 = dist95(gen);
        uint8_t y2 = dist63(gen);
        broadcast([=](Screen& s){s.drawLine(x1, y1, x2, y2, c);}, 100ms);
    }
    std::this_thread::sleep_for(1s);

    broadcast([](Screen& s){s.clearScreen();}, 200ms);
    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::rectangle() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);
    std::uniform_int_distribution<uint8_t> dist95(0, 95);

    // Random colors and coordinates for the rectangles with no fill
    for (size_t i = 0; i < 10; i++) {
        screen::Color c1 = {dist31(gen), dist63(gen), dist31(gen)};
        screen::Color c2 = {0, 0, 0};
        uint8_t x1 = dist95(gen);
        uint8_t y1 = dist63(gen);
        uint8_t x2 = dist95(gen);
        uint8_t y2 = dist63(gen);
        broadcast([=](Screen& s){s.drawRectangle(x1, y1, x2, y2, c1, c2);}, 100ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.enableFill(true, false);});
    // Random colors and coordinates for the rectangles with fill
    for (size_t i = 0; i < 10; i++) {
        screen::Color c1 = {dist31(gen), dist63(gen), dist31(gen)};
        screen::Color c2 = {dist31(gen), dist63(gen), dist31(gen)};
        uint8_t x1 = dist95(gen);
        uint8_t y1 = dist63(gen);
        uint8_t x2 = dist95(gen);
        uint8_t y2 = dist63(gen);
        broadcast([=](Screen& s){s.drawRectangle(x1, y1, x2, y2, c1, c2);}, 100ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    // Rectangles with fill that grow from a corner
    for (size_t i = 0; i < screen::Geometry::Rows; i++) {
        screen::Color c1 = {dist31(gen), dist63(gen), dist31(gen)};
        screen::Color c2 = {dist31(gen), dist63(gen), dist31(gen)};
        uint8_t x1 = 0;
        uint8_t y1 = 0;
        uint8_t x2 = i;
        uint8_t y2 = i;
        broadcast([=](Screen& s){s.drawRectangle(x1, y1, x2, y2, c1, c2);}, 100ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.clearScreen();}, 200ms);
    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::copy() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);

    screen::Color cLine = {dist31(gen), dist63(gen), dist31(gen)};
    screen::Color cFill = {0, 0, 0}; //Fill not enabled
    uint8_t x1 = 15;
    uint8_t y1 = 5;
    uint8_t x2 = 35;
    uint8_t y2 = 25;
    broadcast([=](Screen& s){s.drawRectangle(x1, y1, x2, y2, cLine, cFill);}, 500ms);

    broadcast([=](Screen& s){s.copyWindow(x1, y1, x2, y2, x1 + 5, y1 + 5);}, 500ms);
    broadcast([=](Screen& s){s.copyWindow(x1, y1, x2 + 5, y2 + 5, x1, y1 + 30);}, 500ms);
    broadcast([=](Screen& s){s.copyWindow(x1, y1, x2 + 5, y2 + 35, x1 + 40, y1);}, 1s);

    broadcast([](Screen& s){s.clearScreen();}, 200ms);
    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::image() {

    const std::string imagePath1 = "/home/petalinux/images/default1.jpg";
    const std::string imagePath2 = "/home/petalinux/images/default2.jpg";

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color256); s.applyRemapColorDepth();});
    broadcast([=](Screen& s){s.drawImage(imagePath1);}, 2s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);
    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65k); s.applyRemapColorDepth();});
    broadcast([=](Screen& s){s.drawImage(imagePath1);}, 2s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);
    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65kAlt); s.applyRemapColorDepth();});
    broadcast([=](Screen& s){s.drawImage(imagePath1);}, 2s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color256); s.applyRemapColorDepth();});
    broadcast([=](Screen& s){s.drawImage(imagePath2);}, 2s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);
    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65k); s.applyRemapColorDepth();});
    broadcast([=](Screen& s){s.drawImage(imagePath2);}, 2s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);
    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65kAlt); s.applyRemapColorDepth();});
    broadcast([=](Screen& s){s.drawImage(imagePath2);}, 2s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.applyDefaultSettings();});
}