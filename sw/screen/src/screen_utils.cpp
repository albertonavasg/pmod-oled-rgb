#include <iostream> // cout, endl
#include <cstdint>  // uint32_t
#include <vector>   // vector

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

void Screen::sendPixel(const screen::Color color) {

    uint32_t data = 0;
	uint8_t byte[3] = {0};
    uint8_t colorDepth =
        (m_remapColorDepthCfg & screen::RemapColorDepth::ColorDepth_Msk)
        >> screen::RemapColorDepth::ColorDepth_Pos;

    switch (static_cast<screen::RemapColorDepth::ColorDepth>(colorDepth)) {
        case screen::RemapColorDepth::ColorDepth::Color256: {
            byte[0] = (uint8_t)((color.r>>2) << 5 | (color.g>>3) << 2 | (color.b>>3));
			sendData(byte[0]);
            break;
        }
        case screen::RemapColorDepth::ColorDepth::Color65k: {
 			data = ((color.r) << 11 | (color.g) << 5 | (color.b));
			byte[0] = (uint8_t)(data >> 8);
			byte[1] = (uint8_t)(data & 0x000000FF);
			sendMultiData(byte, 2);
            break;
        }
        case screen::RemapColorDepth::ColorDepth::Color65kAlt: {
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

void Screen::sendMultiPixel(const std::vector<screen::Color> &colors) {

    for (const auto &c : colors) {
        sendPixel(c);
    }
}