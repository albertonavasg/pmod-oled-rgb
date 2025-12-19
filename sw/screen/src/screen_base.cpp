#include <iostream>   // cout, endl
#include <cstdint>    // uint32_t
#include <stdexcept>  // runtime_error
#include <cstring>    // strerror
#include <cerrno>     // errno
#include <span>       // span
#include <fcntl.h>    // open
#include <unistd.h>   // close
#include <sys/mman.h> // mmap, munmap

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

Screen::Screen(const std::string &uio_device) {

    const std::string path = "/dev/" + uio_device;

    m_fd = open(path.c_str(), O_RDWR | O_SYNC);
    if (m_fd < 0) {
        throw std::runtime_error("Failed to open " + path + ": " + std::strerror(errno));
    }

    m_reg = reinterpret_cast<volatile uint32_t *>(mmap(nullptr, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0));

    if (m_reg == MAP_FAILED) {
        close(m_fd);
        m_fd = -1;
        throw std::runtime_error("mmap failed for " + path + ": " + std::strerror(errno));
    }

    writePowerState(true);

    while (readPowerState() != screen::PowerState::On) {
        // Wait
    }
}

Screen::~Screen() {

    writePowerState(false);
    while (readPowerState() != screen::PowerState::Off) {
        // Wait
    }

    if (m_reg && m_reg != MAP_FAILED) {
        munmap((void*)m_reg, MAP_SIZE);
    }

    if (m_fd >= 0) {
        close(m_fd);
    }
}

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