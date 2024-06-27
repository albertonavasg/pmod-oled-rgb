#include <stdio.h>	  // For printf
#include <stdbool.h>  // For bool
#include <stdint.h>   // For uint8_t
#include <stdlib.h>   // For rand()
#include <string.h>	  // For strlen()
#include <inttypes.h> // For PRIu8

#include "screen.h"	  // For custom functions

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>

/*
This program is used to test all the implemented functionalities of the screen
*/

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

uint8_t font[128*8];

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
	sendCommand(&screen, CMD_ENTIREDISPLAYON);
	sleep(1);
	sendCommand(&screen, CMD_NORMALDISPLAY);
	sleep(1);

	clearScreen(&screen);
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
	clearScreen(&screen);
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
	clearScreen(&screen);
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

	setupScrolling(&screen, 1, 0, N_ROWS, 0, 0b00); // Horizontal
	enableScrolling(&screen, true);
	sleep(2);
	enableScrolling(&screen, false);
	sleep(1);
	setupScrolling(&screen, 0, 0, 0, 1, 0b00); // Vertical
	enableScrolling(&screen, true);
	sleep(2);
	enableScrolling(&screen, false);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen(&screen);
}

void testDrawLine(){

	sleep(1);
	colorInstance lineColor[6] = {{R_MAX, 0, 0}, {0, G_MAX, 0}, {0, 0, B_MAX}, {R_MAX, G_MAX, 0}, {0, G_MAX, B_MAX}, {R_MAX, 0, B_MAX}};
	drawLine(&screen, 2, 10, 90, 10, lineColor[0]);
	usleep(500*1000);
	drawLine(&screen, 2, 30, 90, 30, lineColor[1]);
	usleep(500*1000);
	drawLine(&screen, 2, 50, 90, 50, lineColor[2]);
	usleep(500*1000);
	drawLine(&screen, 2, 10, 2, 50, lineColor[3]);
	usleep(500*1000);
	drawLine(&screen, 45, 10, 45, 50, lineColor[4]);
	usleep(500*1000);
	drawLine(&screen, 90, 10, 90, 50, lineColor[5]);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen(&screen);
}

void testDrawRectangle(){

	sleep(1);
	colorInstance lineColor[6] = {{R_MAX, 0, 0}, {0, G_MAX, 0}, {0, 0, B_MAX}, {R_MAX, G_MAX, 0}, {0, G_MAX, B_MAX}, {R_MAX, 0, B_MAX}};
	colorInstance fillColor[3] = {{0, G_MAX, 0}, {0, 0, B_MAX}, {R_MAX, 0, 0}};

	enableFill(&screen, false, false);
	drawRectangle(&screen, 2, 10, 90, 20, lineColor[0], fillColor[0]);
	usleep(500*1000);
	drawRectangle(&screen, 2, 30, 90, 40, lineColor[1], fillColor[0]);
	usleep(500*1000);
	drawRectangle(&screen, 2, 50, 90, 60, lineColor[2], fillColor[0]);
	sleep(1);

	clearScreen(&screen);
	sleep(1);

	drawRectangle(&screen, 30, 10, 40, 60, lineColor[3], fillColor[0]);
	usleep(500*1000);
	drawRectangle(&screen, 10, 25, 80, 45, lineColor[4], fillColor[0]);
	usleep(500*1000);
	drawRectangle(&screen, 50, 10, 90, 60, lineColor[5], fillColor[0]);
	sleep(1);

	clearScreen(&screen);
	sleep(1);

	enableFill(&screen, true, false);
	drawRectangle(&screen, 2, 2, 22, 60, lineColor[0], fillColor[0]);
	usleep(500*1000);
	drawRectangle(&screen, 32, 2, 52, 60, lineColor[1], fillColor[1]);
	usleep(500*1000);
	drawRectangle(&screen, 62, 2, 82, 60, lineColor[2], fillColor[2]);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen(&screen);
}

void testCopy(){

	sleep(1);
	colorInstance lineColor[6] = {{R_MAX, 0, 0}, {0, G_MAX, 0}, {0, 0, B_MAX}, {R_MAX, G_MAX, 0}, {0, G_MAX, B_MAX}, {R_MAX, 0, B_MAX}};
	colorInstance fillColor[3] = {{0, G_MAX, 0}, {0, 0, B_MAX}, {R_MAX, 0, 0}};

	drawRectangle(&screen, 2, 10, 90, 20, lineColor[0], fillColor[0]);
	usleep(500*1000);
	copyWindow(&screen, 2, 10, 90, 20, 2, 30);
	sleep(1);

	clearScreen(&screen);
	sleep(1);

	drawRectangle(&screen, 5, 5, 25, 25,lineColor[1], fillColor[1]);
	usleep(500*1000);
	copyWindow(&screen, 5, 5, 25, 25, 15, 15);
	usleep(500*1000);
	copyWindow(&screen, 5, 5, 35, 35, 50, 5);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen(&screen);
}

void testColumnRowAddress(){

	sleep(1);
	setColumnAddress(&screen, 20, 29);
	setRowAddress(&screen, 20, 29);
	for(int i = 0; i < 100; i++){
		color[i].r = R_MAX;
		color[i].g = 0;
		color[i].b = 0;
	}
	sendMultiPixel(&screen, color, 100);
	usleep(500*1000);

	setColumnAddress(&screen, 50, 59);
	setRowAddress(&screen, 50, 59);
	for(int i = 0; i < 100; i++){
		color[i].r = 0;
		color[i].g = G_MAX;
		color[i].b = 0;
	}
	sendMultiPixel(&screen, color, 100);
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen(&screen);
}

void testDrawBitmap(){

	sleep(1);
	getImageBitmap("../images/image1.txt", color);
	setColorDepth(&screen, 1);
	drawBitmap(&screen, 0, 0, N_COLUMNS-1, N_ROWS-1, color);
	sleep(2);
	setColorDepth(&screen, 2);
	drawBitmap(&screen, 0, 0, N_COLUMNS-1, N_ROWS-1, color);
	sleep(2);
	setColorDepth(&screen, 3);
	drawBitmap(&screen, 0, 0, N_COLUMNS-1, N_ROWS-1, color);
	sleep(2);
	setColorDepth(&screen, 2);

	setDefaultSettings(&screen);
	clearScreen(&screen);
}

void testCursor(){

	sleep(1);
	setCursor(&screen, 5, 5);
	for(int i = 0; i < MAX_CURSOR_X*MAX_CURSOR_Y; i++){
		for(int j = 0; j < 64; j++){
				color[j].r = R_MAX * (i%2 == 0);
				color[j].g = G_MAX * (i%3 == 0);
				color[j].b = B_MAX;
		}
		drawBitmap(&screen, 8*screen.cursorX, 8*screen.cursorY, 8*screen.cursorX + 7, 8*screen.cursorY + 7, color);
		usleep(10*1000);
		incrementCursor(&screen);
	}
	sleep(1);

	setDefaultSettings(&screen);
	clearScreen(&screen);
}

void testDrawSymbol(){

	sleep(1);
	importFont("../fonts/font1.txt", font);
	colorInstance fontColor = {R_MAX, G_MAX, B_MAX};
	setAddressIncrement(&screen, VERTICAL);
	setCursor(&screen, 0, 0);
	for(int i = 0; i < 128; i++){
		drawSymbol(&screen, i, fontColor, font);
		incrementCursor(&screen);
		usleep(10*1000);
		if((screen.cursorX == 0) && (screen.cursorY == 0)){
			sleep(5);
			clearScreen(&screen);
			usleep(10*10000);
		}
	}

	sleep(5);
	clearScreen(&screen);

	setDefaultSettings(&screen);
}

void testDrawString(){

	sleep(1);
	colorInstance fontColor[6] = {{R_MAX, 0, 0}, {0, G_MAX, 0}, {0, 0, B_MAX}, {R_MAX, G_MAX, 0}, {0, G_MAX, B_MAX}, {R_MAX, 0, B_MAX}};
	char symbol0[] = "Alberto";
	char symbol1[] = "Navas";
	char symbol2[] = "Angel";
	char symbol3[] = "Jarillo";
	char symbol4[] = "Design";
	char symbol5[] = "of";
	char symbol6[] = "Embedded";
	char symbol7[] = "Systems";

	setAddressIncrement(&screen, VERTICAL);

	setCursor(&screen, 0, 0);
	drawString(&screen, symbol0, fontColor[0], font);
	setCursor(&screen, 1, 1);
	drawString(&screen, symbol1, fontColor[1], font);
	setCursor(&screen, 2, 2);
	drawString(&screen, symbol2, fontColor[2], font);
	setCursor(&screen, 3, 3);
	drawString(&screen, symbol3, fontColor[3], font);
	setCursor(&screen, 0, 4);
	drawString(&screen, symbol4, fontColor[4], font);
	setCursor(&screen, 1, 5);
	drawString(&screen, symbol5, fontColor[5], font);
	setCursor(&screen, 2, 6);
	drawString(&screen, symbol6, fontColor[6], font);
	setCursor(&screen, 3, 7);
	drawString(&screen, symbol7, fontColor[7], font);
	sleep(5);

	setDefaultSettings(&screen);
	clearScreen(&screen);
}
