#ifndef SCREENH
#define SCREENH

#include <stdbool.h>
#include <stdint.h>

#include <xparameters.h>
#include <xil_io.h>
#include "xil_printf.h"

// ON_OFF_STATUS
#define OFF 0
#define TURNING_ON 1
#define TURNING_OFF 2
#define ON 3

// DATA_COMMAND
#define COMMAND 0
#define DATA 1

void write_on_off(bool value);

uint8_t read_on_off_status();

bool read_ready();

void send_command(uint8_t comm);

void send_multi_command(uint8_t *comm, int n);

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
