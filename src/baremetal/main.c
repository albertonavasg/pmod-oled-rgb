#include <stdio.h>	    // For printf
#include <stdbool.h>    // For bool
#include <stdint.h>     // For uint8_t
#include <sleep.h>      // For sleep()
#include <stdlib.h>     // For rand()
#include <string.h>		// For strlen()

#include "platform.h"   // For UART
#include "xil_printf.h" // For xil_print

#include "xparameters.h" //For IP Addresses
#include "xil_io.h"		 //For IO
#include "screen.h"		 // For custom functions
#include "bitmap.h"

void test();
void testScreen();
void testColorDepth();
void testAddressIncrement();
void testScrolling();
void testDrawLine();
void testDrawRectangle();
void testCopy();
void testColumnRowAddress();
void testDrawBitmap();
void testCursor();
void testDrawSymbol();
void testDrawString();

screenInstance screen;

colorInstance color[N_PIXELS];

int main() {

	screenBegin(&screen);
    test();
    screenEnd(&screen);

    return 0;
}

void test(){

	testScreen();
	testColorDepth();
	testAddressIncrement();
	testScrolling();
	testDrawLine();
	testDrawRectangle();
	testCopy();
	testColumnRowAddress();
	testDrawBitmap();
	testCursor();
	testDrawSymbol();
	testDrawString();
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
		color[i].r = R_MAX;
		color[i].g = 0;
		color[i].b = 0;
	}
	sendMultiPixel(&screen, color, N_PIXELS);
	sleep(1);

	setColorDepth(&screen, 2);
	for(int i = 0; i < N_PIXELS; i++){
		color[i].r = 0;
		color[i].g = G_MAX;
		color[i].b = 0;
	}
	sendMultiPixel(&screen, color, N_PIXELS);
	sleep(1);

	setColorDepth(&screen, 3);
	for(int i = 0; i < N_PIXELS; i++){
		color[i].r = 0;
		color[i].g = 0;
		color[i].b = B_MAX;
	}
	sendMultiPixel(&screen, color, N_PIXELS);
	sleep(1);

	srand(0);
	setColorDepth(&screen, 2);
	for(int i = 0; i < N_PIXELS; i++){
		color[i].r = rand()%R_MAX;
		color[i].g = rand()%G_MAX;
		color[i].b = rand()%B_MAX;
	}
	sendMultiPixel(&screen, color, N_PIXELS);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen();
}

void testAddressIncrement(){

	sleep(1);
	setColorDepth(&screen, 2);
	setAddressIncrement(&screen, HORIZONTAL);
	for(int i = 0; i < N_PIXELS; i++){
		color[i].r = R_MAX;
		color[i].g = 0;
		color[i].b = 0;
	}
	sendMultiPixel(&screen, color, N_PIXELS);
	sleep(1);

	setAddressIncrement(&screen, VERTICAL);
	for(int i = 0; i < N_PIXELS; i++){
		color[i].r = 0;
		color[i].g = 0;
		color[i].b = B_MAX;
	}
	sendMultiPixel(&screen, color, N_PIXELS);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen();
}

void testScrolling(){

	sleep(1);
	setColorDepth(&screen, 2);
	for(int i = 0; i < N_PIXELS; i++){
		if( (i%N_COLUMNS < N_COLUMNS/2 && i < N_PIXELS/2) || (i%N_COLUMNS > N_COLUMNS/2 && i > N_PIXELS/2) ){
			color[i].r = R_MAX;
			color[i].g = 0;
			color[i].b = 0;
		}
		else{
			color[i].r = 0;
			color[i].g = 0;
			color[i].b = B_MAX;
		}
	}
	sendMultiPixel(&screen, color, N_PIXELS);
	sleep(1);

	setupScrolling(1, 0, N_ROWS, 0, 0b00);;
	enableScrolling(true);
	sleep(2);
	enableScrolling(false);
	sleep(1);
	setupScrolling(0, 0, 0, 1, 0b00);
	enableScrolling(true);
	sleep(2);
	enableScrolling(false);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen();
}

void testDrawLine(){

	sleep(1);
	drawLine(2, 10, 90, 10, R_MAX, 0, 0);
	usleep(500*1000);
	drawLine(2, 30, 90, 30, 0, G_MAX, 0);
	usleep(500*1000);
	drawLine(2, 50, 90, 50, 0, 0, B_MAX);
	usleep(500*1000);
	drawLine(2, 10, 2, 50, R_MAX, G_MAX, 0);
	usleep(500*1000);
	drawLine(45, 10, 45, 50, 0, G_MAX, B_MAX);
	usleep(500*1000);
	drawLine(90, 10, 90, 50, R_MAX, 0, B_MAX);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen();
}

void testDrawRectangle(){

	sleep(1);
	drawRectangle(2, 10, 90, 20, R_MAX, 0, 0, 0, 0, 0);
	usleep(500*1000);
	drawRectangle(2, 30, 90, 40, 0, G_MAX, 0, 0, 0, 0);
	usleep(500*1000);
	drawRectangle(2, 50, 90, 60, 0, 0, B_MAX, 0, 0, 0);
	sleep(1);

	clearScreen();
	sleep(1);

	drawRectangle(30, 10, 40, 60, 0, G_MAX, B_MAX, 0, 0, 0);
	usleep(500*1000);
	drawRectangle(10, 25, 80, 45, R_MAX, G_MAX, 0, 0, 0, 0);
	sleep(1);

	clearScreen();
	sleep(1);

	enableFill(true, false);
	drawRectangle(2, 2, 22, 60, R_MAX, 0, 0, 0, B_MAX, 0);
	usleep(500*1000);
	drawRectangle(32, 2, 52, 60, 0, G_MAX, 0, 0, 0, B_MAX);
	usleep(500*1000);
	drawRectangle(62, 2, 82, 60, 0, 0, B_MAX, R_MAX, 0, 0);
	sleep(1);
	enableFill(false, false);

	setDefaultSettings(&screen);
	clearScreen();
}

void testCopy(){

	sleep(1);
	drawRectangle(2, 10, 90, 20, R_MAX, 0, 0, R_MAX, 0, 0);
	usleep(500*1000);
	copyWindow(2, 10, 90, 20, 2, 30);
	sleep(1);

	clearScreen();
	sleep(1);

	drawRectangle(5, 5, 25, 25, 0, G_MAX, 0, 0, 0, 0);
	usleep(500*1000);
	copyWindow(5, 5, 25, 25, 15, 15);
	usleep(500*1000);
	copyWindow(5, 5, 35, 35, 50, 5);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen();
}

void testColumnRowAddress(){

	sleep(1);
	setColumnAddress(20, 29);
	setRowAddress(20, 29);
	for(int i = 0; i < 100; i++){
		color[i].r = R_MAX;
		color[i].g = 0;
		color[i].b = 0;
	}
	sendMultiPixel(&screen, color, 100);
	usleep(500*1000);;

	setColumnAddress(50, 59);
	setRowAddress(50, 59);
	for(int i = 0; i < 100; i++){
		color[i].r = 0;
		color[i].g = G_MAX;
		color[i].b = 0;
	}
	sendMultiPixel(&screen, color, 100);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen();
}

void testDrawBitmap(){

	sleep(1);
	setColorDepth(&screen, 1);
	drawBitmap(&screen, 0, 0, N_COLUMNS-1, N_ROWS-1, imageBitmap);
	sleep(2);
	setColorDepth(&screen, 2);
	drawBitmap(&screen, 0, 0, N_COLUMNS-1, N_ROWS-1, imageBitmap);
	sleep(2);
	setColorDepth(&screen, 3);
	drawBitmap(&screen, 0, 0, N_COLUMNS-1, N_ROWS-1, imageBitmap);
	sleep(2);
	setColorDepth(&screen, 2);

	setDefaultSettings(&screen);
	clearScreen();
}

void testCursor(){

	sleep(1);
	setCursor(&screen, 0, 0);
	for(int i = 0; i < (MAX_CURSOR_X+1)*(MAX_CURSOR_Y+1); i++){
		for(int j = 0; j < 64; j++){
				color[j].r = R_MAX * (i%2 == 0);
				color[j].g = G_MAX * (i%4 == 0);
				color[j].b = B_MAX;
		}
		drawBitmap(&screen, 8*screen.cursorX, 8*screen.cursorY, 8*screen.cursorX + 7, 8*screen.cursorY + 7, color);
		usleep(50*1000);
		incrementCursor(&screen);
	}
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen();
}

void testDrawSymbol(){

	sleep(1);
	colorInstance fontColor = {R_MAX, G_MAX, B_MAX};
	setAddressIncrement(&screen, VERTICAL);
	setCursor(&screen, 2, 2);
	drawSymbol(&screen, 'A', fontColor);
	sleep(2);
	clearScreen();

	setDefaultSettings(&screen);
	setCursor(&screen, 0, 0);
}

void testDrawString(){

	sleep(1);
	colorInstance fontColor1 = {R_MAX, 0, B_MAX};
	colorInstance fontColor2 = {0, G_MAX, B_MAX};
	setAddressIncrement(&screen, VERTICAL);
	setCursor(&screen, 0, 0);
	char symbol1[] = "Alberto";
	char symbol2[] = "Angel";
	drawString(&screen, symbol1, fontColor1);
	setCursor(&screen, 0, 1);
	drawString(&screen, symbol2, fontColor2);
	sleep(2);

	setDefaultSettings(&screen);
	clearScreen();
}
