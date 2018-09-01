/***************************
 * system_pre_init
 *
 * This is a pre init file meant for manually partitioning the MPU memory.
 * Currently excluded from build
****************************/

#include "msp430fr4133.h"

#include <msp430.h>
int _system_pre_init(void)
{
    /* Insert your low-level initializations here */
    /* Disable Watchdog timer to prevent reset during */
    /* long variable initialization sequences. */
    WDTCTL = WDTPW | WDTHOLD;

    // Configure MPU
    MPUCTL0 = MPUPW; // Write PWD to access MPU registers
    MPUSEGB1 = 0x0480; // B1 = 0x4800; B2 = 0x4C00
    MPUSEGB2 = 0x04c0; // Borders are assigned to segments

    // Segment 1 – Allows read and write only (0 to B1)
    // Segment 2 – Allows read only (B1 to B2)
    // Segment 3 – Allows read and execute only (B2 to END)
    MPUSAM = (MPUSEG1WE | MPUSEG1RE | MPUSEG2RE | MPUSEG3RE | MPUSEG3XE);
    MPUCTL0 = MPUPW | MPUENA | MPUSEGIE; // Enable MPU protection

    // MPU registers locked until BOR

    /*==================================*/
    /* Choose if segment initialization */
    /* should be done or not. */
    /* Return: 0 to omit initialization */
    /* 1 to run initialization */
    /*==================================*/
    return 1;
}
