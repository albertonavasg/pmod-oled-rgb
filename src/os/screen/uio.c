#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

/*
This progam is used to test the basic functionality of the uio.h library:
writing in the memory shared registers.
Write a 1 in the ON_OFF register to activate the screen, send two basic commands
to see if it works properly and writing a 0 in the ON_OFF register to deactivate the screen
*/

int main() {

	int fd;
	uint32_t *screen = NULL;

	// Open file descriptor
	fd = open("/dev/uio0",O_RDWR);
	
	// Map physical memory into virtual memory (UIO -> offset = 0)
	screen = mmap(NULL, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	printf("Pointer Physical Address: %x \n", screen);

	sleep(1);
	printf("Turn ON the screen\n");
	screen[0]= 0b01; // Turn ON the screen
	sleep(1);
	printf("Entire display ON\n");
	screen[2] = 0xA5; // Entire display ON
	screen[3] = 0;    // Command
	screen[0] = 0b11; // Send START
	sleep(1);
	printf("Normal display\n");
	screen[2] = 0xA4; // Normal display
	screen[3] = 0;    // Command
	screen[0] = 0b11; // Send START
	sleep(1);
	printf("Turn OFF the screen\n");
	screen[0] = 0b00; // Turn OFF the screen 
	sleep(1);

	// Unmap memory
	munmap(screen, 0x10000);
	// Close file descriptor
	close(fd);
	printf("Exit\n");

	return 0;
}

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
