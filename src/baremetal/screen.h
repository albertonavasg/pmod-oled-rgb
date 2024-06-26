#ifndef SCREENH
#define SCREENH

#include <stdbool.h>     // For bool
#include <stdint.h>      // For uint8_t
#include "string.h"		 // For strlen()

#include "platform.h"    // For UART
#include <xparameters.h> // For Screen Controller Memory Addresses
#include <xil_io.h>      // For IO
#include "xil_printf.h"  // For printf and xil_print
#include <sleep.h>       // For sleep()

// SIZE OF THE SCREEN
#define N_ROWS 64
#define N_COLUMNS 96
#define N_PIXELS 64*96

// ON_OFF_STATUS
#define OFF 0
#define TURNING_ON 1
#define TURNING_OFF 2
#define ON 3

// DATA_COMMAND
#define COMMAND 0
#define DATA 1

// COLOR MAX (565 Format RGB)
#define R_MAX 31
#define G_MAX 63
#define B_MAX 31

// Address increment
#define HORIZONTAL 0
#define VERTICAL 1

// CURSOR MAX
#define MAX_CURSOR_X 12 // N_COLUMNS/8
#define MAX_CURSOR_Y 8  // N_ROWS/8

// COMMANDS
#define CMD_SETCOLUMNADDRESS           0x15
#define CMD_SETROWADDRESS              0x75
#define CMD_SETCONTRASTA               0x81
#define CMD_SETCONTRASTB               0x82
#define CMD_SETCONTRASTC               0x83
#define CMD_MASTERCURRENTCONTROL       0x87
#define CMD_SETPRECHARGESPEEDA         0x8A
#define CMD_SETPRECHARGESPEEDB         0x8B
#define CMD_SETPRECHARGESPEEDC         0x8C
#define CMD_REMAPCOLORDEPTH            0xA0
#define CMD_SETDISPLAYSTARTLINE        0xA1
#define CMD_SETDISPLAYOFFSET           0xA2
#define CMD_NORMALDISPLAY              0xA4
#define CMD_ENTIREDISPLAYON            0xA5
#define CMD_ENTIREDISPLAYOFF           0xA6
#define CMD_INVERSEDISPLAY             0xA7
#define CMD_SETMULTIPLEXRATIO          0xA8
#define CMD_DIMMODESETTING             0xAB
#define CMD_SETMASTERCONFIGURATION     0xAD
#define CMD_DIMMODEDISPLAYON           0xAC
#define CMD_DISPLAYOFF                 0xAE
#define CMD_DISPLAYON                  0xAF
#define CMD_POWERSAVEMODE              0xB0
#define CMD_PHASEPERIODADJUSTMENT      0xB1
#define CMD_DISPLAYCLOCKDIV            0xB3
#define CMD_SETGRAYSCALETABLE          0xB8
#define CMD_ENABLELINEARGRAYSCALETABLE 0xB9
#define CMD_SETPRECHARGEVOLTAGE        0xBB
#define CMD_SETVVOLTAGE                0xBE
#define CMDSETCOMMANDLOCK			   0xFD
#define CMD_DRAWLINE                   0x21
#define CMD_DRAWRECTANGLE              0x22
#define CMD_COPYWINDOW                 0x23
#define CMD_DIMWINDOW                  0x24
#define CMD_CLEARWINDOW                0x25
#define CMD_FILLENABLEDISABLE          0x26
#define CMD_SETUPSCROLLING             0x27
#define CMD_DISABLESCROLLING           0x2E
#define CMD_ENABLESCROLLING            0x2F

// Screen instance
typedef struct{
	uint8_t colorDepth;
	uint8_t addressIncrement;
	uint8_t remapColorDepthSetting;
	uint8_t cursorX;
	uint8_t cursorY;
} screenInstance;

// Color definition
typedef struct{
	uint8_t r;
	uint8_t g;
	uint8_t b;
} colorInstance;

// Functions

// Basic (to send commands and data to the screen)
void screenBegin(screenInstance *screen);

void screenEnd(screenInstance *screen);

void writeOnOff(bool value);

uint8_t readOnOffStatus();

bool readReady();

void sendCommand(uint8_t comm);

void sendMultiCommand(uint8_t *comm, int n);

void sendData(uint8_t data);

void sendMultiData(uint8_t *data, int n);


// Custom (to have high level utilities)
void sendPixel(screenInstance *screen, colorInstance color);

void sendMultiPixel(screenInstance *screen, colorInstance *color, int n);

void clearScreen();

void drawBitmap(screenInstance *screen, uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, colorInstance *color);

void drawSymbol(screenInstance *screen, uint8_t symbol, colorInstance color);

void drawString(screenInstance *screen, char *symbol, colorInstance color);

void setCursor(screenInstance *screen, uint8_t x, uint8_t y);

void incrementCursor(screenInstance *screen);

void setDefaultSettings(screenInstance *screen);

void setColorDepth(screenInstance *screen, uint8_t colorDepth);

void setAddressIncrement(screenInstance *screen, uint8_t addressIncrement);

colorInstance* getSymbolBitmap(uint8_t symbol, colorInstance color, uint8_t *font);

// Standard (Settings from the datasheet)
void setColumnAddress(uint8_t cBegin, uint8_t cEnd);
void setRowAddress(uint8_t rBegin, uint8_t rEnd);

void setContrastA();
void setContrastB();
void setContrastC();
void masterCurrentControl();
void setSecondPrechargeSpeedABC();
void setRemapAndColorSetting(uint8_t value);
void setDisplayStartLine();
void setDisplayOffset();
void setDisplayMode();
void setMultiplexRatio();
void dimModeSetting();
void setMasterConfiguration();
void setDisplayOnOff();
void powerSaveMode();
void period12PeriodAdjustment();
void displayClockDivider();
void setGrayScaleTable();
void enableLinearGrayScaleTable();
void setPrechargeLevel();
void setVcomh();
void setCommandLock();

void drawLine(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, colorInstance color);

void drawRectangle(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, colorInstance colorLine, colorInstance colorFill);

void copyWindow(uint8_t c1, uint8_t r1, uint8_t c2, uint8_t r2, uint8_t c3, uint8_t r3);

void clearWindow(uint8_t r1, uint8_t c1, uint8_t r2, uint8_t c2);

void enableFill(bool fillRectangle, bool reverseCopy);

void setupScrolling(uint8_t horizontalScrollOffset, uint8_t rowStart, uint8_t rowsNumber, uint8_t verticalScrollOffset, uint8_t timeInterval);

void enableScrolling(bool value);


#endif

/* REGISTER MAPPING

Register 0 BIT 1 - START
		   BIT 0 - ON_OFF

Register 1 BIT 2 - ON_OFF_STATUS(1)
		   BIT 1 - ON_OFF_STATUS(0)
		   BIT 0 - READY

Register 2 BIT 7 - DATA(7)
		   BIT 6 - DATA(6)
		   BIT 5 - DATA(5)
		   BIT 4 - DATA(4)
		   BIT 3 - DATA(3)
		   BIT 2 - DATA(2)
		   BIT 1 - DATA(1)
		   BIT 0 - DATA(0)

Register 3 BIT 0 - DATA_COMMAND

*/
