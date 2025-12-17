#include <vector> // vector
#include <thread> // sleep_for
#include <chrono> // seconds

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "screen_tester.h"

ScreenTester::ScreenTester(const std::vector<std::reference_wrapper<Screen>>& screens) {

    m_screens = screens;
}

void ScreenTester::testDisplay() {

    for (Screen& screen : m_screens) {
        screen.setPowerState(true);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (Screen& screen : m_screens) {
        screen.sendCommand(screen::Command::EntireDisplayOn);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (Screen& screen : m_screens) {
        screen.sendCommand(screen::Command::EntireDisplayOff);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (Screen& screen : m_screens) {
        screen.sendCommand(screen::Command::EntireDisplayOn);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (Screen& screen : m_screens) {
        screen.sendCommand(screen::Command::NormalDisplay);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    for (Screen& screen : m_screens) {
        screen.setPowerState(false);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
}