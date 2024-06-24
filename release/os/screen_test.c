#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>

int main() {

	int fd;
	uint32_t *screen = NULL;

	// Open file descriptor
	fd = open("/dev/uio0",O_RDWR);
	
	// Map physical memory into virtual memory (UIO -> offset = 0)
	screen = mmap(NULL, 0x10000, PROT_READ|PROT_WRITE, MAP_SHARED, fd,0);

	printf("Pointer Physical Address: %x \n", screen);

	sleep(1);
	screen[0]= 0b01; // Turn ON the screen
	sleep(3);
	screen[0] = 0b00; // Turn OFF the screen 
	sleep(2);

	// Unmap memory
	munmap(screen, 0x10000);
	// Close file descriptor
	close(fd);
	printf("Exit\n");

	return 0;
}