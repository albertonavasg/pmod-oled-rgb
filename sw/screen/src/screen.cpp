#include <iostream>   // cout, endl
#include <cstdint>    // uint32_t
#include <cstring>    // strerror
#include <cerrno>     // errno
#include <stdexcept>  // runtime_error
#include <chrono>     // time
#include <thread>     // sleep_for
#include <fcntl.h>    // open
#include <unistd.h>   // close
#include <sys/mman.h> // mmap, munmap
#include <vector>     // vector

#include "fonts.h"

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

using namespace std::chrono_literals;

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

    if (!waitForPowerState(screen::PowerState::On, std::chrono::seconds(1))) {
        munmap((void*)m_reg, MAP_SIZE);
        close(m_fd);
        m_fd = -1;
        m_reg = nullptr;
        throw std::runtime_error("Screen did not power ON within timeout");
    }
}

Screen::~Screen() {

    if (m_reg && m_reg != MAP_FAILED) {
        writePowerState(false);
        waitForPowerState(screen::PowerState::Off, std::chrono::seconds(1));
        munmap((void*)m_reg, MAP_SIZE);
    }

    if (m_fd >= 0) {
        close(m_fd);
    }
}

void Screen::clearWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2) {

    uint8_t params[4] = {c1, r1, c2, r2};
    sendCommand(screen::Command::ClearWindow, params, 4);
}

void Screen::clearScreen() {

    clearWindow(0, 0, screen::Geometry::Columns - 1, screen::Geometry::Rows - 1);
}

void Screen::drawBitmap(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const std::vector<screen::Color> &colors) {

    setColumnRowAddr(c1, r1, c2, r2);
    applyColumnRowAddr();
    sendMultiPixel(colors);
}

void Screen::drawLine(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const screen::Color color) {

    uint8_t params[7] = {
        c1, r1, c2, r2,
        static_cast<uint8_t>(color.r << 1),
        color.g,
        static_cast<uint8_t>(color.b << 1)
    };
    sendCommand(screen::Command::DrawLine, params, 10);
}

void Screen::drawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const screen::Color colorLine, const screen::Color colorFill) {

    uint8_t params[10] = {
        c1, r1, c2, r2,
        static_cast<uint8_t>(colorLine.r << 1),
        colorLine.g,
        static_cast<uint8_t>(colorLine.b << 1),
        static_cast<uint8_t>(colorFill.r << 1),
        colorFill.g,
        static_cast<uint8_t>(colorFill.b << 1)
    };
    sendCommand(screen::Command::DrawRectangle, params, 10);
}

void Screen::copyWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint8_t c3, uint8_t r3) {

    uint8_t params[6] = {c1, r1, c2, r2, c3, r3};
    sendCommand(screen::Command::Copy, params, 6);
}

void Screen::drawImage(const std::string &path) {

    auto bitmap = importImageAsBitmap(path);

    if (bitmap.empty() || bitmap.size() != screen::Geometry::Pixels) {
        return;
    }

    drawBitmap(0, 0, screen::Geometry::Columns - 1 , screen::Geometry::Rows - 1, bitmap);
}

void Screen::drawSymbol(const uint8_t symbol, screen::Color color) {

    auto bitmap = importSymbolAsBitmap(symbol, color);

    if (m_orientation == screen::Orientation::Horizontal_0 || m_orientation == screen::Orientation::Horizontal_180) {
        drawBitmap(
            m_textCursor.x * m_fontWidth,
            m_textCursor.y * m_fontHeight,
            ((m_textCursor.x + 1) * m_fontWidth) - 1,
            ((m_textCursor.y + 1) * m_fontHeight)- 1,
            bitmap
        );
    } else {
        drawBitmap(
            m_textCursor.y * m_fontWidth,
            m_textCursor.x * m_fontHeight,
            ((m_textCursor.y + 1) * m_fontWidth) - 1,
            ((m_textCursor.x + 1) * m_fontHeight)- 1,
            bitmap
        );
    }
}

void Screen::drawString(const std::string &phrase, screen::Color color) {

    size_t i = 0;
    while (i < phrase.size()) {
        size_t len = 0;
        uint32_t codepoint = utf8_decode((const uint8_t*)&phrase[i], &len);

        // Only pass codepoints 0–255 to drawSymbol
        if (codepoint < screen::Font::TotalSize) {
            drawSymbol(static_cast<uint8_t>(codepoint), color);
        } else {
            // Unsupported character → fallback to '?'
            drawSymbol('?', color);
        }

        i += len;
        incrementTextCursor();
    }
}

void Screen::setupScrolling(uint8_t horizontalScrollOffset, uint8_t startRow, uint8_t rowsNumber, uint8_t verticalScrollOffset, uint8_t timeInterval) {

    uint8_t params[5] = {horizontalScrollOffset, startRow, rowsNumber, verticalScrollOffset, timeInterval};
    sendCommand(screen::Command::ContinuousScrolling, params, 5);
}

void Screen::enableScrolling(bool value) {

    sendCommand(value ? screen::Command::ActivateScroll : screen::Command::DeactivateScroll);
}

void Screen::setFontId(screen::FontId id) {

    m_fontId = id;

    switch (m_fontId) {
        case screen::FontId::Font6x8:
            m_fontBasic   = &font6x8_basic[0][0];
            m_fontControl = &font6x8_control[0][0];
            m_fontExtLatin = &font6x8_ext_latin[0][0];
            m_fontWidth = 6;
            m_fontHeight = 8;
            break;
        case screen::FontId::Font8x8:
            m_fontBasic   = &font8x8_basic[0][0];
            m_fontControl = &font8x8_control[0][0];
            m_fontExtLatin = &font8x8_ext_latin[0][0];
            m_fontWidth = 8;
            m_fontHeight = 8;
            break;
    }
}

screen::FontId Screen::getFontId() {

    return m_fontId;
}

void Screen::setTextCursor(uint8_t x, uint8_t y) {

    if (x >= textColumns()) {
        m_textCursor.x = textColumns() - 1;
    } else {
        m_textCursor.x = x;
    }

    if (y >= textRows()) {
        m_textCursor.y = textRows() - 1;
    } else {
        m_textCursor.y = y;
    }
}

screen::TextCursor Screen::getTextCursor() const {

    return m_textCursor;
}

void Screen::incrementTextCursor() {

    if (m_textCursor.x < textColumns() - 1) {
        m_textCursor.x++;
    } else if (m_textCursor.y < textRows() - 1) {
        m_textCursor.x = 0;
        m_textCursor.y++;
    } else {
        m_textCursor.x = 0;
        m_textCursor.y = 0;
    }
}

void Screen::setSpiDelay(std::chrono::nanoseconds delay) {

    m_spiDelay = delay;
}

std::chrono::nanoseconds Screen::getSpiDelay() const {

    return m_spiDelay;
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

screen::Orientation Screen::getScreenOrientation() const {

    return m_orientation;
}

void Screen::setFillRectangleEnable(bool fillRectangle) {

    m_fillRectangle = fillRectangle;
    uint8_t param = static_cast<uint8_t>(m_fillRectangle) | (static_cast<uint8_t>(m_reverseCopy) << 4);
    sendCommand(screen::Command::FillEnable, param);
}

void Screen::setReverseCopyEnable(bool reverseCopy) {

    m_reverseCopy = reverseCopy;
    uint8_t param = static_cast<uint8_t>(m_fillRectangle) | (static_cast<uint8_t>(m_reverseCopy) << 4);
    sendCommand(screen::Command::FillEnable, param);
}

void Screen::applyDefaultSettings() {

    setFontId(screen::defaultFontId);
    setSpiDelay(screen::defaultSpiDelay);
    setFillRectangleEnable(screen::defaultFillRectangle);
    setReverseCopyEnable(screen::defaultReverseCopy);
    m_orientation = screen::defaultOrientation;
    m_textCursor = screen::defaultTextCursor;

    applyColumnRowAddr(screen::ApplyMode::Default);
    applyRemapColorDepth(screen::ApplyMode::Default);
}