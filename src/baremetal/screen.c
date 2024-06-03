#include "screen.h"

void screenBegin(screenInstance *screen){
	screen->colorDepth = 2; //Default

    init_platform();
	writeOnOff(true);
}

void screenEnd(screenInstance *screen){

	clearScreen();
    cleanup_platform();
	writeOnOff(false);
}

void writeOnOff(bool value){

    Xil_Out32(XPAR_SCREEN_0_S00_AXI_BASEADDR, value);
}

uint8_t readOnOffStatus() {

	return ( (Xil_In8(XPAR_SCREEN_0_S00_AXI_BASEADDR + 4) >> 1) );
}

bool readReady(){

	return ( Xil_In8(XPAR_SCREEN_0_S00_AXI_BASEADDR + 4) & 0b00000001);
}

void sendCommand(uint8_t comm){

	// Data
	Xil_Out32(XPAR_SCREEN_0_S00_AXI_BASEADDR + 8, comm);
	// DataCommand
	Xil_Out32(XPAR_SCREEN_0_S00_AXI_BASEADDR + 12, COMMAND);
	// Start
	Xil_Out32(XPAR_SCREEN_0_S00_AXI_BASEADDR, 0b11);
	usleep(20);
}

void sendMultiCommand(uint8_t *comm, int n){

	for(int i = 0; i < n; i++){
		sendCommand(comm[i]);
	}
}

void sendData(uint8_t data){

	// Data
	Xil_Out32(XPAR_SCREEN_0_S00_AXI_BASEADDR + 8, data);
	// DataCommand
	Xil_Out32(XPAR_SCREEN_0_S00_AXI_BASEADDR + 12, DATA);
	// Start
	Xil_Out32(XPAR_SCREEN_0_S00_AXI_BASEADDR, 0b11);
	usleep(20);
}

void sendMultiData(uint8_t *data, int n){

	for(int i = 0; i < n; i++){
		sendData(data[i]);
	}
}

void sendPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t colorDepth){

	uint32_t data = 0;
	uint8_t byte[3] = {0};

	switch (colorDepth){
		case 1:
			data = (uint8_t)( (r>>2) << 5 | (g>>3) << 2 | (b>>3) );
			sendData(data);
			break;
		case 2:
			data = ( (r) << 11 | (g) << 5 | (b) );
			byte[0] = (uint8_t)(data >> 8);
			byte[1] = (uint8_t)(data & 0x000000FF);
			sendMultiData(byte, colorDepth);
			break;
		case 3:
			data = ( (r) << 17 | (g) << 8 | (b) << 1);
			byte[0] = (uint8_t) (data >> 16);
			byte[1] = (uint8_t) ((data & 0x0000FF00) >> 8);
			byte[2] = (uint8_t) (data & 0x000000FF);
			sendMultiData(byte, colorDepth);
			break;
		default:
			data = 0xFFFFFFFF;
			break;
	}
}

void sendMultiPixel(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t colorDepth, int n){

	for(int i = 0; i < n; i++){
		sendPixel(r[i], g[i], b[i], colorDepth);
	}
}

void setColorDepth(screenInstance *screen, uint8_t colorDepth){

	screen->colorDepth = colorDepth;

	uint8_t command[2];
	command[0] = CMD_REMAP;
	command[1] = 0x32 | ((colorDepth - 1) << 6);
	sendMultiCommand(command, 2);
}

void clearScreen(){

	clearWindow(0, 0, N_COLUMNS-1, N_ROWS-1);
}

void drawBitmap(screenInstance *screen, uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint8_t *r, uint8_t *g, uint8_t *b){
	setRowAddress(r1, r2);
	setColumnAddress(c1, c2);
	sendMultiPixel(r, g, b, screen->colorDepth, (r2-r1+1)*(c2-c1+1));
}

void setColumnAddress(uint8_t cBegin, uint8_t cEnd){

	uint8_t command[3];
	command[0] = CMD_SETCOLUMNADDRESS;
	command[1] = cBegin;
	command[2] = cEnd;
	sendMultiCommand(command, 3);
}

void setRowAddress(uint8_t rBegin, uint8_t rEnd){

	uint8_t command[3];
	command[0] = CMD_SETROWADDRESS;
	command[1] = rBegin;
	command[2] = rEnd;
	sendMultiCommand(command, 3);
}

void drawLine(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint8_t r, uint8_t g, uint8_t b){

	uint8_t command[8];
	command[0] = CMD_DRAWLINE;
	command[1] = c1;
	command[2] = r1;
	command[3] = c2;
	command[4] = r2;
	command[5] = r << 1;
	command[6] = g;
	command[7] = b << 1;
	sendMultiCommand(command, 8);
}

void drawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint8_t rLine, uint8_t gLine, uint8_t bLine, uint8_t rFill, uint8_t gFill, uint8_t bFill){

	uint8_t command[11];
	command[0] = CMD_DRAWRECTANGLE;
	command[1] = c1;
	command[2] = r1;
	command[3] = c2;
	command[4] = r2;
	command[5] = rLine << 1;
	command[6] = gLine;
	command[7] = bLine << 1;
	command[8] = rFill << 1;
	command[9] = gFill;
	command[10] = bFill << 1;
	sendMultiCommand(command, 11);
}

void copyWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint8_t c3, uint8_t r3){

	uint8_t command[7];
	command[0] = CMD_COPYWINDOW;
	command[1] = c1;
	command[2] = r1;
	command[3] = c2;
	command[4] = r2;
	command[5] = c3;
	command[6] = r3;
	sendMultiCommand(command, 7);
}

void clearWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2){

	uint8_t command[5];
	command[0] = CMD_CLEARWINDOW;
	command[1] = c1;
	command[2] = r1;
	command[3] = c2;
	command[4] = r2;
	sendMultiCommand(command, 5);
}

void enableFill(bool fillRectangle, bool reverseCopy){
	uint8_t command[2];
	command[0] = CMD_FILLENABLEDISABLE;
	command[1] = 0x00 | (reverseCopy << 4) | (fillRectangle);
	sendMultiCommand(command, 2);
}

void setupScrolling(uint8_t horizontalScrollOffset, uint8_t rowStart, uint8_t rowsNumber, uint8_t verticalScrollOffset, uint8_t timeInterval){

	uint8_t command[6];
	command[0] = CMD_SETUPSCROLLING;
	command[1] = horizontalScrollOffset;
	command[2] = rowStart;
	command[3] = rowsNumber;
	command[4] = verticalScrollOffset;
	command[5] = timeInterval;
	sendMultiCommand(command, 6);
}

void enableScrolling(bool value){

	uint8_t command = value ? CMD_ENABLESCROLLING : CMD_DISABLESCROLLING;
	sendCommand(command);
}
