#include "xparameters.h" // For Screen peripherals memory addresses 
#include "xil_io.h"      // For IO (Xil_In, Xil_Out)
#include "sleep.h"       // For sleep()
int main()
{
    // Write a '1' in ON_OFF of Screen A and Screen_B
    Xil_Out32(XPAR_SCREEN_A_BASEADDR, 0b01);
    Xil_Out32(XPAR_SCREEN_B_BASEADDR, 0b01);

    sleep(1);

    // Entire Display ON Command
    Xil_Out32(XPAR_SCREEN_A_BASEADDR + 8, 0xA5 | 0 << 8  | 1 << 9);
    Xil_Out32(XPAR_SCREEN_B_BASEADDR + 8, 0xA5 | 0 << 8  | 1 << 9);

    sleep(1);

    // Entire Display OFF Command
    Xil_Out32(XPAR_SCREEN_A_BASEADDR + 8, 0xA6 | 0 << 8  | 1 << 9);
    Xil_Out32(XPAR_SCREEN_B_BASEADDR + 8, 0xA6 | 0 << 8  | 1 << 9);

    sleep(1);

    // Write a '0' in ON_OFF of Screen A and Screen_B
    Xil_Out32(XPAR_SCREEN_A_BASEADDR, 0b00);
    Xil_Out32(XPAR_SCREEN_B_BASEADDR, 0b00);

    sleep(1);

    return 0;
}