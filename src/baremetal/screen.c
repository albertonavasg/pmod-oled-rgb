#include "screen.h"

void write_on_off(bool value){

    Xil_Out8(XPAR_SCREEN_0_S00_AXI_BASEADDR, value);
}

uint8_t read_on_off_status() {

	return ( Xil_In8(XPAR_SCREEN_0_S00_AXI_BASEADDR + 4) >> 1);
}

bool read_ready(){

	return ( Xil_In8(XPAR_SCREEN_0_S00_AXI_BASEADDR + 4) && 0b00000001);
}

void send_command(uint8_t comm){

	// Data
	Xil_Out8(XPAR_SCREEN_0_S00_AXI_BASEADDR + 8, comm);
	// DataCommand
	Xil_Out8(XPAR_SCREEN_0_S00_AXI_BASEADDR + 12, COMMAND);
	// Start
	Xil_Out8(XPAR_SCREEN_0_S00_AXI_BASEADDR, 0b11);
}

void send_multi_command(uint8_t *comm, int n){

	for(int i = 0; i < n; i++){
		send_command(comm[i]);
	}
}
