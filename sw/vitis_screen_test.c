#include "xparameters.h" // For Screen peripherals memory addresses
//#include "xil_io.h"      // For IO (Xil_In, Xil_Out)
#include "sleep.h"       // For sleep()

int main()
{   
    // Store Screen A and B memory positions
    volatile uint32_t *screenA = (volatile uint32_t *) XPAR_SCREEN_A_BASEADDR;
    volatile uint32_t *screenB = (volatile uint32_t *) XPAR_SCREEN_B_BASEADDR;

    // Write a '1' in ON_OFF of Screen A and Screen_B
    *screenA = 0x01;
    *screenB = 0x01;
    //Xil_Out32(XPAR_SCREEN_A_BASEADDR, 0x01);
    //Xil_Out32(XPAR_SCREEN_B_BASEADDR, 0x01);
    
    sleep(1);

    // Entire Display ON Command
    *(screenA + 2) = 0xA5 | 0 << 8  | 1 << 9;
    *(screenB + 2) = 0xA5 | 0 << 8  | 1 << 9;
    //Xil_Out32(XPAR_SCREEN_A_BASEADDR + 8, 0xA5 | 0 << 8  | 1 << 9);
    //Xil_Out32(XPAR_SCREEN_B_BASEADDR + 8, 0xA5 | 0 << 8  | 1 << 9);

    sleep(1);

    // Entire Display OFF Command
    *(screenA + 2) = 0xA6 | 0 << 8  | 1 << 9;
    *(screenB + 2) = 0xA6 | 0 << 8  | 1 << 9;
    //Xil_Out32(XPAR_SCREEN_A_BASEADDR + 8, 0xA6 | 0 << 8  | 1 << 9);
    //Xil_Out32(XPAR_SCREEN_B_BASEADDR + 8, 0xA6 | 0 << 8  | 1 << 9);

    sleep(1);

    // Write a '0' in ON_OFF of Screen A and Screen_B
    *screenA = 0x00;
    *screenB = 0x00;
    //Xil_Out32(XPAR_SCREEN_A_BASEADDR, 0x00);
    //Xil_Out32(XPAR_SCREEN_B_BASEADDR, 0x00);

    sleep(1);

    return 0;
}