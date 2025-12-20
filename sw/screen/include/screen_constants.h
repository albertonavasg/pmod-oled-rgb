#ifndef SCREEN_CONSTANTS_H
#define SCREEN_CONSTANTS_H

#include <cstdint>

namespace screen {

    namespace Geometry {

        constexpr uint16_t Rows    = 64;
        constexpr uint16_t Columns = 96;
        constexpr uint16_t Pixels  = Rows * Columns;
    }

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

    namespace RemapColorDepth {

        constexpr uint8_t HorizontalIncrement = 0 << 0;
        constexpr uint8_t VerticalIncrement   = 1 << 0;

        constexpr uint8_t ColumnNormal        = 0 << 1;
        constexpr uint8_t ColumnRemap         = 1 << 1;

        constexpr uint8_t RGB                 = 0 << 2;
        constexpr uint8_t BGR                 = 1 << 2;

        constexpr uint8_t COMNoSwap           = 0 << 3;
        constexpr uint8_t COMSwap             = 1 << 3;

        constexpr uint8_t ScanCOM0toN         = 0 << 4;
        constexpr uint8_t ScanCOMNto0         = 1 << 4;

        constexpr uint8_t COMSplitDisable     = 0 << 5;
        constexpr uint8_t COMSplitEnable      = 1 << 5;

        constexpr uint8_t Color256            = 0b00 << 6;
        constexpr uint8_t Color65k            = 0b01 << 6;
        constexpr uint8_t Color65kAlt         = 0b10 << 6;
    }

    namespace RemapColorDepthMask {

        constexpr uint8_t AddressIncrement = 1u << 0;    // bit 0
        constexpr uint8_t ColumnRemap      = 1u << 1;    // bit 1
        constexpr uint8_t ColorOrder       = 1u << 2;    // bit 2
        constexpr uint8_t COMSwap          = 1u << 3;    // bit 3
        constexpr uint8_t ScanDirection    = 1u << 4;    // bit 4
        constexpr uint8_t COMSplit         = 1u << 5;    // bit 5
        constexpr uint8_t ColorDepth       = 0b11u << 6; // bits 6–7
    }

    enum class RemapApplyMode {
        Current,
        Default
    };

    constexpr uint8_t defaultRemapColorDepth =
        RemapColorDepth::HorizontalIncrement |
        RemapColorDepth::ColumnRemap         |
        RemapColorDepth::RGB                 |
        RemapColorDepth::COMNoSwap           |
        RemapColorDepth::ScanCOMNto0         |
        RemapColorDepth::COMSplitEnable      |
        RemapColorDepth::Color65k;
}

#endif // SCREEN_CONSTANTS_H