#ifndef SCREEN_H
#define SCREEN_H

#include <cstdint> // uint32_t
#include <string>  // string

#include "screen_constants.h"
#include "screen_registers.h"

class Screen {

    public:
        explicit Screen(const std::string &uio_device);
        ~Screen();

        void setPowerState(bool value);
        screen::PowerState getPowerState() const;

        void sendCommand(const screen::Command cmd);
        void sendMultiCommand(const screen::Command *cmds, size_t length);
        void sendData(const uint8_t data);
        void sendMultiData(const uint8_t *data, size_t length);

    private:
        int m_fd;
        volatile uint32_t *m_reg = nullptr;
        static constexpr uint64_t MAP_SIZE = 0x10000;

        void writeRegister(size_t reg, uint32_t value);
        uint32_t readRegister(size_t reg) const;

        bool isSpiReady() const;
        bool isSpiDataRequest() const;

        void sendSpiByte(uint8_t byte, screen::DataMode mode);
};

#endif // SCREEN_H