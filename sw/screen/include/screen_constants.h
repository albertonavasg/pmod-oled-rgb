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
        RemapAndColorDepth    = 0xA0,
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
        DisplayOff            = 0xAE,
        DisplayOn             = 0xAF,
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
}

#endif // SCREEN_CONSTANTS_H