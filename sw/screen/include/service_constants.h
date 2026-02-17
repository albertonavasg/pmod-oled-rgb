#ifndef SERVICE_CONSTANTS_H
#define SERVICE_CONSTANTS_H

#include <cstdint> // uint

#include "screen_constants.h"
#include "screen.h"

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

    struct ScreenContext {

        std::unique_ptr<Screen> screen;
        std::string id;
        service::ScreenMode mode;
        service::ScreenSubMode subMode;
        bool enteringNewMode;
    };

    struct Date {

        uint16_t year;
        uint8_t month;
        uint8_t day;
    };

    struct Time {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    };

    struct Network {

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

        const screen::Font &font;
        screen::Color color;
    };

    inline const TextBlock dateBlock        { 0, 0,  96,  8, screen::Font8x8, screen::StandardColor::White};
    inline const TextBlock hoursBlock       { 0, 16, 16,  8, screen::Font8x8, screen::StandardColor::White};
    inline const TextBlock firstColonBlock  {16, 16,  8,  8, screen::Font8x8, screen::StandardColor::White};
    inline const TextBlock minutesBlock     {24, 16, 16,  8, screen::Font8x8, screen::StandardColor::White};
    inline const TextBlock secondColonBlock {40, 16,  8,  8, screen::Font8x8, screen::StandardColor::White};
    inline const TextBlock secondsBlock     {48, 16, 16,  8, screen::Font8x8, screen::StandardColor::White};
    inline const TextBlock tickBlock        {40, 16,  8,  8, screen::Font8x8, screen::StandardColor::White};
    inline const TextBlock ipBlock          { 0, 32, 96,  8, screen::Font6x8, screen::StandardColor::White};
    inline const TextBlock maskBlock        { 0, 48, 96,  8, screen::Font6x8, screen::StandardColor::White};
}

#endif // SERVICE_CONSTANTS_H