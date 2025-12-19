#include <iostream>   // cout, endl
#include <cstdint>    // uint32_t
#include <chrono>     // milliseconds
#include <thread>     // sleep_for
#include <span>       // span
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

void Screen::sendPixel(const screen::Color color) {

    uint32_t data = 0;
	uint8_t byte[3] = {0};
    uint8_t colorDepth = remapColorDepthCfg & screen::RemapColorDepthMask::ColorDepth;

    switch (colorDepth) {
        case screen::RemapColorDepth::Color256: {
            byte[0] = (uint8_t)((color.r>>2) << 5 | (color.g>>3) << 2 | (color.b>>3));
			sendData(byte[0]);
            break;
        }
        case screen::RemapColorDepth::Color65k: {
 			data = ((color.r) << 11 | (color.g) << 5 | (color.b));
			byte[0] = (uint8_t)(data >> 8);
			byte[1] = (uint8_t)(data & 0x000000FF);
			sendMultiData(byte, 2);
            break;
        }
        case screen::RemapColorDepth::Color65kAlt: {
			data = ((color.r) << 17 | (color.g) << 8 | (color.b) << 1);
			byte[0] = (uint8_t)(data >> 16);
			byte[1] = (uint8_t)((data & 0x0000FF00) >> 8);
			byte[2] = (uint8_t)(data & 0x000000FF);
			sendMultiData(byte, 3);
            break;
        }
        default:
            break;
    }
}

void Screen::sendMultiPixel(const screen::Color *color, size_t length) {

    for (size_t i = 0; i < length; i++) {
        sendPixel(color[i]);
    }
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

void Screen::applyRemapColorDepth(){

    sendCommand(screen::Command::RemapColorDepth, remapColorDepthCfg);
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
