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

Test::Test(const std::vector<std::reference_wrapper<Screen>> &screens) {

    m_screens = screens;
}

void Test::display() {

    broadcast([](Screen &s){s.sendCommand(screen::Command::EntireDisplayOn);}, 500ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::EntireDisplayOff);}, 500ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::EntireDisplayOn);}, 500ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::NormalDisplay);}, 500ms);

    broadcast([](Screen &s){s.clearScreen();});
}

void Test::randomPattern() {

    std::vector<screen::Color> colors(screen::Geometry::Pixels);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);

    // Random pixels
    for (auto &c : colors) {
        c = {dist31(gen), dist63(gen), dist31(gen)};
    }
    broadcast([&colors](Screen &s){s.sendMultiPixel(colors);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();});
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

    broadcast([](Screen &s){s.applyDefaultSettings();});
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

    broadcast([](Screen &s){s.applyDefaultSettings();});
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

    broadcast([](Screen &s){s.applyDefaultSettings();});
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
    broadcast([](Screen &s){s.applyDefaultSettings();});
}

void Test::line() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);
    std::uniform_int_distribution<uint8_t> dist95(0, 95);

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

    broadcast([](Screen &s){s.applyDefaultSettings();});
}

void Test::rectangle() {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dist31(0, 31);
    std::uniform_int_distribution<uint8_t> dist63(0, 63);
    std::uniform_int_distribution<uint8_t> dist95(0, 95);

    // Random colors and coordinates for rectangles with no fill
    for (size_t i = 0; i < 20; i++) {
        screen::Color c1 = {dist31(gen), dist63(gen), dist31(gen)};
        screen::Color c2 = {0, 0, 0};
        uint8_t rx1 = dist95(gen);
        uint8_t ry1 = dist63(gen);
        uint8_t rx2 = dist95(gen);
        uint8_t ry2 = dist63(gen);
        uint8_t x1 = std::min(rx1, rx2);
        uint8_t x2 = std::max(rx1, rx2);
        uint8_t y1 = std::min(ry1, ry2);
        uint8_t y2 = std::max(ry1, ry2);
        broadcast([=](Screen &s){s.drawRectangle(x1, y1, x2, y2, c1, c2);}, 200ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.setFillRectangleEnable(true);});
    // Random colors and coordinates for rectangles with fill
    for (size_t i = 0; i < 20; i++) {
        screen::Color c1 = {dist31(gen), dist63(gen), dist31(gen)};
        screen::Color c2 = {dist31(gen), dist63(gen), dist31(gen)};
        uint8_t rx1 = dist95(gen);
        uint8_t ry1 = dist63(gen);
        uint8_t rx2 = dist95(gen);
        uint8_t ry2 = dist63(gen);
        uint8_t x1 = std::min(rx1, rx2);
        uint8_t x2 = std::max(rx1, rx2);
        uint8_t y1 = std::min(ry1, ry2);
        uint8_t y2 = std::max(ry1, ry2);
        broadcast([=](Screen &s){s.drawRectangle(x1, y1, x2, y2, c1, c2);}, 200ms);
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Rectangles with fill that grow from a corner
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

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();});
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

    broadcast([](Screen &s){s.applyDefaultSettings();});
}

void Test::image() {

    const std::string imagePath1 = "/home/petalinux/images/default1.jpg";
    const std::string imagePath2 = "/home/petalinux/images/default2.jpg";

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

    broadcast([](Screen &s){s.applyDefaultSettings();});
}

void Test::symbol() {

    screen::Color color = screen::StandardColor::White;

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

    size_t totalSymbols = 256;
    size_t maxSymbols = screen::TextGeometry::TextChars;

    for (size_t i = 0; i < totalSymbols; i += maxSymbols) {
        size_t end = std::min(i + maxSymbols, totalSymbols);
        for (size_t j = i; j < end; j++) {
            broadcast([=](Screen &s){s.drawSymbol(j, color); s.incrementTextCursor();});
        }
        std::this_thread::sleep_for(5s);
        broadcast([](Screen &s){s.clearScreen();}, 200ms);
        broadcast([](Screen &s){s.setTextCursor(0, 0);});
    }

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();});
}

void Test::string() {

    std::string phrase = "Pmod OLEDrgb";

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

    for (size_t i = 0; i < colors.size(); i++) {
        broadcast([&](Screen &s){s.drawString(phrase, colors[i]);});
    }
    std::this_thread::sleep_for(2s);

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();});
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

    size_t totalColors = colors.size();
    size_t rows = screen::TextGeometry::TextRows;

    for (size_t i = 0; i < totalColors; i += rows) {
        size_t end = std::min(i + rows, totalColors);
        for (size_t j = i; j < end; j++) {
            broadcast([&](Screen &s){s.setTextCursor(0, j - i); s.drawString(phrases[j], colors[j]);});
        }
        std::this_thread::sleep_for(4s);
        broadcast([](Screen &s){s.clearScreen();}, 200ms);
    }

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();});
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

    for (size_t i = 0; i < colors.size() / 2; i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i); s.drawString(phrases[i], colors[i]);});
    }
    std::this_thread::sleep_for(200ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::InverseDisplay);}, 1s);
    for (size_t i = colors.size() / 2; i < colors.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i); s.drawString(phrases[i], colors[i]);});
    }
    std::this_thread::sleep_for(200ms);
    broadcast([](Screen &s){s.sendCommand(screen::Command::NormalDisplay);}, 1s);


    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();});
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
    for (size_t i = 0; i < colors.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i); s.drawString(phrase, colors[i]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});

    phrase = "ColumnNormal";
    broadcast([](Screen &s){s.setColumnRemap(screen::RemapColorDepth::ColumnRemap::Normal); s.applyRemapColorDepth();});
    for (size_t i = 0; i < colors.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i); s.drawString(phrase, colors[i]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    phrase = "BGR";
    broadcast([](Screen &s){s.setColorOrder(screen::RemapColorDepth::ColorOrder::BGR); s.applyRemapColorDepth();});
    for (size_t i = 0; i < colors.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i); s.drawString(phrase, colors[i]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    phrase = "COM Swap";
    broadcast([](Screen &s){s.setCOMSwap(screen::RemapColorDepth::COMSwap::Swap); s.applyRemapColorDepth();});
    for (size_t i = 0; i < colors.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i); s.drawString(phrase, colors[i]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    phrase = "ScanCOM0toN";
    broadcast([](Screen &s){s.setScanDirection(screen::RemapColorDepth::ScanDirection::COM0toN); s.applyRemapColorDepth();});
    for (size_t i = 0; i < colors.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i); s.drawString(phrase, colors[i]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    phrase = "COMSplitDis";
    broadcast([](Screen &s){s.setCOMSplit(screen::RemapColorDepth::COMSplit::Disable); s.applyRemapColorDepth();});
    for (size_t i = 0; i < colors.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i); s.drawString(phrase, colors[i]);});
    }
    std::this_thread::sleep_for(2s);
    broadcast([](Screen &s){s.applyRemapColorDepth(screen::ApplyMode::Default);});
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.clearScreen();}, 200ms);
    broadcast([](Screen &s){s.applyDefaultSettings();});
}

void Test::screenOrientation() {

    screen::Color color = screen::StandardColor::White;

    std::vector<std::string> phrases(3);

    const std::string imagePathHorizontal = "/home/petalinux/images/horizontal.jpg";
    const std::string imagePathVertical = "/home/petalinux/images/vertical.jpg";

    broadcast([](Screen &s){s.setSpiDelay(1ns);});

    // Horizontal orientation
    phrases = {" Horizontal ", "Orientation ", "     0º     "};
    broadcast([](Screen &s){s.setScreenOrientation(screen::Orientation::Horizontal_0);});
    for (size_t i = 0; i < phrases.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i + screen::TextGeometry::TextRows / 2 - phrases.size() / 2); s.drawString(phrases[i], color);});
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([=](Screen &s){s.drawImage(imagePathHorizontal);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Vertical orientation
    phrases = {"Vertical", "Orientat", "  90º   "};
    broadcast([](Screen &s){s.setScreenOrientation(screen::Orientation::Vertical_90);});
    for (size_t i = 0; i < phrases.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i + screen::TextGeometry::TextColumns / 2 - phrases.size() / 2); s.drawString(phrases[i], color);});
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([=](Screen &s){s.drawImage(imagePathVertical);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Horizontal reverse orientation
    phrases = {" Horizontal ", "Orientation ", "    180º    "};
    broadcast([](Screen &s){s.setScreenOrientation(screen::Orientation::Horizontal_180);});
    for (size_t i = 0; i < phrases.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i + screen::TextGeometry::TextRows / 2 - phrases.size() / 2); s.drawString(phrases[i], color);});
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([=](Screen &s){s.drawImage(imagePathHorizontal);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    // Vertical reverse orientation
    phrases = {"Vertical", "Orientat", "  270º  "};
    broadcast([](Screen &s){s.setScreenOrientation(screen::Orientation::Vertical_270);});
    for (size_t i = 0; i < phrases.size(); i++) {
        broadcast([&](Screen &s){s.setTextCursor(0, i + screen::TextGeometry::TextColumns / 2 - phrases.size() / 2); s.drawString(phrases[i], color);});
    }
    std::this_thread::sleep_for(1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([=](Screen &s){s.drawImage(imagePathVertical);}, 1s);
    broadcast([](Screen &s){s.clearScreen();}, 200ms);

    broadcast([](Screen &s){s.applyDefaultSettings();});
}