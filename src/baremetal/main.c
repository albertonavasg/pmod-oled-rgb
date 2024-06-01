#include <stdio.h>	    // For printf
#include <stdbool.h>    // For bool
#include <stdint.h>     // For uint8_t
#include <sleep.h>
#include <stdlib.h>

#include "platform.h"   // For UART
#include "xil_printf.h" // For xil_print

#include "xparameters.h" //For IP Addresses
#include "xil_io.h"		 //For IO
#include "screen.h"		 // For custom functions

int main() {

    init_platform();

    writeOnOff(true);
    sleep(2);

    sendCommand(0xA5);
    sleep(2);
    sendCommand(0xA6);
    sleep(2);
    sendCommand(0xA4);
    sleep(2);

    setColorDepth(1);
    for(int i = 0; i < N_PIXELS; i++){
    	sendPixel(R_MAX, 0, 0, 1);
    }
    sleep(2);

    setColorDepth(2);
    for(int i = 0; i < N_PIXELS; i++){
    	sendPixel(0, G_MAX, 0, 2);
    }
    sleep(2);

    setColorDepth(3);
    for(int i = 0; i < N_PIXELS; i++){
    	sendPixel(0, 0, B_MAX, 3);
    }
    sleep(2);

    writeOnOff(false);

    cleanup_platform();

    return 0;
}
