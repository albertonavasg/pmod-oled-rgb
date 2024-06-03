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
void testScreen();
void testColorDepth();
void testScrolling();
void testDrawLine();
void testDrawRectangle();
void testCopy();

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

	testScreen();
	testColorDepth();
	testScrolling();
	testDrawLine();
	testDrawRectangle();
	testCopy();
}

void testScreen(){

	sleep(1);
	sendCommand(CMD_ENTIREDISPLAYON);
	sleep(1);
	sendCommand(CMD_NORMALDISPLAY);
	clearScreen();
}

void testColorDepth(){

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
	clearScreen();
}

void testScrolling(){

	sleep(1);
	setColorDepth(&screen, 2);
	for(int i = 0; i < N_PIXELS; i++){
		if( (i%N_COLUMNS < N_COLUMNS/2 && i < N_PIXELS/2) || (i%N_COLUMNS > N_COLUMNS/2 && i > N_PIXELS/2) ){
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

	setupScrolling(1, 0, N_ROWS, 0, 0b00);;
	enableScrolling(true);
	sleep(2);
	enableScrolling(false);
	sleep(1);
	setupScrolling(0, 0, 64, 1, 0b00);
	enableScrolling(true);
	sleep(2);
	enableScrolling(false);
	sleep(1);
	clearScreen();
}

void testDrawLine(){

	sleep(1);
	drawLine(2, 10, 90, 10, R_MAX, 0, 0);
	sleep(1);
	drawLine(2, 30, 90, 30, 0, G_MAX, 0);
	sleep(1);
	drawLine(2, 50, 90, 50, 0, 0, B_MAX);
	sleep(1);
	drawLine(2, 10, 2, 50, R_MAX, G_MAX, 0);
	sleep(1);
	drawLine(40, 10, 40, 50, 0, G_MAX, B_MAX);
	sleep(1);
	drawLine(90, 10, 90, 50, R_MAX, 0, B_MAX);
	sleep(1);
	clearScreen();
}

void testDrawRectangle(){

	sleep(1);
	drawRectangle(2, 10, 90, 20, R_MAX, 0, 0, 0, 0, 0);
	sleep(1);
	drawRectangle(2, 30, 90, 40, 0, G_MAX, 0, 0, 0, 0);
	sleep(1);
	drawRectangle(2, 50, 90, 60, 0, 0, B_MAX, 0, 0, 0);
	sleep(1);

	clearScreen();
	sleep(1);

	drawRectangle(30, 10, 40, 60, 0, G_MAX, B_MAX, 0, 0, 0);
	sleep(1);
	drawRectangle(10, 25, 80, 45, R_MAX, G_MAX, 0, 0, 0, 0);
	sleep(1);

	clearScreen();
	sleep(1);

	enableFill(true, false);
	drawRectangle(2, 2, 22, 60, R_MAX, 0, 0, 0, B_MAX, 0);
	sleep(1);
	drawRectangle(32, 2, 52, 60, 0, G_MAX, 0, 0, 0, B_MAX);
	sleep(1);
	drawRectangle(62, 2, 82, 60, 0, 0, B_MAX, R_MAX, 0, 0);
	sleep(1);
	enableFill(false, false);
	clearScreen();
}

void testCopy(){

	sleep(1);
	drawRectangle(2, 10, 90, 20, R_MAX, 0, 0, R_MAX, 0, 0);
	sleep(1);
	copyWindow(2, 10, 90, 20, 2, 30);
	sleep(1);

	clearScreen();
	sleep(1);

	drawRectangle(5, 5, 25, 25, 0, G_MAX, 0, 0, 0, 0);
	sleep(1);
	copyWindow(5, 5, 25, 25, 15, 15);
	sleep(1);
	copyWindow(5, 5, 35, 35, 50, 5);
	sleep(1);
	clearScreen();
}
