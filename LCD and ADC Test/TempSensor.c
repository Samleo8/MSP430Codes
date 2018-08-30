/***************************
 * TempSensor.C
 * Contains code for Temperature sensor
 *
 * Functions:
 *      Init_Temp: Initializes ADC and timers for the temperature sensor
 *
 * Header Files:
 *      MSP430FR4133.h
 *      main.h (for the constants)
 *      LCD.h
****************************/

#include "msp430fr4133.h"
#include "main.h"
#include "TempSensor.h"

volatile unsigned char tempSensorRunning = FALSE;

//Initialize Temperature Sensor
void Init_Temp(){
    //*-------- FIRST INIT ADC ----------*//

    Init_ADC();

    //Interface
    P1OUT |= BIT0;
    tempSensorRunning = TRUE;
}
