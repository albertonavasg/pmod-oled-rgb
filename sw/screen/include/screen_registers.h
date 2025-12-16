#ifndef SCREEN_REGISTERS_H
#define SCREEN_REGISTERS_H

namespace screen::reg {

    constexpr uint32_t POWER_CTRL   = 0; // slv_reg0
    constexpr uint32_t POWER_STATUS = 1; // slv_reg1
    constexpr uint32_t SPI_CTRL     = 2; // slv_reg2
    constexpr uint32_t SPI_STATUS   = 3; // slv_reg3
}

namespace screen::bit {

    // slv_reg0
    constexpr uint32_t ON_OFF = 0;

    // slv_reg1
    constexpr uint32_t ON_OFF_STATUS = 0; // bits [1:0]

    // slv_reg2
    constexpr uint32_t BYTE        = 0; // bits [7:0]
    constexpr uint32_t DC_SELECT   = 8;
    constexpr uint32_t SPI_TRIGGER = 9;

    // slv_reg3
    constexpr uint32_t SPI_READY        = 0;
    constexpr uint32_t SPI_DATA_REQUEST = 1;
}

namespace screen::mask {

    // slv_reg0
    constexpr uint32_t ON_OFF = 1u << bit::ON_OFF;

    // slv_reg1
    constexpr uint32_t ON_OFF_STATUS = 0b11 << bit::ON_OFF_STATUS;

    // slv_reg2
    constexpr uint32_t BYTE        = 0xFF << bit::BYTE;
    constexpr uint32_t DC_SELECT   = 1u   << bit::DC_SELECT;
    constexpr uint32_t SPI_TRIGGER = 1u   << bit::SPI_TRIGGER;

    // slv_reg3
    constexpr uint32_t SPI_READY        = 1u << bit::SPI_READY;
    constexpr uint32_t SPI_DATA_REQUEST = 1u << bit::SPI_DATA_REQUEST;

}

//  -- SCREEN IP REGISTER MAP --

// 	-- Slave Register 0 (slv_reg0) (WRITE)
// 		-- Bits 31:1  : Reserved
// 		-- Bit 0      : ON_OFF (W) Control signal to turn ON/OFF the screen

// 	-- Slave Register 1 (slv_reg1) (READ)
// 		-- Bits 31:2  : Reserved
// 		-- Bits 1:0   : ON_OFF_STATUS (R) Status signal to indicate if the status of the screen is ON/OFF
// 			-- 00: Screen OFF
// 			-- 01: Screen turning ON
// 			-- 10: Screen turning OFF
// 			-- 11: Screen ON

// 	-- Slave Register 2 (slv_reg2) (WRITE)
// 		-- Bits 31:10 : Reserved
// 		-- Bit 9      : SPI_TRIGGER (W) Control signal to trigger sending the BYTE to the screen via SPI
// 		-- Bit 8      : DC_SELECT (W) Control signal to select Data/Command for the BYTE to send
// 			-- 0: Data
// 			-- 1: Command
// 		-- Bits 7:0   : BYTE (W) Byte to send to the screen via SPI

// 	-- Slave Register 3 (slv_reg3) (READ)
// 		-- Bits 31:2  : Reserved
// 		-- Bit 1      : SPI_DATA_REQUEST (R) Status signal to indicate that the screen_controller is ready to receive a new BYTE to send via SPI
// 		-- Bit 0      : SPI_READY (R) Status signal to indicate that the screen_controller has finished sending the previous BYTE via SPI

#endif // SCREEN_REGISTERS_H