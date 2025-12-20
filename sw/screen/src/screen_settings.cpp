#include <iostream> // cout, endl

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

void Screen::setSpiDelay(std::chrono::nanoseconds delay) {

    m_spiDelay = delay;
}

std::chrono::nanoseconds Screen::getSpiDelay() {

    return m_spiDelay;
}

void Screen::setColumnRowAddr(screen::ColumnRowAddr cr) {

    m_columnRowAddr = cr;

    uint8_t column_params[2] = {m_columnRowAddr.columnStart, m_columnRowAddr.columnEnd};
    uint8_t row_params[2] = {m_columnRowAddr.rowStart, m_columnRowAddr.rowEnd};
    sendCommand(screen::Command::ColumnAddress, column_params, 2);
    sendCommand(screen::Command::RowAddress, row_params, 2);
}

screen::ColumnRowAddr Screen::getColumnRowAddr() {

    return m_columnRowAddr;
}

void Screen::setAddressIncrement(bool vertical) {

    setField(remapColorDepthCfg,
             screen::RemapColorDepthMask::AddressIncrement,
             vertical ? screen::RemapColorDepth::VerticalIncrement : screen::RemapColorDepth::HorizontalIncrement);
}

void Screen::setColumnRemap(bool remap) {

    setField(remapColorDepthCfg,
             screen::RemapColorDepthMask::ColumnRemap,
             remap ? screen::RemapColorDepth::ColumnRemap : screen::RemapColorDepth::ColumnNormal);
}

void Screen::setColorOrder(bool bgr) {

    setField(remapColorDepthCfg,
             screen::RemapColorDepthMask::ColorOrder,
             bgr ? screen::RemapColorDepth::BGR : screen::RemapColorDepth::RGB);
}

void Screen::setCOMSwap(bool swap) {

    setField(remapColorDepthCfg,
             screen::RemapColorDepthMask::COMSwap,
             swap ? screen::RemapColorDepth::COMSwap : screen::RemapColorDepth::COMNoSwap);
}

void Screen::setScanDirection(bool comNto0) {

    setField(remapColorDepthCfg,
             screen::RemapColorDepthMask::ScanDirection,
             comNto0 ? screen::RemapColorDepth::ScanCOMNto0 : screen::RemapColorDepth::ScanCOM0toN);
}

void Screen::setCOMSplit(bool enable) {

    setField(remapColorDepthCfg,
             screen::RemapColorDepthMask::COMSplit,
             enable ? screen::RemapColorDepth::COMSplitEnable : screen::RemapColorDepth::COMSplitDisable);
}

void Screen::setColorDepth(uint8_t depth) {

    setField(remapColorDepthCfg,
             screen::RemapColorDepthMask::ColorDepth,
             depth);
}

void Screen::applyRemapColorDepth(screen::RemapApplyMode mode) {

    if (mode == screen::RemapApplyMode::Default) {
        remapColorDepthCfg = screen::defaultRemapColorDepth;
    }
    sendCommand(screen::Command::RemapColorDepth, remapColorDepthCfg);
}