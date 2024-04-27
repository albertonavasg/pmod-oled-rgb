#include <stdio.h>	    // For printf
#include <stdbool.h>    // For bool
#include <stdint.h>     // For uint8_t
#include "platform.h"   // For UART
#include "xil_printf.h" // For xil_print

#include "xparameters.h" //For IP Addresses
#include "xil_io.h"		 //For IO

#include "screen.h"		 // For custom functions

int main() {

	uint8_t commands[2] = {0xA5, 0xA6};

    init_platform();

    printf("\n ON OFF STATUS: %d", read_on_off_status());
    printf("\n READY: %d", read_ready());

    write_on_off(true);

    printf("\n ON OFF STATUS: %d", read_on_off_status());
    printf("\n READY: %d", read_ready());

    send_multi_command(commands, 2);

    write_on_off(false);

    cleanup_platform();

    return 0;
}
