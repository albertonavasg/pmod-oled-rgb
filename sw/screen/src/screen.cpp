#include <iostream>    // cout, endl
#include <cstdint>     // uint32_t
#include <cstring>     // strerror
#include <cerrno>      // errno
#include <stdexcept>   // runtime_error
#include <chrono>      // time
#include <thread>      // sleep_for
#include <fcntl.h>     // open
#include <unistd.h>    // close
#include <sys/mman.h>  // mmap, munmap
#include <vector>      // vector
#include <string_view> // string_view

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

bool Screen::clearWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2) {

    // Check geometry
    if (c1 > c2 || r1 > r2 || c2 >= screen::Geometry::Columns || r2 >= screen::Geometry::Rows) {
        return false;
    }

    uint8_t params[4] = {c1, r1, c2, r2};
    sendCommand(screen::Command::ClearWindow, params, 4);

    return true;
}

void Screen::clearScreen() {

    clearWindow(0, 0, screen::Geometry::Columns - 1, screen::Geometry::Rows - 1);
}

bool Screen::drawBitmap(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const std::vector<screen::Color> &colors) {

    // Check geometry
    if (c1 > c2 || r1 > r2 || c2 >= screen::Geometry::Columns || r2 >= screen::Geometry::Rows) {
        return false;
    }

    size_t expectedSize = (c2 - c1 + 1) * (r2 - r1 + 1);

    // Check size
    if (expectedSize != colors.size()) {
        return false;
    }

    setColumnRowAddr(c1, r1, c2, r2);
    applyColumnRowAddr();
    sendMultiPixel(colors);

    return true;
}

bool Screen::drawLine(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const screen::Color color) {

    // Check geometry
    if (c1 >= screen::Geometry::Columns || c2 >= screen::Geometry::Columns || r1 >= screen::Geometry::Rows || r2 >= screen::Geometry::Rows) {
        return false;
    }

    uint8_t params[7] = {
        c1, r1, c2, r2,
        static_cast<uint8_t>(color.r << 1),
        color.g,
        static_cast<uint8_t>(color.b << 1)
    };
    sendCommand(screen::Command::DrawLine, params, 7);

    return true;
}

bool Screen::drawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const screen::Color colorLine, const screen::Color colorFill) {

    // Check geometry
    if (c1 >= screen::Geometry::Columns || c2 >= screen::Geometry::Columns || r1 >= screen::Geometry::Rows || r2 >= screen::Geometry::Rows) {
        return false;
    }

    uint8_t c_min = std::min(c1, c2);
    uint8_t c_max = std::max(c1, c2);

    uint8_t r_min = std::min(r1, r2);
    uint8_t r_max = std::max(r1, r2);

    uint8_t params[10] = {
        c_min, r_min, c_max, r_max,
        static_cast<uint8_t>(colorLine.r << 1),
        colorLine.g,
        static_cast<uint8_t>(colorLine.b << 1),
        static_cast<uint8_t>(colorFill.r << 1),
        colorFill.g,
        static_cast<uint8_t>(colorFill.b << 1)
    };
    sendCommand(screen::Command::DrawRectangle, params, 10);

    return true;
}

bool Screen::drawCircle(uint8_t x, uint8_t y, uint8_t d, const screen::Color colorLine) {

    // Check geometry
    if (d == 0 || x + d > screen::Geometry::Columns || y + d > screen::Geometry::Rows) {
        return false;
    }

    // Initialize bitmap
    std::vector<screen::Color> bitmap(d*d, screen::StandardColor::Black);

    //////////////////////////////////////////
    /// BASED ON MIDPOINT CIRCLE ALGORITHM ///
    //////////////////////////////////////////

    // Circle radius
    int r = d / 2;

    // std::cout << "d = " << static_cast<int>(d) << std::endl;
    // std::cout << "r = " << r << std::endl;

    // Variables to iterate pixel coords
    int px = r;
    int py = (d % 2) ? 0 : 1;
    int error = 3 - 2 * r;

    // Plot helper
    auto plot = [&](int lx, int ly) {
        if (lx >= -r && lx <= r && ly >= -r && ly <= r) {
            // Convert from circle coords to bitmap coords
            uint8_t col = (d % 2) ? (lx + r) : ((lx > 0) ? (lx + r - 1) : (lx + r));
            uint8_t row = (d % 2) ? (-ly + r) : ((ly > 0) ? (-ly+ r) : (-ly + r -1));
            bitmap[row * d + col] = colorLine;
            // std::cout << "(col, row): (" << static_cast<int>(col) << "," << static_cast<int>(row) << ")" << std::endl;
        }
    };

    while (py <= px){

        // Add symmetric points to bitmap
        plot(+px, +py); // Octant 1
        plot(-px, +py); // Octant 4
        plot(+px, -py); // Octant 8
        plot(-px, -py); // Octant 5

        plot(+py, +px); // Octant 2
        plot(-py, +px); // Octant 3
        plot(+py, -px); // Octant 7
        plot(-py, -px); // Octant 6
        // std::cout << "(+px, +py): (" << +px << "," << +py << ")" << std::endl;
        // std::cout << "Error: " << error << std::endl;
        // Midpoint update
        if (error > 0) {
            error += 2 * (5 - 2 * px + 2 * py);
            px--;
        } else {
            error += 2 * (3 + 2 * py);
        }

        py++;
    }

    drawBitmap(x, y, x + d - 1, y + d - 1, bitmap);

    return true;
}

bool Screen::copyWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint8_t c3, uint8_t r3) {

    // Check geometry
    // Valid starting window
    if (c1 > c2 || r1 > r2 || c2 >= screen::Geometry::Columns || r2 >= screen::Geometry::Rows) {
        return false;
    }
    // Valid end window
    if (c3 >= screen::Geometry::Columns || r3 >= screen::Geometry::Rows || c3 + (c2 - c1 + 1) >= screen::Geometry::Columns || r3 + (r2 - r1 + 1) >= screen::Geometry::Rows) {
        return false;
    }

    uint8_t params[6] = {c1, r1, c2, r2, c3, r3};
    sendCommand(screen::Command::Copy, params, 6);

    return true;
}

bool Screen::drawImage(const std::string &path) {

    std::vector<screen::Color> bitmap = importImageAsBitmap(path);

    if (bitmap.empty() || bitmap.size() != screen::Geometry::Pixels) {
        return false;
    }

    drawBitmap(0, 0, screen::Geometry::Columns - 1 , screen::Geometry::Rows - 1, bitmap);

    return true;
}

bool Screen::drawSymbol(const uint8_t symbol, uint8_t x, uint8_t y, const screen::Font &font, screen::Color color) {

    std::vector<screen::Color> bitmap = importSymbolAsBitmap(symbol, font, color);

    return drawBitmap(x, y, x + font.width - 1, y + font.height - 1, bitmap);
}

bool Screen::drawString(std::string_view phrase, uint8_t x, uint8_t y, const screen::Font &font, screen::Color color) {

    size_t i = 0;
    bool valid = true;

    uint8_t pixelCol = 0;
    uint8_t pixelRow = 0;
    uint8_t incrCol = 0;
    uint8_t incrRow = 0;

    if (m_orientation == screen::Orientation::Horizontal_0 || m_orientation == screen::Orientation::Horizontal_180) {
        pixelCol = x;
        pixelRow = y;
        incrCol = font.width;
        incrRow = 0;
    } else {
        pixelCol = y;
        pixelRow = x;
        incrCol = 0;
        incrRow = font.width;
    }

    while (i < phrase.size()) {
        size_t len = 0;
        uint32_t codepoint = utf8_decode((const uint8_t*)&phrase[i], &len);
        uint8_t glyph = (codepoint < 256) ? static_cast<uint8_t>(codepoint) : '?';
        valid &= drawSymbol(glyph, pixelCol , pixelRow, font, color);
        pixelCol += incrCol;
        pixelRow += incrRow;
        i += len;
    }

    return valid;
}

bool Screen::setupScrolling(uint8_t horizontalScrollOffset, uint8_t startRow, uint8_t rowsNumber, uint8_t verticalScrollOffset, uint8_t timeInterval) {

    // Valid scroll offset
    if (horizontalScrollOffset >= screen::Geometry::Columns || verticalScrollOffset >= screen::Geometry::Rows ) {
        return false;
    }
    // Valid horizontal rows scrolled
    if (startRow >= screen::Geometry::Rows || startRow + rowsNumber > screen::Geometry::Rows) {
        return false;
    }
    // Valid time interval
    if (timeInterval >= 0b11) {
        return false;
    }

    uint8_t params[5] = {horizontalScrollOffset, startRow, rowsNumber, verticalScrollOffset, timeInterval};
    sendCommand(screen::Command::ContinuousScrolling, params, 5);

    return true;
}

void Screen::enableScrolling(bool value) {

    sendCommand(value ? screen::Command::ActivateScroll : screen::Command::DeactivateScroll);
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

    setSpiDelay(screen::defaultSpiDelay);
    setFillRectangleEnable(screen::defaultFillRectangle);
    setReverseCopyEnable(screen::defaultReverseCopy);
    m_orientation = screen::defaultOrientation;

    applyColumnRowAddr(screen::ApplyMode::Default);
    applyRemapColorDepth(screen::ApplyMode::Default);
}