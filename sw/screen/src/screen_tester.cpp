#include <vector> // vector
#include <thread> // sleep_for
#include <chrono> // seconds

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "screen_tester.h"

ScreenTester::ScreenTester(std::vector<Screen*>& screens) {

    m_screens = screens;
}

void ScreenTester::testDisplay() {

    for (auto* screen : m_screens) {
        if (!screen) continue;  // Skip null pointers
        screen->setPowerState(true);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (auto* screen : m_screens) {
        if (!screen) continue;
        screen->sendCommand(screen::Command::EntireDisplayOn);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (auto* screen : m_screens) {
        if (!screen) continue;
        screen->sendCommand(screen::Command::NormalDisplay);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (auto* screen : m_screens) {
        if (!screen) continue;
        screen->setPowerState(false);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
}