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

void test();

screenInstance screen;

uint8_t r[N_PIXELS];
uint8_t g[N_PIXELS];
uint8_t b[N_PIXELS];

int main() {

	screenBegin(&screen);
    test();
    screenEnd(&screen);

    return 0;
}

void test(){

	sleep(1);
	sendCommand(0xA5);
	sleep(1);
	sendCommand(0xA4);
	clearScreen(0, 0, N_COLUMNS-1, N_ROWS-1);
	sleep(1);

	setColorDepth(&screen, 1);
	for(int i = 0; i < N_PIXELS; i++){
		r[i] = R_MAX;
		g[i] = 0;
		b[i] = 0;
	}
	sendMultiPixel(r, g, b, screen.colorDepth, N_PIXELS);
	sleep(1);

	setColorDepth(&screen, 2);
	for(int i = 0; i < N_PIXELS; i++){
		r[i] = 0;
		g[i] = G_MAX;
		b[i] = 0;
	}
	sendMultiPixel(r, g, b, screen.colorDepth, N_PIXELS);
	sleep(1);

	setColorDepth(&screen, 3);
	for(int i = 0; i < N_PIXELS; i++){
		r[i] = 0;
		g[i] = 0;
		b[i] = B_MAX;
	}
	sendMultiPixel(r, g, b, screen.colorDepth, N_PIXELS);
	sleep(1);

	srand(0);
	setColorDepth(&screen, 2);
	for(int i = 0; i < N_PIXELS; i++){
		r[i] = rand()%R_MAX;
		g[i] = rand()%G_MAX;
		b[i] = rand()%B_MAX;
	}
	sendMultiPixel(r, g, b, 2, N_PIXELS);
	sleep(1);

	setColorDepth(&screen, 2);
	for(int i = 0; i < N_PIXELS; i++){
		if(i%N_COLUMNS < N_COLUMNS/2){
			r[i] = R_MAX;
			g[i] = 0;
			b[i] = 0;
		}
		else{
			r[i] = 0;
			g[i] = 0;
			b[i] = B_MAX;
		}
	}
	sendMultiPixel(r, g, b, screen.colorDepth, N_PIXELS);
	sleep(1);

	setupScrolling(1, 0, N_ROWS, 0, 0b00);
	enableScrolling(true);
	sleep(5);
	enableScrolling(false);
	sleep(1);
}
