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
    std::uniform_int_distribution<int> dist31(0, 31);
    std::uniform_int_distribution<int> dist63(0, 63);

    // Random pixels
    for (auto& c : colors) {
        c.r = dist31(gen);
        c.g = dist63(gen);
        c.b = dist31(gen);
    }
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::colorDepth() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);

    // Horizontal red and vertical green fade
    for (size_t i = 0; i < colors.size(); ++i) {
        colors[i].r = (i % screen::Geometry::Columns) * screen::ColorLimit::R_565_MAX / (screen::Geometry::Columns - 1);
        colors[i].g = (i / screen::Geometry::Columns) * screen::ColorLimit::G_565_MAX / (screen::Geometry::Rows - 1);
        colors[i].b = 0;
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
        colors[i].r = (i % screen::Geometry::Columns) * screen::ColorLimit::R_565_MAX / (screen::Geometry::Columns - 1);
        colors[i].g = 0;
        colors[i].b = (i / screen::Geometry::Columns) * screen::ColorLimit::B_565_MAX / (screen::Geometry::Rows - 1);
    }
    // Horizontal address increment
    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65kAlt); s.setAddressIncrement(false); s.applyRemapColorDepth();});
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    // Vertical green and horizontal blue fade
    for (size_t i = 0; i < colors.size(); i++) {
        colors[i].r = 0;
        colors[i].g = (i % screen::Geometry::Rows) * screen::ColorLimit::G_565_MAX / (screen::Geometry::Rows - 1);
        colors[i].b = (i / screen::Geometry::Rows) * screen::ColorLimit::B_565_MAX / (screen::Geometry::Columns - 1);
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
        colors[i].r = (i % (c2 - c1 + 1)) * screen::ColorLimit::R_565_MAX/ ((c2 - c1 + 1) - 1);
        colors[i].g = (i / (c2 - c1 + 1)) * screen::ColorLimit::G_565_MAX/ ((r2 - r1 + 1) - 1);
        colors[i].b = screen::ColorLimit::B_565_MAX;
    }
    broadcast([&colors, c1, r1, c2, r2](Screen& s){s.drawBitmap(c1, r1, c2, r2, colors);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.applyDefaultSettings();});
}

void Test::scrolling() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);
    uint8_t stripeWidth = 16;

    // Squares
    for (size_t i = 0; i < colors.size(); i++) {
        if (((i % screen::Geometry::Columns) / stripeWidth) % 2 != ((i / screen::Geometry::Columns) / stripeWidth) % 2) {
            colors[i].r = screen::ColorLimit::R_565_MAX;
            colors[i].g = 0;
            colors[i].b = screen::ColorLimit::B_565_MAX;
        } else {
            colors[i].r = 0;
            colors[i].g = screen::ColorLimit::G_565_MAX;
            colors[i].b = screen::ColorLimit::B_565_MAX;
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