#include <cstdint>    // uint32_t
#include <fcntl.h>    // open
#include <unistd.h>   // close
#include <sys/mman.h> // mmap, munmap

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"
#include "screen_utils.h"

Screen::Screen(const std::string &uio_device) {

    const std::string path = "/dev/" + uio_device;

    m_fd = open(path.c_str(), O_RDWR | O_SYNC);
    if (m_fd < 0) {
        throw sys_error("Failed to open " + path);
    }

    m_reg = reinterpret_cast<volatile uint32_t *>(mmap(nullptr, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0));

    if (m_reg == MAP_FAILED) {
        close(m_fd);
        m_fd = -1;
        throw sys_error("mmap failed for " + path);
    }
}

Screen::~Screen() {

    if (m_reg && m_reg != MAP_FAILED) {
        munmap((void*)m_reg, MAP_SIZE);
    }

    if (m_fd >= 0) {
        close(m_fd);
    }
}

void Screen::setPowerState(bool value) {

    uint32_t ctrl = readRegister(screen::reg::POWER_CTRL);

    if (value) {
        ctrl |= screen::mask::ON_OFF;
    } else {
        ctrl &= ~screen::mask::ON_OFF;
    }

    writeRegister(screen::reg::POWER_CTRL, ctrl);
}

screen::PowerState Screen::getPowerState() const {

    uint32_t status = readRegister(screen::reg::POWER_STATUS);
    return static_cast<screen::PowerState>(status & screen::mask::ON_OFF_STATUS);
}

void Screen::sendCommand(const screen::Command cmd) {

    sendSpiByte(static_cast<uint8_t>(cmd), screen::DataMode::Command);
}

void Screen::sendMultiCommand(const screen::Command *cmds, size_t length) {

    for (size_t i = 0; i < length; ++i) {
        sendSpiByte(static_cast<uint8_t>(cmds[i]), screen::DataMode::Command);
    }
}

void Screen::sendData(const uint8_t data) {

    sendSpiByte(data, screen::DataMode::Data);
}

void Screen::sendMultiData(const uint8_t *data, size_t length) {

    for (size_t i = 0; i < length; ++i) {
        sendSpiByte(data[i], screen::DataMode::Data);
    }

}

void Screen::writeRegister(size_t reg, uint32_t value) {

    m_reg[reg] = value;
}

uint32_t Screen::readRegister(size_t reg) const {

    return m_reg[reg];
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
        // wait
    }

    uint32_t value = (static_cast<uint32_t>(byte) << screen::bit::BYTE)
                    | (static_cast<uint32_t>(mode) << screen::bit::DC_SELECT)
                    | (screen::mask::SPI_TRIGGER);

    writeRegister(screen::reg::SPI_CTRL, value);
}
