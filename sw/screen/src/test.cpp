#include <iostream> // cout, endl
#include <vector>   // vector
#include <thread>   // sleep_for
#include <chrono>   // time, seconds, milliseconds
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

    screen::Color colors[screen::Geometry::Pixels];

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist31(0, 31);
    std::uniform_int_distribution<int> dist63(0, 63);

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = dist31(gen);
        colors[i].g = dist63(gen);
        colors[i].b = dist31(gen);
    }

    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 1s);

    broadcast([](Screen& s){s.clearScreen();});

    broadcast([](Screen& s){s.applyRemapColorDepth(screen::RemapApplyMode::Default);});
}

void Test::colorDepth() {

    screen::Color colors[screen::Geometry::Pixels];

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = (i % screen::Geometry::Columns) * screen::ColorLimit::R_565_MAX / (screen::Geometry::Columns - 1);
        colors[i].g = (i / screen::Geometry::Columns) * screen::ColorLimit::G_565_MAX / (screen::Geometry::Rows - 1);
        colors[i].b = 0;
    }

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color256); s.applyRemapColorDepth();});
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65k); s.applyRemapColorDepth();});
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65kAlt); s.applyRemapColorDepth();});
    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 1s);
    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    broadcast([](Screen& s){s.applyRemapColorDepth(screen::RemapApplyMode::Default);});
}

void Test::addressIncrement() {

    screen::Color colors[screen::Geometry::Pixels];

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = (i % screen::Geometry::Columns) * screen::ColorLimit::R_565_MAX / (screen::Geometry::Columns - 1);
        colors[i].g = 0;
        colors[i].b = (i / screen::Geometry::Columns) * screen::ColorLimit::B_565_MAX / (screen::Geometry::Rows - 1);
    }

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65kAlt); s.setAddressIncrement(false); s.applyRemapColorDepth();});

    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 1s);

    broadcast([](Screen& s){s.clearScreen();}, 200ms);

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = 0;
        colors[i].g = (i % screen::Geometry::Rows) * screen::ColorLimit::G_565_MAX / (screen::Geometry::Rows - 1);
        colors[i].b = (i / screen::Geometry::Rows) * screen::ColorLimit::B_565_MAX / (screen::Geometry::Columns - 1);
    }

    broadcast([](Screen& s){s.setAddressIncrement(true); s.applyRemapColorDepth();});

    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 1s);

    broadcast([](Screen& s){s.clearScreen();}, 100ms);

    broadcast([](Screen& s){s.applyRemapColorDepth(screen::RemapApplyMode::Default);});

}