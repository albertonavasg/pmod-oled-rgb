#ifndef SCREEN_CONSTANTS_H
#define SCREEN_CONSTANTS_H

#include <cstdint>

namespace screen {

    enum class PowerState : uint8_t {

        Off = 0b00,
        TurningOn = 0b01,
        TurningOff = 0b10,
        On = 0b11
    };

    enum class DataMode : uint8_t {

        Command = 0,
        Data    = 1
    };

    enum class Command : uint8_t {

        ColumnAddress         = 0x15,
        RowAddress            = 0x75,
        ContrastA             = 0x81,
        ContrastB             = 0x82,
        ContrastC             = 0x83,
        MasterCurrentControl  = 0x87,
        SecondPrechargeSpeedA = 0x8A,
        SecondPrechargeSpeedB = 0x8B,
        SecondPrechargeSpeedC = 0x8C,
        RemapColorDepth       = 0xA0,
        DisplayStartLine      = 0xA1,
        DisplayOffset         = 0xA2,
        NormalDisplay         = 0xA4,
        EntireDisplayOn       = 0xA5,
        EntireDisplayOff      = 0xA6,
        InverseDisplay        = 0xA7,
        MuxRatio              = 0xA8,
        DimMode               = 0xAB,
        MasterConfiguration   = 0xAD,
        DisplayOnDimMode      = 0xAC,
        DisplayOffSleepMode   = 0xAE,
        DIsplayOnNormalMode   = 0xAF,
        PowerSaveMode         = 0xB0,
        PhasePeriodAdjustment = 0xB1,
        DisplayClockDiv       = 0xB3,
        GrayScaleTable        = 0xB8,
        EnableLinearGrayScale = 0xB9,
        PreChargeLevel        = 0xBB,
        VCOMH                 = 0xBE,
        CommandLock           = 0xFD,
        DrawLine              = 0x21,
        DrawRectangle         = 0x22,
        Copy                  = 0x23,
        DimWindow             = 0x24,
        ClearWindow           = 0x25,
        FillEnable            = 0x26,
        ContinuousScrolling   = 0x27,
        DeactivateScroll      = 0x2E,
        ActivateScroll        = 0x2F,
    };

    namespace Geometry {

        constexpr uint8_t Rows    = 64;
        constexpr uint8_t Columns = 96;
        constexpr uint16_t Pixels  = static_cast<uint16_t>(Rows) * static_cast<uint16_t>(Columns);
    }

    enum class Orientation : uint8_t {

        Horizontal_0,
        Vertical_90,
        Horizontal_180,
        Vertical_270
    };

    constexpr Orientation defaultOrientation = Orientation::Horizontal_0;

    constexpr uint8_t FontHeight = 8;
    constexpr uint8_t FontWidth = 8;
    constexpr uint16_t FontPixels = static_cast<uint16_t>(FontHeight) * static_cast<uint16_t>(FontWidth);

    namespace TextGeometry {

        constexpr uint8_t TextRows    = Geometry::Rows / FontHeight;
        constexpr uint8_t TextColumns = Geometry::Columns / FontWidth;
        constexpr uint16_t TextPixels = static_cast<uint16_t>(TextRows) * static_cast<uint16_t>(TextColumns);
    }

    struct TextCursor {

        uint8_t x;
        uint8_t y;
    };

    constexpr TextCursor defaultTextCursor = {0, 0};

    struct Color {

        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    namespace ColorLimit {

        constexpr uint8_t R_565_MAX = 31; // 5 bits
        constexpr uint8_t G_565_MAX = 63; // 6 bits
        constexpr uint8_t B_565_MAX = 31; // 5 bits
    };

    namespace StandardColor {

        constexpr Color White = {screen::ColorLimit::R_565_MAX, screen::ColorLimit::G_565_MAX, screen::ColorLimit::B_565_MAX};

        constexpr Color Red   = {screen::ColorLimit::R_565_MAX, 0, 0};
        constexpr Color Green = {0, screen::ColorLimit::G_565_MAX, 0};
        constexpr Color Blue  = {0, 0, screen::ColorLimit::B_565_MAX};

        constexpr Color Yellow = {screen::ColorLimit::R_565_MAX, screen::ColorLimit::G_565_MAX, 0};
        constexpr Color Violet = {screen::ColorLimit::R_565_MAX, 0, screen::ColorLimit::B_565_MAX};
        constexpr Color Cyan   = {0, screen::ColorLimit::G_565_MAX, screen::ColorLimit::B_565_MAX};

        constexpr Color Grey = {screen::ColorLimit::R_565_MAX / 2, screen::ColorLimit::G_565_MAX / 2, screen::ColorLimit::B_565_MAX / 2};

        constexpr Color Orange = {screen::ColorLimit::R_565_MAX, screen::ColorLimit::G_565_MAX / 2, 0};
        constexpr Color Lime   = {screen::ColorLimit::R_565_MAX / 2, screen::ColorLimit::G_565_MAX, 0};
        constexpr Color Olive  = {screen::ColorLimit::R_565_MAX / 2, screen::ColorLimit::G_565_MAX / 2, 0};

        constexpr Color Pink       = {screen::ColorLimit::R_565_MAX, 0, screen::ColorLimit::B_565_MAX / 2};
        constexpr Color Purple     = {screen::ColorLimit::R_565_MAX / 2, 0, screen::ColorLimit::B_565_MAX};
        constexpr Color DarkViolet = {screen::ColorLimit::R_565_MAX / 2, 0, screen::ColorLimit::B_565_MAX / 2};

        constexpr Color LightGreen = {0, screen::ColorLimit::G_565_MAX, screen::ColorLimit::B_565_MAX / 2};
        constexpr Color SkyBlue    = {0, screen::ColorLimit::G_565_MAX / 2, screen::ColorLimit::B_565_MAX};
        constexpr Color Teal       = {0, screen::ColorLimit::G_565_MAX / 2, screen::ColorLimit::B_565_MAX / 2};

        constexpr Color Black = {0, 0, 0};
    }

    enum class ApplyMode {

        Current,
        Default
    };

    struct ColumnRowAddr {

        uint8_t columnStart;
        uint8_t rowStart;
        uint8_t columnEnd;
        uint8_t rowEnd;
    };

    constexpr ColumnRowAddr defaultColumnRowAddr = {0, 0, screen::Geometry::Columns - 1, screen::Geometry::Rows - 1};

    namespace RemapColorDepth {

        // Bit positions
        constexpr uint8_t AddressIncrement_Pos = 0;
        constexpr uint8_t ColumnRemap_Pos      = 1;
        constexpr uint8_t ColorOrder_Pos       = 2;
        constexpr uint8_t COMSwap_Pos          = 3;
        constexpr uint8_t ScanDirection_Pos    = 4;
        constexpr uint8_t COMSplit_Pos         = 5;
        constexpr uint8_t ColorDepth_Pos       = 6;

        // Masks (shifted)
        constexpr uint8_t AddressIncrement_Msk = 1u << AddressIncrement_Pos;
        constexpr uint8_t ColumnRemap_Msk      = 1u << ColumnRemap_Pos;
        constexpr uint8_t ColorOrder_Msk       = 1u << ColorOrder_Pos;
        constexpr uint8_t COMSwap_Msk          = 1u << COMSwap_Pos;
        constexpr uint8_t ScanDirection_Msk    = 1u << ScanDirection_Pos;
        constexpr uint8_t COMSplit_Msk         = 1u << COMSplit_Pos;
        constexpr uint8_t ColorDepth_Msk       = 0b11u << ColorDepth_Pos;

        // Raw values (NOT shifted)
        enum class AddressIncrement : uint8_t { Horizontal = 0, Vertical = 1 };
        enum class ColumnRemap      : uint8_t { Normal = 0, Remap = 1 };
        enum class ColorOrder       : uint8_t { RGB = 0, BGR = 1 };
        enum class COMSwap          : uint8_t { NoSwap = 0, Swap = 1 };
        enum class ScanDirection    : uint8_t { COM0toN = 0, COMNto0 = 1 };
        enum class COMSplit         : uint8_t { Disable = 0, Enable = 1 };
        enum class ColorDepth       : uint8_t { Color256 = 0b00, Color65k = 0b01, Color65kAlt = 0b10 };
    }

    constexpr uint8_t defaultRemapColorDepth =
        (static_cast<uint8_t>(RemapColorDepth::AddressIncrement::Horizontal) << RemapColorDepth::AddressIncrement_Pos) |
        (static_cast<uint8_t>(RemapColorDepth::ColumnRemap::Remap)           << RemapColorDepth::ColumnRemap_Pos)      |
        (static_cast<uint8_t>(RemapColorDepth::ColorOrder::RGB)              << RemapColorDepth::ColorOrder_Pos)       |
        (static_cast<uint8_t>(RemapColorDepth::COMSwap::NoSwap)              << RemapColorDepth::COMSwap_Pos)          |
        (static_cast<uint8_t>(RemapColorDepth::ScanDirection::COMNto0)       << RemapColorDepth::ScanDirection_Pos)    |
        (static_cast<uint8_t>(RemapColorDepth::COMSplit::Enable)             << RemapColorDepth::COMSplit_Pos)         |
        (static_cast<uint8_t>(RemapColorDepth::ColorDepth::Color65k)         << RemapColorDepth::ColorDepth_Pos);

    constexpr bool defaultFillRectangle = false;

    constexpr bool defaultReverseCopy = false;
}

#endif // SCREEN_CONSTANTS_H