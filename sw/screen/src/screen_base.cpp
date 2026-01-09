#include <iostream>   // cout, endl
#include <chrono>     // time
#include <thread>     // sleep_for
#include <span>       // span

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

void Screen::writeRegister(size_t reg, uint32_t value) {

    m_reg[reg] = value;
}

uint32_t Screen::readRegister(size_t reg) const {

    return m_reg[reg];
}

void Screen::writePowerState(bool value) {

    uint32_t ctrl = readRegister(screen::reg::POWER_CTRL);

    if (value) {
        ctrl |= screen::mask::ON_OFF;
    } else {
        ctrl &= ~screen::mask::ON_OFF;
    }

    writeRegister(screen::reg::POWER_CTRL, ctrl);
}

screen::PowerState Screen::readPowerState() const {

    uint32_t status = readRegister(screen::reg::POWER_STATUS);
    return static_cast<screen::PowerState>(status & screen::mask::ON_OFF_STATUS);
}

bool Screen::isSpiReady() const {

    uint32_t status = readRegister(screen::reg::SPI_STATUS);
    return status & screen::mask::SPI_READY;
}

bool Screen::isSpiDataRequest() const {

    uint32_t status = readRegister(screen::reg::SPI_STATUS);
    return status & screen::mask::SPI_DATA_REQUEST;
}

void Screen::sendSpiByte(uint8_t byte, screen::DataMode mode) {

    while (!isSpiReady()) {
        // Wait
    }

    uint32_t value = (static_cast<uint32_t>(byte) << screen::bit::BYTE)
                    | (static_cast<uint32_t>(mode) << screen::bit::DC_SELECT)
                    | (screen::mask::SPI_TRIGGER);

    writeRegister(screen::reg::SPI_CTRL, value);

    if (m_spiDelay.count() > 0) {
        std::this_thread::sleep_for(m_spiDelay);
    }

    while (!isSpiReady()) {
        // Wait
    }
}

void Screen::sendCommand(screen::Command cmd, std::span<const uint8_t> params) {

    sendSpiByte(static_cast<uint8_t>(cmd), screen::DataMode::Command);

    for (uint8_t p : params) {
        sendSpiByte(p, screen::DataMode::Command);
    }
}

void Screen::sendData(const uint8_t data) {

    sendSpiByte(data, screen::DataMode::Data);
}

void Screen::sendMultiData(const uint8_t *data, size_t length) {

    for (size_t i = 0; i < length; i++) {
        sendSpiByte(data[i], screen::DataMode::Data);
    }
}