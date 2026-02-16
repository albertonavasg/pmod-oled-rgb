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

    enum class ScreenSubMode {

        None,
        HourMinute,
        HourMinuteSecond,
        HourMinuteTick

    };

    struct TimeData {

        uint16_t year;
        uint8_t month;
        uint8_t day;
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    };

    struct NetworkData {

        bool interfaceUp;
        bool hasCarrier;
        bool isIPv4;
        uint32_t ip;
        uint32_t netmask;
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