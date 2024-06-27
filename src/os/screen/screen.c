#include "screen.h"

// BASIC ---------------------------------------------------------
void screenBegin(screenInstance *screen){
	
	// Open file descriptor
	fd = open("/dev/uio0",O_RDWR);

	// Map physical memory into virtual memory (UIO -> offset = 0)
	screenAddress = mmap(NULL, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, fd,0);
	
	setDefaultSettings(screen);

	writeOnOff(true);
}

void screenEnd(screenInstance *screen){

	clearScreen();
	
	writeOnOff(false);
	
	// Unmap memory
	munmap(screenAddress, 0x10000);
	
	// Close file descriptor
	close(fd);
}

void writeOnOff(bool value){

    screenAddress[0] = value;
}

uint8_t readOnOffStatus() {

	return ( screenAddress[1] >> 1 );
}

bool readReady(){

	return (screenAddress[1] & 0b00000001);
}

void sendCommand(uint8_t comm){

	// Data
	screenAddress[2] = comm;
	// DataCommand
	screenAddress[3] = COMMAND;
	// Start
	screenAddress[0] = 0b11;
	usleep(20);
}

void sendMultiCommand(uint8_t *comm, int n){

	for(int i = 0; i < n; i++){
		sendCommand(comm[i]);
	}
}

void sendData(uint8_t data){

	// Data
	screenAddress[2] = data;
	// DataCommand
	screenAddress[3] = DATA;
	// Start
	screenAddress[0] = 0b11;
	usleep(20);
}

void sendMultiData(uint8_t *data, int n){

	for(int i = 0; i < n; i++){
		sendData(data[i]);
	}
}

// CUSTOM ---------------------------------------------------------
void sendPixel(screenInstance *screen, colorInstance color){

	uint32_t data = 0;
	uint8_t byte[3] = {0};

	switch (screen->colorDepth){
		case 1:
			data = (uint8_t)( (color.r>>2) << 5 | (color.g>>3) << 2 | (color.b>>3) );
			sendData(data);
			break;
		case 2:
			data = ( (color.r) << 11 | (color.g) << 5 | (color.b) );
			byte[0] = (uint8_t)(data >> 8);
			byte[1] = (uint8_t)(data & 0x000000FF);
			sendMultiData(byte, screen->colorDepth);
			break;
		case 3:
			data = ( (color.r) << 17 | (color.g) << 8 | (color.b) << 1);
			byte[0] = (uint8_t) (data >> 16);
			byte[1] = (uint8_t) ((data & 0x0000FF00) >> 8);
			byte[2] = (uint8_t) (data & 0x000000FF);
			sendMultiData(byte, screen->colorDepth);
			break;
		default:
			data = 0xFFFFFFFF;
			break;
	}
}

void sendMultiPixel(screenInstance *screen, colorInstance *color, int n){

	for(int i = 0; i < n; i++){
		sendPixel(screen, color[i]);
	}
}

void clearScreen(){

	clearWindow(0, 0, N_COLUMNS-1, N_ROWS-1);
}

void drawBitmap(screenInstance *screen, uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, colorInstance *color){
	setRowAddress(r1, r2);
	setColumnAddress(c1, c2);
	sendMultiPixel(screen, color, (r2-r1+1)*(c2-c1+1));
}

void drawSymbol(screenInstance *screen, uint8_t symbol, colorInstance color, uint8_t *font){
	
	colorInstance symbolBitmap[8*8];
	getSymbolBitmap(symbol, color, font, symbolBitmap);
	drawBitmap(screen, screen->cursorX*8, screen->cursorY*8, screen->cursorX*8 + 7, screen->cursorY*8 + 7, symbolBitmap);
}

void drawString(screenInstance *screen, char *symbol, colorInstance color, uint8_t *font){

	for(int i = 0; i < strlen(symbol); i++){
		drawSymbol(screen, symbol[i], color, font);
		incrementCursor(screen);
	}
}

void setCursor(screenInstance *screen, uint8_t x, uint8_t y){

	if(x < 0){
		screen->cursorX = 0;
	}
	else if(x > MAX_CURSOR_X - 1){
		screen->cursorX = MAX_CURSOR_X - 1;
	}
	else{
		screen->cursorX = x;
	}

	if(y < 0){
		screen->cursorY = 0;
	}
	else if(y > MAX_CURSOR_Y - 1){
		screen->cursorY = MAX_CURSOR_Y - 1;
	}
	else{
		screen->cursorY = y;
	}

}

void incrementCursor(screenInstance *screen){

	if(screen->cursorX < MAX_CURSOR_X - 1){
		screen->cursorX++;
	}
	else if (screen->cursorY < MAX_CURSOR_Y - 1){
		screen->cursorX = 0;
		screen->cursorY++;
	}
	else{
		screen->cursorX = 0;
		screen->cursorY = 0;
	}
}

void setDefaultSettings(screenInstance *screen){

	screen->colorDepth = 2;
	screen->addressIncrement = HORIZONTAL;
	screen->remapColorDepthSetting = 0x72;
	setRemapAndColorSetting(screen->remapColorDepthSetting);

	setColumnAddress(0, N_COLUMNS-1);
	setRowAddress(0, N_ROWS-1);

	setCursor(screen, 0, 0);

	enableFill(false, false);
	enableScrolling(false);
}

void setColorDepth(screenInstance *screen, uint8_t colorDepth){

	screen->colorDepth = colorDepth;
	screen->remapColorDepthSetting =  (screen->remapColorDepthSetting & 0b00111111) | ((colorDepth - 1) << 6);
	setRemapAndColorSetting(screen->remapColorDepthSetting);
}

void setAddressIncrement(screenInstance *screen, uint8_t addressIncrement){

	screen->addressIncrement = addressIncrement;
	screen->remapColorDepthSetting = (screen->remapColorDepthSetting & 0b1111111) | addressIncrement;
	setRemapAndColorSetting(screen->remapColorDepthSetting);
}

// HELPERS ---------------------------------------------------------
void getImageBitmap(char *imagePath, colorInstance *imageBitmap){
	
	int r, g, b = 0;
	FILE *imageFile;
	char line[256];
	char imageName[256];

	imageFile = fopen(imagePath, "r");

	if(imageFile == NULL){
		printf("\nError opening : %s", imagePath);
		fclose(imageFile);
	}
	else{
		// First line is the name of the image
		if (fgets(line,sizeof(line), imageFile) != NULL){
			strcpy(imageName, line);
		}
		else{
			printf("\nError reading : %s", imagePath);
		}
		// Second line is "R G B" (useless header, ignored)
		if (fgets(line, sizeof(line), imageFile) == NULL) {
        	printf("Not enough lines in file: %s\n", imageName);
        	fclose(imageFile);
		}
		// Remaining lines are RGB values of the pixels
		int i = 0;
		while (fgets(line, sizeof(line), imageFile) != NULL) {
			sscanf(line, "%d %d %d", &r, &g, &b);
			imageBitmap[i].r = r;
			imageBitmap[i].g = g;
			imageBitmap[i].b = b;
			i++;
		}
		fclose(imageFile);
	}
}

void importFont(char *fontPath, uint8_t *font){
	uint32_t readSymbol[8] = {0};
	uint8_t symbol[8] = {0};
	FILE *fontFile;
	char line[256];
	char fontName[256];

	fontFile = fopen(fontPath, "r");

	if(fontFile == NULL){
		printf("\nError opening : %s", fontPath);
		fclose(fontFile);
	}
	else{
		// First line is the name of the font
		if (fgets(line,sizeof(line), fontFile) != NULL){
			strcpy(fontName, line);
		}
		else{
			printf("\nError reading : %s", fontPath);
		}
		//Remaining lines are the 8 byte values of the characters
		for (int i = 0; i < 128; i++){
			fgets(line,sizeof(line), fontFile);
			sscanf(line, "0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x", &readSymbol[0], &readSymbol[1], &readSymbol[2], &readSymbol[3], &readSymbol[4], &readSymbol[5], &readSymbol[6], &readSymbol[7]);
			for (int j = 0; j < 8; j++){
				symbol[j] = (uint8_t) readSymbol[j];
			}
			for (int j = 0; j < 8; j++){
				font[8*i + j] = symbol[j];
			}
		}
		fclose(fontFile);
	}
}

void getSymbolBitmap(uint8_t symbol, colorInstance color, uint8_t *font, colorInstance *symbolBitmap){

	for(int i = 0; i < 8; i++){
		for(int j = 0; j < 8; j++){
			if( (font[8*symbol + i] >> j) & 1 ){	
				symbolBitmap[8*i + j] = color;
			}
			else{
				symbolBitmap[8*i + j].r = 0;
				symbolBitmap[8*i + j].g = 0;
				symbolBitmap[8*i + j].b = 0;
			}
		}
	}
}

// STANDARD ---------------------------------------------------------
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

void setRemapAndColorSetting(uint8_t value){
	uint8_t command[2];
	command[0] = CMD_REMAPCOLORDEPTH;
	command[1] = value;
	sendMultiCommand(command, 2);
}

void drawLine(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, colorInstance color){

	uint8_t command[8];
	command[0] = CMD_DRAWLINE;
	command[1] = c1;
	command[2] = r1;
	command[3] = c2;
	command[4] = r2;
	command[5] = color.r << 1;
	command[6] = color.g;
	command[7] = color.b << 1;
	sendMultiCommand(command, 8);
}

void drawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, colorInstance colorLine, colorInstance colorFill){

	uint8_t command[11];
	command[0] = CMD_DRAWRECTANGLE;
	command[1] = c1;
	command[2] = r1;
	command[3] = c2;
	command[4] = r2;
	command[5] = colorLine.r << 1;
	command[6] = colorLine.g;
	command[7] = colorLine.b << 1;
	command[8] = colorFill.r << 1;
	command[9] = colorFill.g;
	command[10] = colorFill.b << 1;
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
