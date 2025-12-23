#ifndef SCREEN_H
#define SCREEN_H

#include <cstdint> // uint32_t
#include <string>  // string
#include <span>    // span
#include <vector>  // vector
#include <chrono>  // time

#include "screen_constants.h"
#include "screen_registers.h"

class Screen {

    friend class Test;

    public:
        // Constructor and Destructor
        explicit Screen(const std::string &uio_device);
        ~Screen();

        // Utilities
        void sendPixel(const screen::Color color);
        void sendMultiPixel(const std::vector<screen::Color>& colors);
        void clearWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2);
        void clearScreen();
        void drawBitmap(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const std::vector<screen::Color>& colors);
        void drawLine(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const screen::Color color);
        void drawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, const screen::Color colorLine, const screen::Color colorFill);
        void enableFill(bool fillRectangle, bool reverseCopy);
        void setupScrolling(uint8_t horizontalScrollOffset, uint8_t startRow, uint8_t rowsNumber, uint8_t verticalScrollOffset, uint8_t timeInterval);
        void enableScrolling(bool value);

        // Settings
        // Default settings
        void applyDefaultSettings();
        // SPI delay
        void setSpiDelay(std::chrono::nanoseconds delay);
        std::chrono::nanoseconds getSpiDelay();
        // Column and Row addresses
        void setColumnRowAddr(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2);
        void applyColumnRowAddr(screen::ApplyMode mode = screen::ApplyMode::Current);
        screen::ColumnRowAddr getColumnRowAddr();
        // Remap and Color Depth
        void setAddressIncrement(bool vertical);
        void setColumnRemap(bool remap);
        void setColorOrder(bool bgr);
        void setCOMSwap(bool swap);
        void setScanDirection(bool comNto0);
        void setCOMSplit(bool enable);
        void setColorDepth(uint8_t depth);
        void applyRemapColorDepth(screen::ApplyMode mode = screen::ApplyMode::Current);
        uint8_t getRemapColorDepth();

    private:
        int m_fd;
        volatile uint32_t *m_reg = nullptr;
        static constexpr uint64_t MAP_SIZE = 0x10000;

        // Settings
        std::chrono::nanoseconds m_spiDelay{0};
        screen::ColumnRowAddr m_columnRowAddr = screen::defaultColumnRowAddr;
        uint8_t m_remapColorDepthCfg = screen::defaultRemapColorDepth;
        bool m_enableFill = false;
        bool m_reverseCopy = false;

        // Helper for byte manipulation
        static constexpr void setField(uint8_t& reg, uint8_t mask, uint8_t value) {
            reg = (reg & ~mask) | value;
        }

        // Base
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
};

#endif // SCREEN_H