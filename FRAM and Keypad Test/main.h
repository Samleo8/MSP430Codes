/***************************
 * MAIN.H
 * Use this header file to attach functions and define constants
****************************/

#include "msp430fr4133.h"

//FRAM READING AND WRITING
#define FRAM_MEM_PWD 0x24

#define FRAM_PWD_ADDRESS 0xE000
#define FRAM_START_ADDRESS 0xE0E0

#define MAX_ADC_CHANNEL 16

//Number parsing
#define TRUE 0xFF
#define FALSE 0x00

#define ABS(n) (n>0)?n:-n
#define MAX(m,n) (m>n)?m:n
#define MIN(m,n) (m<n)?m:n


void Init_GPIO(void);
void Init_Clock(void);
void Init_ADC(void);
