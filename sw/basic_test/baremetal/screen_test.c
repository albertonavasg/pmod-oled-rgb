#include "xparameters.h" // For Screen peripherals memory addresses
//#include "xil_io.h"      // For IO (Xil_In, Xil_Out)
#include "sleep.h"       // For sleep()

/*
-- SCREEN IP REGISTER MAP --

	-- Slave Register 0 (slv_reg0) (WRITE)
		-- Bits 31:1  : Reserved
		-- Bit 0      : ON_OFF (W) Control signal to turn ON/OFF the screen

	-- Slave Register 1 (slv_reg1) (READ)
		-- Bits 31:2  : Reserved
		-- Bits 1:0   : ON_OFF_STATUS (R) Status signal to indicate if the status of the screen is ON/OFF
			-- 00: Screen OFF
			-- 01: Screen turning ON
			-- 10: Screen turning OFF
			-- 11: Screen ON

	-- Slave Register 2 (slv_reg2) (WRITE)
		-- Bits 31:10 : Reserved
		-- Bit 9      : SPI_TRIGGER (W) Control signal to trigger sending the BYTE to the screen via SPI
		-- Bit 8      : DC_SELECT (W) Control signal to select Data/Command for the BYTE to send
			-- 0: Data
			-- 1: Command
		-- Bits 7:0   : BYTE (W) Byte to send to the screen via SPI

	-- Slave Register 3 (slv_reg3) (READ)
		-- Bits 31:2  : Reserved
		-- Bit 1      : SPI_DATA_REQUEST (R) Status signal to indicate that the screen_controller is ready to receive a new BYTE to send via SPI
		-- Bit 0      : SPI_READY (R) Status signal to indicate that the screen_controller has finished sending the previous BYTE via SPI
*/

int main() {   
    // Store Screen A and B memory positions
    volatile uint32_t *screenA = (volatile uint32_t *) XPAR_SCREEN_A_BASEADDR;
    volatile uint32_t *screenB = (volatile uint32_t *) XPAR_SCREEN_B_BASEADDR;

    // Write a '1' in ON_OFF of Screen A and Screen_B
    *screenA = 0x01;
    *screenB = 0x01;
    //Xil_Out32(XPAR_SCREEN_A_BASEADDR, 0x01);
    //Xil_Out32(XPAR_SCREEN_B_BASEADDR, 0x01);
    
    sleep(1);

    // Entire Display ON Command
    *(screenA + 2) = 0xA5 | 0 << 8  | 1 << 9;
    *(screenB + 2) = 0xA5 | 0 << 8  | 1 << 9;
    //Xil_Out32(XPAR_SCREEN_A_BASEADDR + 8, 0xA5 | 0 << 8  | 1 << 9);
    //Xil_Out32(XPAR_SCREEN_B_BASEADDR + 8, 0xA5 | 0 << 8  | 1 << 9);

    sleep(1);

    // Entire Display OFF Command
    *(screenA + 2) = 0xA6 | 0 << 8  | 1 << 9;
    *(screenB + 2) = 0xA6 | 0 << 8  | 1 << 9;
    //Xil_Out32(XPAR_SCREEN_A_BASEADDR + 8, 0xA6 | 0 << 8  | 1 << 9);
    //Xil_Out32(XPAR_SCREEN_B_BASEADDR + 8, 0xA6 | 0 << 8  | 1 << 9);

    sleep(1);

    // Write a '0' in ON_OFF of Screen A and Screen_B
    *screenA = 0x00;
    *screenB = 0x00;
    //Xil_Out32(XPAR_SCREEN_A_BASEADDR, 0x00);
    //Xil_Out32(XPAR_SCREEN_B_BASEADDR, 0x00);

    sleep(1);

    return 0;
}