#ifndef SCREEN_H
#define SCREEN_H

#include <cstdint> // uint32_t
#include <string>  // string

#include "screen_constants.h"
#include "screen_registers.h"

class Screen {

    public:
        //Constructor and Destructor
        explicit Screen(const std::string &uio_device);
        ~Screen();

        // Power Control
        void setPowerState(bool value);
        screen::PowerState getPowerState() const;

        // Basic
        void sendCommand(const screen::Command cmd);
        void sendCommand(const screen::Command cmd, const uint8_t parameter);
        void sendCommand(const screen::Command cmd, const uint8_t *parameter, size_t length);
        void sendData(const uint8_t data);
        void sendMultiData(const uint8_t *data, size_t length);

        // Custom
        void sendPixel(const screen::Color color);
        void sendMultiPixel(const screen::Color *color, size_t length);

        // Remap and Color Depth Settings
        void setAddressIncrement(bool vertical);
        void setColumnRemap(bool remap);
        void setColorOrder(bool bgr);
        void setCOMSwap(bool swap);
        void setScanDirection(bool comNto0);
        void setCOMSplit(bool enable);
        void setColorDepth(uint8_t depth);
        void applyRemapColorDepth();

    private:
        int m_fd;
        volatile uint32_t *m_reg = nullptr;
        static constexpr uint64_t MAP_SIZE = 0x10000;

        // Screen properties and configurations
        uint8_t remapColorDepthCfg = screen::defaultRemapColorDepth;

        // Helper for byte manipulation
        static constexpr void setField(uint8_t& reg, uint8_t mask, uint8_t value) {
            reg = (reg & ~mask) | value;
        }

        // Register access
        void writeRegister(size_t reg, uint32_t value);
        uint32_t readRegister(size_t reg) const;

        // SPI
        bool isSpiReady() const;
        bool isSpiDataRequest() const;
        void sendSpiByte(uint8_t byte, screen::DataMode mode);
};

#endif // SCREEN_H