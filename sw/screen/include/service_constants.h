#ifndef SERVICE_CONSTANTS_H
#define SERVICE_CONSTANTS_H

#include <cstdint> // uint

#include "screen_constants.h"

namespace service {

    enum class ScreenMode {

        None,
        Info,
        DigitalClock,
        AnalogClock
    };

    struct TextBlock {

        uint8_t x;
        uint8_t y;
        uint8_t width;
        uint8_t height;

        const screen::Font *font;
        screen::Color color;
    };
}

#endif // SERVICE_CONSTANTS_H