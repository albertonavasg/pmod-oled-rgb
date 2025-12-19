#include <iostream> // cout, endl
#include <vector> // vector
#include <thread> // sleep_for
#include <chrono> // seconds
#include <random> // rand

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "screen_tester.h"

ScreenTester::ScreenTester(const std::vector<std::reference_wrapper<Screen>>& screens) {

    m_screens = screens;
}

void ScreenTester::testDisplay() {

    for (Screen& screen : m_screens) {
        screen.sendCommand(screen::Command::EntireDisplayOn);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (Screen& screen : m_screens) {
        screen.sendCommand(screen::Command::EntireDisplayOff);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (Screen& screen : m_screens) {
        screen.sendCommand(screen::Command::EntireDisplayOn);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (Screen& screen : m_screens) {
        screen.sendCommand(screen::Command::NormalDisplay);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

void ScreenTester::testRemapColorDepth() {

    screen::Color colors[screen::Geometry::Pixels];

    for (Screen& screen : m_screens) {
        screen.setColorDepth(screen::RemapColorDepth::Color256);
        screen.applyRemapColorDepth();
    }

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = screen::ColorLimit::R_565_MAX;
		colors[i].g = 0;
		colors[i].b = 0;
    }

    for (Screen& screen : m_screens) {
        screen.sendMultiPixel(colors, screen::Geometry::Pixels);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (Screen& screen : m_screens) {
        screen.setColorDepth(screen::RemapColorDepth::Color65k);
        screen.applyRemapColorDepth();
    }

    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = 0;
		colors[i].g = screen::ColorLimit::G_565_MAX;
		colors[i].b = 0;
    }

    for (Screen& screen : m_screens) {
        screen.sendMultiPixel(colors, screen::Geometry::Pixels);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    for (Screen& screen : m_screens) {
        screen.setColorDepth(screen::RemapColorDepth::Color65kAlt);
        screen.applyRemapColorDepth();
    }
    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = 0;
		colors[i].g = 0;
		colors[i].b = screen::ColorLimit::B_565_MAX;
    }

    for (Screen& screen : m_screens) {
        screen.sendMultiPixel(colors, screen::Geometry::Pixels);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist31(0, 31);
    std::uniform_int_distribution<int> dist63(0, 63);

    for (Screen& screen : m_screens) {
        screen.setColorDepth(screen::RemapColorDepth::Color256);
        screen.applyRemapColorDepth();
    }
    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = dist31(gen);
        colors[i].g = dist63(gen);
        colors[i].b = dist31(gen);
    }

    for (Screen& screen : m_screens) {
        screen.sendMultiPixel(colors, screen::Geometry::Pixels);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

        for (Screen& screen : m_screens) {
        screen.setColorDepth(screen::RemapColorDepth::Color65k);
        screen.applyRemapColorDepth();
    }
    for (size_t i = 0; i < screen::Geometry::Pixels; i++) {
        colors[i].r = dist31(gen);
        colors[i].g = dist63(gen);
        colors[i].b = dist31(gen);
    }

    for (Screen& screen : m_screens) {
        screen.sendMultiPixel(colors, screen::Geometry::Pixels);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}