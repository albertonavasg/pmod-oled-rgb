#ifndef SCREENH
#define SCREENH

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"
#include <xparameters.h>
#include <xil_io.h>
#include "xil_printf.h"
#include <sleep.h>

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

// COLOR MAX
#define R_MAX 31
#define G_MAX 63
#define B_MAX 31

typedef struct{
	uint8_t colorDepth;
} screenInstance;

void screenBegin(screenInstance *screen);

void screenEnd(screenInstance *screen);

void writeOnOff(bool value);

uint8_t readOnOffStatus();

bool readReady();

void sendCommand(uint8_t comm);

void sendMultiCommand(uint8_t *comm, int n);

void sendData(uint8_t data);

void sendMultiData(uint8_t *data, int n);

void sendPixel(uint8_t r, uint8_t g, uint8_t b, uint8_t colorDepth);

void sendMultiPixel(uint8_t *r, uint8_t *g, uint8_t *b, uint8_t colorDepth, int n);

void setColorDepth(screenInstance *screen, uint8_t colorDepth);

void setupScrolling(uint8_t horizontalScrollOffset, uint8_t rowStart, uint8_t rowsNumber, uint8_t verticalScrollOffset, uint8_t timeInterval);

void enableScrolling(bool value);

void clearScreen(uint8_t r1, uint8_t c1, uint8_t r2, uint8_t c2);

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
