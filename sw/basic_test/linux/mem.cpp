#include <cstdio>     // perror
#include <cstdint>    // uint32_t
#include <unistd.h>   // sleep
#include <fcntl.h>    // open
#include <sys/mman.h> // mmap, munmap

#define SCREEN_A_BASE_ADDR 0x43C00000
#define SCREEN_B_BASE_ADDR 0x43C10000
#define MAP_SIZE 4096

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

    // Open file descriptor for memory access
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("Failed to open /dev/mem");
        return -1;
    }

    // Map Screen A and B memory addresses
    volatile uint32_t *screenA = (volatile uint32_t *) mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SCREEN_A_BASE_ADDR);
    volatile uint32_t *screenB = (volatile uint32_t *) mmap(NULL, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, SCREEN_B_BASE_ADDR);

    if (!screenA || !screenB) {
        close(fd);
        return -1;
    }

    // Turn ON both screens
    screenA[0] = 0x01;
    screenB[0] = 0x01;

    sleep(1);

    // Entire Display ON command
    screenA[2] = (0xA5) | (0 << 8) | (1 << 9);
    screenB[2] = (0xA5) | (0 << 8) | (1 << 9);

    sleep(1);

    // Entire Display OFF command
    screenA[2] = (0xA6) | (0 << 8) | (1 << 9);
    screenB[2] = (0xA6) | (0 << 8) | (1 << 9);

    sleep(1);

    // Turn OFF both screens
    screenA[0] = 0x00;
    screenB[0] = 0x00;

    sleep(1);

    munmap((void *)screenA, MAP_SIZE);
    munmap((void *)screenB, MAP_SIZE);
    close(fd);

    return 0;
}
