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
    enableFill(false, false);
    m_textCursor = screen::defaultTextCursor;
    m_orientation = screen::defaultOrientation;
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

void Screen::setAddressIncrement(screen::RemapColorDepth::AddressIncrement inc) {

    setField(m_remapColorDepthCfg,
            screen::RemapColorDepth::AddressIncrement_Msk,
            screen::RemapColorDepth::AddressIncrement_Pos,
            static_cast<uint8_t>(inc));
}

void Screen::setColumnRemap(screen::RemapColorDepth::ColumnRemap col) {

    setField(m_remapColorDepthCfg,
            screen::RemapColorDepth::ColumnRemap_Msk,
            screen::RemapColorDepth::ColumnRemap_Pos,
            static_cast<uint8_t>(col));
}

void Screen::setColorOrder(screen::RemapColorDepth::ColorOrder ord) {

    setField(m_remapColorDepthCfg,
            screen::RemapColorDepth::ColorOrder_Msk,
            screen::RemapColorDepth::ColorOrder_Pos,
            static_cast<uint8_t>(ord));
}

void Screen::setCOMSwap(screen::RemapColorDepth::COMSwap swap) {

    setField(m_remapColorDepthCfg,
            screen::RemapColorDepth::COMSwap_Msk,
            screen::RemapColorDepth::COMSwap_Pos,
            static_cast<uint8_t>(swap));
}

void Screen::setScanDirection(screen::RemapColorDepth::ScanDirection scan) {

    setField(m_remapColorDepthCfg,
            screen::RemapColorDepth::ScanDirection_Msk,
            screen::RemapColorDepth::ScanDirection_Pos,
            static_cast<uint8_t>(scan));
}

void Screen::setCOMSplit(screen::RemapColorDepth::COMSplit split) {

    setField(m_remapColorDepthCfg,
            screen::RemapColorDepth::COMSplit_Msk,
            screen::RemapColorDepth::COMSplit_Pos,
            static_cast<uint8_t>(split));
}

void Screen::setColorDepth(screen::RemapColorDepth::ColorDepth depth) {

    setField(m_remapColorDepthCfg,
            screen::RemapColorDepth::ColorDepth_Msk,
            screen::RemapColorDepth::ColorDepth_Pos,
            static_cast<uint8_t>(depth));
}

void Screen::applyRemapColorDepth(screen::ApplyMode mode) {

    if (mode == screen::ApplyMode::Default) {
        m_remapColorDepthCfg = screen::defaultRemapColorDepth;
    }
    sendCommand(screen::Command::RemapColorDepth, m_remapColorDepthCfg);
}

void Screen::setScreenOrientation(const screen::Orientation orientation) {

    m_orientation = orientation;

    switch (m_orientation) {
        case screen::Orientation::Horizontal_0:
            setAddressIncrement(screen::RemapColorDepth::AddressIncrement::Horizontal);
            setColumnRemap(screen::RemapColorDepth::ColumnRemap::Remap);
            setScanDirection(screen::RemapColorDepth::ScanDirection::COMNto0);
            break;
        case screen::Orientation::Vertical_90:
            setAddressIncrement(screen::RemapColorDepth::AddressIncrement::Vertical);
            setColumnRemap(screen::RemapColorDepth::ColumnRemap::Remap);
            setScanDirection(screen::RemapColorDepth::ScanDirection::COM0toN);
            break;
        case screen::Orientation::Horizontal_180:
            setAddressIncrement(screen::RemapColorDepth::AddressIncrement::Horizontal);
            setColumnRemap(screen::RemapColorDepth::ColumnRemap::Normal);
            setScanDirection(screen::RemapColorDepth::ScanDirection::COM0toN);
            break;
        case screen::Orientation::Vertical_270:
            setAddressIncrement(screen::RemapColorDepth::AddressIncrement::Vertical);
            setColumnRemap(screen::RemapColorDepth::ColumnRemap::Normal);
            setScanDirection(screen::RemapColorDepth::ScanDirection::COMNto0);
            break;
        default:
            break;
    }

    applyRemapColorDepth();
}

screen::Orientation Screen::getScreenOrientation() {

    return m_orientation;
}