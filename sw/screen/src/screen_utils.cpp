#include <iostream> // cout, endl
#include <cstdint>  // uint32_t
#include <vector>   // vector

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

void Screen::sendPixel(const screen::Color color) {

    uint32_t data = 0;
	uint8_t byte[3] = {0};
    uint8_t colorDepth = remapColorDepthCfg & screen::RemapColorDepthMask::ColorDepth;

    switch (colorDepth) {
        case screen::RemapColorDepth::Color256: {
            byte[0] = (uint8_t)((color.r>>2) << 5 | (color.g>>3) << 2 | (color.b>>3));
			sendData(byte[0]);
            break;
        }
        case screen::RemapColorDepth::Color65k: {
 			data = ((color.r) << 11 | (color.g) << 5 | (color.b));
			byte[0] = (uint8_t)(data >> 8);
			byte[1] = (uint8_t)(data & 0x000000FF);
			sendMultiData(byte, 2);
            break;
        }
        case screen::RemapColorDepth::Color65kAlt: {
			data = ((color.r) << 17 | (color.g) << 8 | (color.b) << 1);
			byte[0] = (uint8_t)(data >> 16);
			byte[1] = (uint8_t)((data & 0x0000FF00) >> 8);
			byte[2] = (uint8_t)(data & 0x000000FF);
			sendMultiData(byte, 3);
            break;
        }
        default:
            break;
    }
}

void Screen::sendMultiPixel(const std::vector<screen::Color>& colors) {

    for (const auto& c : colors) {
        sendPixel(c);
    }
}

void Screen::clearWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2) {

    uint8_t params[4] = {c1, r1, c2, r2};
    sendCommand(screen::Command::ClearWindow, params, 4);
}

void Screen::clearScreen() {

    clearWindow(0, 0, screen::Geometry::Columns - 1, screen::Geometry::Rows - 1);
}

 void Screen::drawBitmap(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const std::vector<screen::Color>& colors) {

    setColumnRowAddr(c1, r1, c2, r2);
    applyColumnRowAddr();
    sendMultiPixel(colors);
 }

void Screen::setupScrolling(uint8_t horizontalScrollOffset, uint8_t startRow, uint8_t rowsNumber, uint8_t verticalScrollOffset, uint8_t timeInterval) {

    uint8_t params[5] = {horizontalScrollOffset, startRow, rowsNumber, verticalScrollOffset, timeInterval};
    sendCommand(screen::Command::ContinuousScrolling, params, 5);
}

void Screen::enableScrolling(bool value) {

    sendCommand(value ? screen::Command::ActivateScroll : screen::Command::DeactivateScroll);
}