#include <iostream> // cout, endl
#include <vector>   // vector
#include <thread>   // sleep_for
#include <chrono>   // time, seconds, milliseconds
#include <random>   // rand

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "screen_tester.h"

using namespace std::chrono_literals;

ScreenTester::ScreenTester(const std::vector<std::reference_wrapper<Screen>>& screens) {

    m_screens = screens;
}

void ScreenTester::testDisplay() {

    broadcast([](Screen& s){s.sendCommand(screen::Command::EntireDisplayOn);}, 500ms);
    broadcast([](Screen& s){s.sendCommand(screen::Command::EntireDisplayOff);}, 500ms);
    broadcast([](Screen& s){s.sendCommand(screen::Command::EntireDisplayOn);}, 500ms);
    broadcast([](Screen& s){s.sendCommand(screen::Command::NormalDisplay);}, 500ms);
}

void ScreenTester::testRemapColorDepth() {

    screen::Color colors[screen::Geometry::Pixels];

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color256); s.applyRemapColorDepth();});

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = screen::ColorLimit::R_565_MAX;
		colors[i].g = 0;
		colors[i].b = 0;
    }

    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 500ms);

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65k); s.applyRemapColorDepth();});

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = 0;
		colors[i].g = screen::ColorLimit::G_565_MAX;
		colors[i].b = 0;
    }

    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 500ms);

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65kAlt); s.applyRemapColorDepth();});

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = 0;
		colors[i].g = 0;
		colors[i].b = screen::ColorLimit::B_565_MAX;
    }

    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 500ms);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist31(0, 31);
    std::uniform_int_distribution<int> dist63(0, 63);

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color256); s.applyRemapColorDepth();});

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = dist31(gen);
        colors[i].g = dist63(gen);
        colors[i].b = dist31(gen);
    }

    broadcast([&colors](Screen& s){s.sendMultiPixel(colors, screen::Geometry::Pixels);}, 500ms);

    broadcast([](Screen& s){s.setColorDepth(screen::RemapColorDepth::Color65k); s.applyRemapColorDepth();});

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = dist31(gen);
        colors[i].g = dist63(gen);
        colors[i].b = dist31(gen);
    }

    broadcast([&colors](Screen& screen){screen.sendMultiPixel(colors, screen::Geometry::Pixels);}, 500ms);
}