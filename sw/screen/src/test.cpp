#include <iostream>   // cout, endl
#include <vector>     // vector
#include <thread>     // sleep_for
#include <chrono>     // time
#include <random>     // rand
#include <functional> // reference_wrapper

#include "paths.h"
#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "test.h"

using namespace std::chrono_literals;

Test::Test(const std::vector<std::reference_wrapper<Screen>> &screens) {

    m_screens = screens;
}

void Test::full() {

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);

    display();
    randomPattern();
    colorDepth();
    addressIncrement();
    bitmap();
    scrolling();
    line();
    rectangle();
    circle();
    clear();
    copy();
    image();
    symbol();
    string();
    standardColors();
    inverseDisplay();
    remap();
    screenOrientation();

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::display() {

    broadcast([](Screen &s){s.sendCommand(screen::Command::EntireDisplayOn);}, 500ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::EntireDisplayOff);}, 500ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::EntireDisplayOn);}, 500ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::NormalDisplay);}, 500ms);

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::randomPattern() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);

    // Random pixels
    for (screen::Color &c : colors) {
        c = {dist31(gen), dist63(gen), dist31(gen)};
    }
    broadcast([&colors](Screen &s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
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

    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color256); s.applyRemapColorDepth();});
    broadcast([&colors](Screen &s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color65k); s.applyRemapColorDepth();});
    broadcast([&colors](Screen &s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color65kAlt); s.applyRemapColorDepth();});
    broadcast([&colors](Screen &s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::addressIncrement() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

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
    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color65kAlt); s.setAddressIncrement(screen::RemapColorDepth::AddressIncrement::Horizontal); s.applyRemapColorDepth();});
    broadcast([&colors](Screen &s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

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
    broadcast([](Screen &s){s.setAddressIncrement(screen::RemapColorDepth::AddressIncrement::Vertical); s.applyRemapColorDepth();});
    broadcast([&colors](Screen &s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::bitmap() {

    size_t size = screen::Geometry::Pixels / 4;

    size_t c1 = (screen::Geometry::Columns / 4);
    size_t r1 = (screen::Geometry::Rows / 4);
    size_t c2 = ((screen::Geometry::Columns / 4) * 3) - 1;
    size_t r2 = ((screen::Geometry::Rows / 4) * 3) - 1;

    std::vector<screen::Color> colors(size);

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

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
    broadcast([&colors, c1, r1, c2, r2](Screen &s){s.drawBitmap(c1, r1, c2, r2, colors);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::scrolling() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);
    size_t squareDimension = 16;

    // Squares
    for (size_t i = 0; i < colors.size(); i++) {
        const size_t xBlock = (i % screen::Geometry::Columns) / squareDimension;
        const size_t yBlock = (i / screen::Geometry::Columns) / squareDimension;
        if ((xBlock ^ yBlock)  &1) {
            colors[i] = screen::StandardColor::Violet;
        } else {
            colors[i] = screen::StandardColor::Cyan;
        }
    }
    broadcast([&colors](Screen &s){s.sendMultiPixel(colors);}, 1s);

    // Horizontal right scroll
    broadcast([&colors](Screen &s){s.setupScrolling(1, 0, screen::Geometry::Rows, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    // Horizontal right fast scroll
    broadcast([&colors](Screen &s){s.setupScrolling(4, 0, screen::Geometry::Rows, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    // Horizontal left scroll
    broadcast([&colors](Screen &s){s.setupScrolling(95, 0, screen::Geometry::Rows, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    // Horizontal left fast scroll
    broadcast([&colors](Screen &s){s.setupScrolling(92, 0, screen::Geometry::Rows, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);

    // Horizontal partial right scroll
    broadcast([&colors](Screen &s){s.setupScrolling(1, screen::Geometry::Rows/4, screen::Geometry::Rows/2, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    broadcast([&colors](Screen &s){s.setupScrolling(63, screen::Geometry::Rows/4, screen::Geometry::Rows/2, 0, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);

    // Vertical up scroll
    broadcast([&colors](Screen &s){s.setupScrolling(1, 0, 0, 1, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    // Vertical up fast scroll
    broadcast([&colors](Screen &s){s.setupScrolling(1, 0, 0, 4, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    // Vertical down scroll
    broadcast([&colors](Screen &s){s.setupScrolling(1, 0, 0, 63, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    // Vertical down fast scroll
    broadcast([&colors](Screen &s){s.setupScrolling(1, 0, 0, 60, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);

    // Diagonal up right scroll
    broadcast([&colors](Screen &s){s.setupScrolling(1, 0, screen::Geometry::Rows, 1, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    // Diagonal up left scroll
    broadcast([&colors](Screen &s){s.setupScrolling(95, 0, screen::Geometry::Rows, 1, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    // Diagonal down right scroll
    broadcast([&colors](Screen &s){s.setupScrolling(1, 0, screen::Geometry::Rows, 63, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);
    // Diagonal down left scroll
    broadcast([&colors](Screen &s){s.setupScrolling(95, 0, screen::Geometry::Rows, 63, 0b00); s.enableScrolling(true);}, 1s);
    broadcast([&colors](Screen &s){s.enableScrolling(false);}, 200ms);

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::line() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);
    std::uniform_int_distribution<uint8_t> dist95(0, 95);

    // Four lines to test orientation
    broadcast([](Screen &s){s.drawLine(0, 0, 95, 63, screen::StandardColor::White);}, 500ms);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.drawLine(95, 63, 0, 0, screen::StandardColor::White);}, 500ms);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.drawLine(95, 0, 0, 63, screen::StandardColor::White);}, 500ms);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.drawLine(0, 63, 95, 0, screen::StandardColor::White);}, 500ms);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Three horizontal lines to check color order
    broadcast([](Screen &s){s.drawLine(10, 10, 90, 10, screen::StandardColor::Red);}, 200ms);
    broadcast([](Screen &s){s.drawLine(10, 30, 90, 30, screen::StandardColor::Green);}, 200ms);
    broadcast([](Screen &s){s.drawLine(10, 50, 90, 50, screen::StandardColor::Blue);}, 200ms);
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Random colors and coordinates for horizontal lines
    for (size_t i = 0; i < 20; i++) {
        screen::Color c = {dist31(gen), dist63(gen), dist31(gen)};
        uint8_t x1 = 0;
        uint8_t y1 = dist63(gen);
        uint8_t x2 = screen::Geometry::Columns - 1;
        uint8_t y2 = y1;
        broadcast([=](Screen &s){s.drawLine(x1, y1, x2, y2, c);}, 200ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Random colors and coordinates for vertical lines
    for (size_t i = 0; i < 20; i++) {
        screen::Color c = {dist31(gen), dist63(gen), dist31(gen)};
        uint8_t x1 = dist95(gen);
        uint8_t y1 = 0;
        uint8_t x2 = x1;
        uint8_t y2 = screen::Geometry::Rows - 1;
        broadcast([=](Screen &s){s.drawLine(x1, y1, x2, y2, c);}, 200ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Random colors and coordinates for lines
    for (size_t i = 0; i < 20; i++) {
        screen::Color c = {dist31(gen), dist63(gen), dist31(gen)};
        uint8_t x1 = dist95(gen);
        uint8_t y1 = dist63(gen);
        uint8_t x2 = dist95(gen);
        uint8_t y2 = dist63(gen);
        broadcast([=](Screen &s){s.drawLine(x1, y1, x2, y2, c);}, 200ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::rectangle() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);
    std::uniform_int_distribution<uint8_t> dist95(0, 95);

    // Four rectangles to test orientation
    broadcast([](Screen &s){s.drawRectangle(0, 0, 95, 63, screen::StandardColor::White, screen::StandardColor::Black);}, 500ms);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.drawRectangle(95, 63, 0, 0, screen::StandardColor::White, screen::StandardColor::Black);}, 500ms);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.drawRectangle(95, 0, 0, 63, screen::StandardColor::White, screen::StandardColor::Black);}, 500ms);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.drawRectangle(0, 63, 95, 0, screen::StandardColor::White, screen::StandardColor::Black);}, 500ms);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Three rectangles withn no fill to check color order
    broadcast([](Screen &s){s.setFillRectangleEnable(false);});
    broadcast([](Screen &s){s.drawRectangle(5, 5, 30, 60, screen::StandardColor::Red, screen::StandardColor::Black);}, 200ms);
    broadcast([](Screen &s){s.drawRectangle(35, 5, 60, 60, screen::StandardColor::Green, screen::StandardColor::Black);}, 200ms);
    broadcast([](Screen &s){s.drawRectangle(65, 5, 90, 60, screen::StandardColor::Blue, screen::StandardColor::Black);}, 200ms);
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Random colors and coordinates for rectangles with no fill
    broadcast([](Screen &s){s.setFillRectangleEnable(false);});
    for (size_t i = 0; i < 20; i++) {
        screen::Color c1 = {dist31(gen), dist63(gen), dist31(gen)};
        screen::Color c2 = {0, 0, 0};
        uint8_t x1 = dist95(gen);
        uint8_t y1 = dist63(gen);
        uint8_t x2 = dist95(gen);
        uint8_t y2 = dist63(gen);
        broadcast([=](Screen &s){s.drawRectangle(x1, y1, x2, y2, c1, c2);}, 200ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Random colors and coordinates for rectangles with fill
    broadcast([](Screen &s){s.setFillRectangleEnable(true);});
    for (size_t i = 0; i < 20; i++) {
        screen::Color c1 = {dist31(gen), dist63(gen), dist31(gen)};
        screen::Color c2 = {dist31(gen), dist63(gen), dist31(gen)};
        uint8_t x1 = dist95(gen);
        uint8_t y1 = dist63(gen);
        uint8_t x2 = dist95(gen);
        uint8_t y2 = dist63(gen);
        broadcast([=](Screen &s){s.drawRectangle(x1, y1, x2, y2, c1, c2);}, 200ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Rectangles with fill that grow from a corner
    broadcast([](Screen &s){s.setFillRectangleEnable(true);});
    for (size_t i = 0; i < screen::Geometry::Rows; i++) {
        screen::Color c1 = {dist31(gen), dist63(gen), dist31(gen)};
        screen::Color c2 = {dist31(gen), dist63(gen), dist31(gen)};
        uint8_t x1 = 0;
        uint8_t y1 = 0;
        uint8_t x2 = static_cast<uint8_t>((i * screen::Geometry::Columns) / screen::Geometry::Rows);
        uint8_t y2 = i;
        broadcast([=](Screen &s){s.drawRectangle(x1, y1, x2, y2, c1, c2);}, 200ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::circle() {

    // Small circle to test algorithm
    // broadcast([](Screen &s){s.drawCircle(20, 20, 10, screen::StandardColor::White);}, 200ms);
    // broadcast([](Screen &s){s.drawCircle(20, 20, 20, screen::StandardColor::White);}, 200ms);
    // broadcast([](Screen &s){s.drawCircle(20, 20, 30, screen::StandardColor::White);}, 200ms);
    // std::this_thread::sleep_for(1s);
    // broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // RGB circles
    broadcast([](Screen &s){s.drawCircle(0, 15, 32, screen::StandardColor::Red);}, 200ms);
    broadcast([](Screen &s){s.drawCircle(32, 15, 32, screen::StandardColor::Green);}, 200ms);
    broadcast([](Screen &s){s.drawCircle(64, 15, 32, screen::StandardColor::Blue);}, 200ms);
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    uint8_t x_coord = (screen::Geometry::Columns / 2) - (screen::Geometry::Rows / 2);
    uint8_t y_coord = 0;
    uint8_t d = screen::Geometry::Rows;

    // Big circle
    broadcast([&](Screen &s){s.drawCircle(x_coord, y_coord, d, screen::StandardColor::White);}, 500ms);
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Centered circles with increasing size
    for (int d = 2; d <= screen::Geometry::Rows; d += 2) {
        uint8_t x = (screen::Geometry::Columns / 2) - (d / 2);
        uint8_t y = (screen::Geometry::Rows / 2) - (d / 2);
        broadcast([&](Screen &s){s.drawCircle(x, y, d, screen::StandardColor::White);}, 200ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::clear() {

    std::vector<screen::Color> bitmap(50*50, screen::StandardColor::White);

    broadcast([&](Screen &s){s.drawBitmap(10,10,59,59, bitmap);}, 500ms);
    broadcast([](Screen &s){s.clearWindow(10,10,59,59);}, 200ms);

    broadcast([&](Screen &s){s.drawBitmap(10,10,59,59, bitmap);}, 500ms);
    broadcast([](Screen &s){s.clearWindow(59,59,10,10);}, 200ms);

    broadcast([&](Screen &s){s.drawBitmap(10,10,59,59, bitmap);}, 500ms);
    broadcast([](Screen &s){s.clearWindow(10,59,59,10);}, 200ms);

    broadcast([&](Screen &s){s.drawBitmap(10,10,59,59, bitmap);}, 500ms);
    broadcast([](Screen &s){s.clearWindow(59,10,10,59);}, 200ms);

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::copy() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);

    screen::Color cLine = {dist31(gen), dist63(gen), dist31(gen)};
    screen::Color cFill = {dist31(gen), dist63(gen), dist31(gen)};
    uint8_t x1 = 15;
    uint8_t y1 = 5;
    uint8_t x2 = 35;
    uint8_t y2 = 25;

    // Rectangles without fill, to show the overlap when copying
    broadcast([=](Screen &s){s.drawRectangle(x1, y1, x2, y2, cLine, cFill);}, 500ms);

    broadcast([=](Screen &s){s.copyWindow(x1, y1, x2, y2, x1 + 5, y1 + 5);}, 500ms);
    broadcast([=](Screen &s){s.copyWindow(x1, y1, x2 + 5, y2 + 5, x1, y1 + 30);}, 500ms);
    broadcast([=](Screen &s){s.copyWindow(x1, y1, x2 + 5, y2 + 35, x1 + 40, y1);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Rectangles with fill, to show reverseCopy of colors
    broadcast([=](Screen &s){s.setFillRectangleEnable(true);});
    broadcast([=](Screen &s){s.setReverseCopyEnable(true);});
    broadcast([=](Screen &s){s.drawRectangle(x1, y1, x2, y2, cLine, cFill);}, 500ms);

    broadcast([=](Screen &s){s.copyWindow(x1, y1, x2, y2, x1 + 5, y1 + 5);}, 500ms);
    broadcast([=](Screen &s){s.copyWindow(x1, y1, x2 + 5, y2 + 5, x1, y1 + 30);}, 500ms);
    broadcast([=](Screen &s){s.copyWindow(x1, y1, x2 + 5, y2 + 35, x1 + 40, y1);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::image() {

    const std::string imagePath1 = AppPaths::IMAGES_DIR + "default1.jpg";
    const std::string imagePath2 = AppPaths::IMAGES_DIR + "default2.jpg";

    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color256); s.applyRemapColorDepth();});
    broadcast([=](Screen &s){s.drawImage(imagePath1);}, 2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color65k); s.applyRemapColorDepth();});
    broadcast([=](Screen &s){s.drawImage(imagePath1);}, 2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color65kAlt); s.applyRemapColorDepth();});
    broadcast([=](Screen &s){s.drawImage(imagePath1);}, 2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color256); s.applyRemapColorDepth();});
    broadcast([=](Screen &s){s.drawImage(imagePath2);}, 2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color65k); s.applyRemapColorDepth();});
    broadcast([=](Screen &s){s.drawImage(imagePath2);}, 2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.setColorDepth(screen::RemapColorDepth::ColorDepth::Color65kAlt); s.applyRemapColorDepth();});
    broadcast([=](Screen &s){s.drawImage(imagePath2);}, 2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::symbol() {

    size_t totalSymbols = 256;

    const screen::Font font1 = screen::Font8x8;
    const screen::Font font2 = screen::Font6x8;

    screen::Color color = screen::StandardColor::White;

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

    size_t textRows = screen::Geometry::Rows / font1.height;
    size_t textCols = screen::Geometry::Columns / font1.width;
    size_t symbolsPerScreen = textRows * textCols;

    for (size_t i = 0; i < totalSymbols; i += symbolsPerScreen) {
        size_t end = std::min(i + symbolsPerScreen, totalSymbols);
        for (size_t j = i; j < end; j++) {
            size_t localIndex = j - i;
            size_t row = localIndex / textCols;
            size_t col = localIndex % textCols;
            uint16_t x = col * font1.width;
            uint16_t y = row * font1.height;
            broadcast([=](Screen &s){s.drawSymbol(static_cast<uint8_t>(j), x, y, font1, color);});
        }
        std::this_thread::sleep_for(5s);
        broadcast([](Screen &s){ s.clearScreen(); }, 200ms);
    }
    broadcast([](Screen &s){ s.clearScreen(); }, 200ms);

    textRows = screen::Geometry::Rows / font2.height;
    textCols = screen::Geometry::Columns / font2.width;
    symbolsPerScreen = textRows * textCols;

    for (size_t i = 0; i < totalSymbols; i += symbolsPerScreen) {
        size_t end = std::min(i + symbolsPerScreen, totalSymbols);
        for (size_t j = i; j < end; j++) {
            size_t localIndex = j - i;
            size_t row = localIndex / textCols;
            size_t col = localIndex % textCols;
            uint16_t x = col * font2.width;
            uint16_t y = row * font2.height;
            broadcast([=](Screen &s){s.drawSymbol(static_cast<uint8_t>(j), x, y, font2, color);});
        }
        std::this_thread::sleep_for(5s);
        broadcast([](Screen &s){ s.clearScreen(); }, 200ms);
    }
    broadcast([](Screen &s){ s.clearScreen(); }, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::string() {

    std::string phrase = "Pmod OLEDrgb";

    const screen::Font font1 = screen::Font8x8;
    const screen::Font font2 = screen::Font6x8;

    std::vector<screen::Color> colors = {
        screen::StandardColor::White,
        screen::StandardColor::Red,
        screen::StandardColor::Green,
        screen::StandardColor::Blue,
        screen::StandardColor::Yellow,
        screen::StandardColor::Violet,
        screen::StandardColor::Cyan,
        screen::StandardColor::White,
    };

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

    // Font8x8
    for (size_t row = 0; row < colors.size(); row++) {
        broadcast([&](Screen &s){s.drawString(phrase, 0, row * font1.height, font1, colors[row]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Font6x8
    for (size_t row = 0; row < colors.size(); row++) {
        broadcast([&](Screen &s){s.drawString(phrase, 0, row * font2.height, font2, colors[row]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Strings that do not fit
    broadcast([&](Screen &s){s.drawString("This does not fit", 0, 0, font1, colors[0]);});
    broadcast([&](Screen &s){s.drawString("This does not fit", 0, 10, font2, colors[0]);});
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::standardColors() {

    std::vector<screen::Color> colors = {
        screen::StandardColor::White,

        screen::StandardColor::Red,
        screen::StandardColor::Green,
        screen::StandardColor::Blue,

        screen::StandardColor::Yellow,
        screen::StandardColor::Violet,
        screen::StandardColor::Cyan,

        screen::StandardColor::Grey,

        screen::StandardColor::Orange,
        screen::StandardColor::Lime,
        screen::StandardColor::Olive,

        screen::StandardColor::Pink,
        screen::StandardColor::Purple,
        screen::StandardColor::DarkViolet,

        screen::StandardColor::LightGreen,
        screen::StandardColor::SkyBlue,
        screen::StandardColor::Teal
    };

    std::vector<std::string> phrases = {
        "White",

        "Red",
        "Green",
        "Blue",

        "Yellow",
        "Violet",
        "Cyan",

        "Grey",

        "Orange",
        "Lime",
        "Olive",

        "Pink",
        "Purple",
        "DarkViolet",

        "LightGreen",
        "SkyBlue",
        "Teal",
    };

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

    size_t totalColors = colors.size();
    size_t rowsPerScreen = screen::Geometry::Rows / screen::Font8x8.height;

    for (size_t i = 0; i < totalColors; i += rowsPerScreen) {
        size_t end = std::min(i + rowsPerScreen, totalColors);
        for (size_t j = i; j < end; j++) {
            size_t row = j - i;
            size_t x = 0;
            size_t y = row * screen::Font8x8.height;
            broadcast([&](Screen &s){s.drawString(phrases[j], x, y, screen::Font8x8, colors[j]);});
        }
        std::this_thread::sleep_for(5s);
        broadcast([](Screen &s){s.clearScreen();}, 200ms);
    }

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::inverseDisplay() {

    std::vector<screen::Color> colors = {
        screen::StandardColor::White,
        screen::StandardColor::Red,
        screen::StandardColor::Green,
        screen::StandardColor::Blue,
        screen::StandardColor::Yellow,
        screen::StandardColor::Violet,
        screen::StandardColor::Cyan,
        screen::StandardColor::White,
    };

    std::vector<std::string> phrases = {
        "Inverting",
        "Display",
        "In 3 2 1 ...",
        "",
        "",
        "Going back",
        "To normal",
        "In 3 2 1 ..."
    };

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

    for (size_t row = 0; row < colors.size() / 2; row++) {
        uint8_t x = 0;
        uint8_t y = row * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrases[row], x, y, screen::Font8x8, colors[row]);});
    }
    std::this_thread::sleep_for(200ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::InverseDisplay);}, 1s);

    for (size_t row = colors.size() / 2; row < colors.size(); row++) {
        uint8_t x = 0;
        uint8_t y = row * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrases[row], x, y, screen::Font8x8, colors[row]);});
    }
    std::this_thread::sleep_for(200ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::NormalDisplay);}, 1s);

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::remap() {

    std::vector<screen::Color> colors = {
        screen::StandardColor::White,
        screen::StandardColor::Red,
        screen::StandardColor::Green,
        screen::StandardColor::Blue,
        screen::StandardColor::Yellow,
        screen::StandardColor::Violet,
        screen::StandardColor::Cyan,
        screen::StandardColor::White,
    };

    std::string phrase;

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

    phrase = "Default";
    for (size_t row = 0; row < colors.size(); row++) {
        uint8_t x = 0;
        uint8_t y = row * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrase, x, y, screen::Font8x8, colors[row]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});

    phrase = "ColumnNormal";
    broadcast([](Screen &s){s.setColumnRemap(screen::RemapColorDepth::ColumnRemap::Normal); s.applyRemapColorDepth();});
    for (size_t row = 0; row < colors.size(); row++) {
        uint8_t x = 0;
        uint8_t y = row * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrase, x, y, screen::Font8x8, colors[row]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    phrase = "BGR";
    broadcast([](Screen &s){s.setColorOrder(screen::RemapColorDepth::ColorOrder::BGR); s.applyRemapColorDepth();});
    for (size_t row = 0; row < colors.size(); row++) {
        uint8_t x = 0;
        uint8_t y = row * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrase, x, y, screen::Font8x8, colors[row]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    phrase = "COM Swap";
    broadcast([](Screen &s){s.setCOMSwap(screen::RemapColorDepth::COMSwap::Swap); s.applyRemapColorDepth();});
    for (size_t row = 0; row < colors.size(); row++) {
        uint8_t x = 0;
        uint8_t y = row * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrase, x, y, screen::Font8x8, colors[row]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    phrase = "ScanCOM0toN";
    broadcast([](Screen &s){s.setScanDirection(screen::RemapColorDepth::ScanDirection::COM0toN); s.applyRemapColorDepth();});
    for (size_t row = 0; row < colors.size(); row++) {
        uint8_t x = 0;
        uint8_t y = row * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrase, x, y, screen::Font8x8, colors[row]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    phrase = "COMSplitDis";
    broadcast([](Screen &s){s.setCOMSplit(screen::RemapColorDepth::COMSplit::Disable); s.applyRemapColorDepth();});
    for (size_t row = 0; row < colors.size(); row++) {
        uint8_t x = 0;
        uint8_t y = row * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrase, x, y, screen::Font8x8, colors[row]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}

void Test::screenOrientation() {

    screen::Color color = screen::StandardColor::White;

    std::vector<std::string> phrases(3);

    const std::string imagePathHorizontal = AppPaths::IMAGES_DIR + "horizontal.jpg";
    const std::string imagePathVertical = AppPaths::IMAGES_DIR + "vertical.jpg";

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

    // Horizontal 0 orientation
    phrases = {" Horizontal ", "Orientation ", "     0º     "};
    broadcast([](Screen &s){s.setScreenOrientation(screen::Orientation::Horizontal_0);});
    for (size_t row = 0; row < phrases.size(); row++) {
        uint8_t x = 0;
        uint8_t y = (2 + row) * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrases[row], x, y, screen::Font8x8, color);});
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([=](Screen &s){s.drawImage(imagePathHorizontal);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Vertical 90 orientation
    phrases = {"Vertical", "Orientat", "   90º  "};
    broadcast([](Screen &s){s.setScreenOrientation(screen::Orientation::Vertical_90);});
    for (size_t row = 0; row < phrases.size(); row++) {
        uint8_t x = 0;
        uint8_t y = (4 + row) * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrases[row], x, y, screen::Font8x8, color);});
    }
    std::this_thread::sleep_for(5s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([=](Screen &s){s.drawImage(imagePathVertical);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Horizontal 180 orientation
    phrases = {" Horizontal ", "Orientation ", "    180º    "};
    broadcast([](Screen &s){s.setScreenOrientation(screen::Orientation::Horizontal_180);});
    for (size_t row = 0; row < phrases.size(); row++) {
        uint8_t x = 0;
        uint8_t y = (2 + row) * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrases[row], x, y, screen::Font8x8, color);});
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([=](Screen &s){s.drawImage(imagePathHorizontal);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Vertical 270 orientation
    phrases = {"Vertical", "Orientat", "  270º  "};
    broadcast([](Screen &s){s.setScreenOrientation(screen::Orientation::Vertical_270);});
    for (size_t row = 0; row < phrases.size(); row++) {
        uint8_t x = 0;
        uint8_t y = (4 + row) * screen::Font8x8.height;
        broadcast([&](Screen &s){s.drawString(phrases[row], x, y, screen::Font8x8, color);});
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([=](Screen &s){s.drawImage(imagePathVertical);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();}, 100ms);
}