#ifndef SCREEN_H
#define SCREEN_H

#include <cstdint> // uint32_t
#include <string>  // string
#include <span>    // span
#include <vector>  // vector
#include <chrono>  // time

#include "fonts.h"

#include "screen_constants.h"
#include "screen_registers.h"

class Screen {

    friend class Test;

    public:
        //// Constructor and Destructor
        explicit Screen(const std::string &uio_device);
        ~Screen();

        //// Public methods
        void clearWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2);
        void clearScreen();

        void drawBitmap(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const std::vector<screen::Color> &colors);

        void drawLine(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const screen::Color color);
        void drawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const screen::Color colorLine, const screen::Color colorFill);

        void copyWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint8_t c3, uint8_t r3);

        void drawImage(const std::string &path);

        void drawSymbol(const uint8_t symbol, screen::Color color);
        void drawString(const std::string &phrase, screen::Color color);

        void setupScrolling(uint8_t horizontalScrollOffset, uint8_t startRow, uint8_t rowsNumber, uint8_t verticalScrollOffset, uint8_t timeInterval);
        void enableScrolling(bool value);

        //// Public settings
        void setFontId(screen::FontId id);
        screen::FontId getFontId();

        void setTextCursor(uint8_t x, uint8_t y);
        screen::TextCursor getTextCursor() const;
        void incrementTextCursor();

        void setSpiDelay(std::chrono::nanoseconds delay);
        std::chrono::nanoseconds getSpiDelay() const;

        void setScreenOrientation(const screen::Orientation orientation);
        screen::Orientation getScreenOrientation() const;

        void setFillRectangleEnable(bool fillRectangle);
        void setReverseCopyEnable(bool reverseCopy);

        void applyDefaultSettings();

    private:
        int m_fd;
        volatile uint32_t *m_reg = nullptr;
        static constexpr uint64_t MAP_SIZE = 0x10000;

        screen::FontId m_fontId = screen::defaultFontId;
        screen::TextCursor m_textCursor = screen::defaultTextCursor;
        std::chrono::nanoseconds m_spiDelay = screen::defaultSpiDelay;
        screen::Orientation m_orientation = screen::defaultOrientation;
        bool m_fillRectangle = screen::defaultFillRectangle;
        bool m_reverseCopy = screen::defaultReverseCopy;

        screen::ColumnRowAddr m_columnRowAddr = screen::defaultColumnRowAddr;
        uint8_t m_remapColorDepthCfg = screen::defaultRemapColorDepth;

        uint8_t m_fontWidth = 8;
        uint8_t m_fontHeight = 8;
        const uint8_t* m_fontBasic  = &font8x8_basic[0][0];
        const uint8_t* m_fontControl = &font8x8_control[0][0];
        const uint8_t* m_fontExtLatin = &font8x8_ext_latin[0][0];

        // Helper for byte manipulation
        static constexpr void setField(uint8_t &reg, uint8_t mask, uint8_t pos, uint8_t value) {
            reg = (reg & ~mask) | ((value << pos) & mask);
        }

        //// Base
        // Register access
        void writeRegister(size_t reg, uint32_t value);
        uint32_t readRegister(size_t reg) const;

        // Power Control
        void writePowerState(bool value);
        screen::PowerState readPowerState() const;

        // SPI Communication
        bool isSpiReady() const;
        bool isSpiDataRequest() const;
        void sendSpiByte(uint8_t byte, screen::DataMode mode);
        void sendCommand(screen::Command cmd, std::span<const uint8_t> params);
        inline void sendCommand(screen::Command cmd) {
            sendCommand(cmd, std::span<const uint8_t>{});
        }
        inline void sendCommand(screen::Command cmd, uint8_t param){
            sendCommand(cmd, std::span<const uint8_t>{ &param, 1 });
        }
        inline void sendCommand(screen::Command cmd, const uint8_t* params, size_t length) {
            sendCommand(cmd, std::span<const uint8_t>{ params, length });
        }
        void sendData(const uint8_t data);
        void sendMultiData(const uint8_t *data, size_t length);

        //// Utilities
        void sendPixel(const screen::Color color);
        void sendMultiPixel(const std::vector<screen::Color> &colors);

        ////  Internal settings
        void setColumnRowAddr(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2);
        void applyColumnRowAddr(screen::ApplyMode mode = screen::ApplyMode::Current);
        screen::ColumnRowAddr getColumnRowAddr() const;

        void setAddressIncrement(screen::RemapColorDepth::AddressIncrement inc);
        void setColumnRemap(screen::RemapColorDepth::ColumnRemap col);
        void setColorOrder(screen::RemapColorDepth::ColorOrder ord);
        void setCOMSwap(screen::RemapColorDepth::COMSwap swap);
        void setScanDirection(screen::RemapColorDepth::ScanDirection scan);
        void setCOMSplit(screen::RemapColorDepth::COMSplit split);
        void setColorDepth(screen::RemapColorDepth::ColorDepth depth);
        void applyRemapColorDepth(screen::ApplyMode mode = screen::ApplyMode::Current);
        uint8_t getRemapColorDepth() const;

        //// Helpers
        bool waitForPowerState(screen::PowerState target, std::chrono::milliseconds timeout);
        uint8_t textColumns() const;
        uint8_t textRows() const;
        std::vector<screen::Color> importImageAsBitmap(const std::string &path);
        std::vector<screen::Color> importSymbolAsBitmap(const uint8_t symbol, screen::Color color);
        uint32_t utf8_decode(const uint8_t *s, size_t *len);
};

#endif // SCREEN_H