/***************************
 * MAIN.H
 * Use this header file to attach functions and define constants
****************************/

#include "msp430fr4133.h"


#define MAX_ADC_CHANNEL 15
                                                        // See device datasheet for TLV table memory mapping
#define CALADC_15V_30C  *((unsigned int *)0x1A1A)       // Temperature Sensor Calibration-30 C
#define CALADC_15V_85C  *((unsigned int *)0x1A1C)       // Temperature Sensor Calibration-85 C

//Since the CALADC values refer to the address/registers,
//we use the debugger to find that the difference (CALADC_15V_85C-CALADC_15V_30C) = 121

#define ADC_TO_TEMP 4.54 // = 10 * (85-30)/(CALADC_15V_85C-CALADC_15V_30C) ~ 550/121;
                         // (reason for the times 10 is so that we can get the temperature to the last decimal point)
#define ADC_TO_V_REF_3V3 0.322580645 // 100*3.3/1023
#define ADC_TO_V_REF_1V5 0.146627566 // 100*1.5/1023

//Number parsing
#define TRUE 0xFF
#define FALSE 0x00

#define ABS(n) (n>0)?n:-n
#define MAX(m,n) (m>n)?m:n
#define MIN(m,n) (m<n)?m:n


void Init_GPIO(void);
void Init_Clock(void);
void Init_ADC(void);
