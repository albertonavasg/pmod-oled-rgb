#include <iostream> // cout, endl
#include <cstdint>  // uint32_t
#include <vector>   // vector

#include "screen_constants.h"
#include "screen_registers.h"
#include "screen.h"

void Screen::sendPixel(const screen::Color color) {

    uint32_t data = 0;
	uint8_t byte[3] = {0};
    uint8_t colorDepth =
        (m_remapColorDepthCfg & screen::RemapColorDepth::ColorDepth_Msk)
        >> screen::RemapColorDepth::ColorDepth_Pos;

    switch (static_cast<screen::RemapColorDepth::ColorDepth>(colorDepth)) {
        case screen::RemapColorDepth::ColorDepth::Color256: {
            byte[0] = (uint8_t)((color.r>>2) << 5 | (color.g>>3) << 2 | (color.b>>3));
			sendData(byte[0]);
            break;
        }
        case screen::RemapColorDepth::ColorDepth::Color65k: {
 			data = ((color.r) << 11 | (color.g) << 5 | (color.b));
			byte[0] = (uint8_t)(data >> 8);
			byte[1] = (uint8_t)(data & 0x000000FF);
			sendMultiData(byte, 2);
            break;
        }
        case screen::RemapColorDepth::ColorDepth::Color65kAlt: {
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

void Screen::sendMultiPixel(const std::vector<screen::Color>& colors) {

    for (const auto& c : colors) {
        sendPixel(c);
    }
}

void Screen::clearWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2) {

    uint8_t params[4] = {c1, r1, c2, r2};
    sendCommand(screen::Command::ClearWindow, params, 4);
}

void Screen::clearScreen() {

    clearWindow(0, 0, screen::Geometry::Columns - 1, screen::Geometry::Rows - 1);
}

void Screen::drawBitmap(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const std::vector<screen::Color>& colors) {

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

void Screen::setTextCursor(uint8_t x, uint8_t y) {

    if (m_orientation == screen::Orientation::Horizontal_0 || m_orientation == screen::Orientation::Horizontal_180) {
        if (x > screen::TextGeometry::TextColumns - 1) {
            m_textCursor.x = screen::TextGeometry::TextColumns - 1;
        } else {
            m_textCursor.x = x;
        }
        if (y > screen::TextGeometry::TextRows - 1) {
            m_textCursor.y = screen::TextGeometry::TextRows - 1;
        } else {
            m_textCursor.y = y;
        }
    } else {
        if (x > screen::TextGeometry::TextRows - 1) {
            m_textCursor.x = screen::TextGeometry::TextRows - 1;
        } else {
            m_textCursor.x = x;
        }
        if (y > screen::TextGeometry::TextColumns - 1) {
            m_textCursor.y = screen::TextGeometry::TextColumns - 1;
        } else {
            m_textCursor.y = y;
        }
    }

}

screen::TextCursor Screen::getTextCursor() {

    return m_textCursor;
}

void Screen::incrementTextCursor() {

    if (m_orientation == screen::Orientation::Horizontal_0 || m_orientation == screen::Orientation::Horizontal_180) {
        if (m_textCursor.x < screen::TextGeometry::TextColumns - 1) {
            m_textCursor.x++;
        } else if (m_textCursor.y < screen::TextGeometry::TextRows - 1) {
            m_textCursor.x = 0;
            m_textCursor.y++;
        } else {
            m_textCursor.x = 0;
            m_textCursor.y = 0;
        }
    } else {
        if (m_textCursor.x < screen::TextGeometry::TextRows - 1) {
            m_textCursor.x++;
        } else if (m_textCursor.y < screen::TextGeometry::TextColumns - 1) {
            m_textCursor.x = 0;
            m_textCursor.y++;
        } else {
            m_textCursor.x = 0;
            m_textCursor.y = 0;
        }
    }

}

void Screen::drawSymbol(const uint8_t symbol, screen::Color color) {

    auto bitmap = importSymbolAsBitmap(symbol, color);

    if (m_orientation == screen::Orientation::Horizontal_0 || m_orientation == screen::Orientation::Horizontal_180) {
        drawBitmap(
            m_textCursor.x * screen::FontWidth,
            m_textCursor.y * screen::FontHeight,
            ((m_textCursor.x + 1) * screen::FontWidth) - 1,
            ((m_textCursor.y + 1) * screen::FontHeight)- 1,
            bitmap
        );
    } else {
        drawBitmap(
            m_textCursor.y * screen::FontWidth,
            m_textCursor.x * screen::FontHeight,
            ((m_textCursor.y + 1) * screen::FontWidth) - 1,
            ((m_textCursor.x + 1) * screen::FontHeight)- 1,
            bitmap
        );
    }

}

void Screen::drawString(const std::string &phrase, screen::Color color) {

    for (char c : phrase) {
        drawSymbol(c, color);
        incrementTextCursor();
    }
}

void Screen::enableFillRectangle(bool fillRectangle) {

    m_fillRectangle = fillRectangle;
    uint8_t param = static_cast<uint8_t>(m_fillRectangle) | (static_cast<uint8_t>(m_reverseCopy) << 4);
    sendCommand(screen::Command::FillEnable, param);
}

void Screen::enableReverseCopy(bool reverseCopy) {

    m_reverseCopy = reverseCopy;
    uint8_t param = static_cast<uint8_t>(m_fillRectangle) | (static_cast<uint8_t>(m_reverseCopy) << 4);
    sendCommand(screen::Command::FillEnable, param);
}

void Screen::setupScrolling(uint8_t horizontalScrollOffset, uint8_t startRow, uint8_t rowsNumber, uint8_t verticalScrollOffset, uint8_t timeInterval) {

    uint8_t params[5] = {horizontalScrollOffset, startRow, rowsNumber, verticalScrollOffset, timeInterval};
    sendCommand(screen::Command::ContinuousScrolling, params, 5);
}

void Screen::enableScrolling(bool value) {

    sendCommand(value ? screen::Command::ActivateScroll : screen::Command::DeactivateScroll);
}