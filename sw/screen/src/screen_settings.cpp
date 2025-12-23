#include <iostream> // cout, endl

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

using namespace std::chrono_literals;

void Screen::setSpiDelay(std::chrono::nanoseconds delay) {

    m_spiDelay = delay;
}

std::chrono::nanoseconds Screen::getSpiDelay() {

    return m_spiDelay;
}

void Screen::applyDefaultSettings() {

    setSpiDelay(0ns);
    applyColumnRowAddr(screen::ApplyMode::Default);
    applyRemapColorDepth(screen::ApplyMode::Default);
    m_enableFill = false;
    m_reverseCopy = false;
}

void Screen::setColumnRowAddr(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2) {

    screen::ColumnRowAddr cr = {c1, r1, c2, r2};
    m_columnRowAddr = cr;
}

void Screen::applyColumnRowAddr(screen::ApplyMode mode) {

    if (mode == screen::ApplyMode::Default) {
        m_columnRowAddr = screen::defaultColumnRowAddr;
    }
    uint8_t column_params[2] = {m_columnRowAddr.columnStart, m_columnRowAddr.columnEnd};
    uint8_t row_params[2] = {m_columnRowAddr.rowStart, m_columnRowAddr.rowEnd};
    sendCommand(screen::Command::ColumnAddress, column_params, 2);
    sendCommand(screen::Command::RowAddress, row_params, 2);
}

screen::ColumnRowAddr Screen::getColumnRowAddr() {

    return m_columnRowAddr;
}

void Screen::setAddressIncrement(bool vertical) {

    setField(m_remapColorDepthCfg,
             screen::RemapColorDepthMask::AddressIncrement,
             vertical ? screen::RemapColorDepth::VerticalIncrement : screen::RemapColorDepth::HorizontalIncrement);
}

void Screen::setColumnRemap(bool remap) {

    setField(m_remapColorDepthCfg,
             screen::RemapColorDepthMask::ColumnRemap,
             remap ? screen::RemapColorDepth::ColumnRemap : screen::RemapColorDepth::ColumnNormal);
}

void Screen::setColorOrder(bool bgr) {

    setField(m_remapColorDepthCfg,
             screen::RemapColorDepthMask::ColorOrder,
             bgr ? screen::RemapColorDepth::BGR : screen::RemapColorDepth::RGB);
}

void Screen::setCOMSwap(bool swap) {

    setField(m_remapColorDepthCfg,
             screen::RemapColorDepthMask::COMSwap,
             swap ? screen::RemapColorDepth::COMSwap : screen::RemapColorDepth::COMNoSwap);
}

void Screen::setScanDirection(bool comNto0) {

    setField(m_remapColorDepthCfg,
             screen::RemapColorDepthMask::ScanDirection,
             comNto0 ? screen::RemapColorDepth::ScanCOMNto0 : screen::RemapColorDepth::ScanCOM0toN);
}

void Screen::setCOMSplit(bool enable) {

    setField(m_remapColorDepthCfg,
             screen::RemapColorDepthMask::COMSplit,
             enable ? screen::RemapColorDepth::COMSplitEnable : screen::RemapColorDepth::COMSplitDisable);
}

void Screen::setColorDepth(uint8_t depth) {

    setField(m_remapColorDepthCfg,
             screen::RemapColorDepthMask::ColorDepth,
             depth);
}

void Screen::applyRemapColorDepth(screen::ApplyMode mode) {

    if (mode == screen::ApplyMode::Default) {
        m_remapColorDepthCfg = screen::defaultRemapColorDepth;
    }
    sendCommand(screen::Command::RemapColorDepth, m_remapColorDepthCfg);
}