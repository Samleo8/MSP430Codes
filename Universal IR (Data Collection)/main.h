/***************************
 * MAIN.H
 * Use this header file to attach functions and define constants
****************************/

#include "msp430fr4133.h"

//FRAM READING AND WRITING
#define MAX_ADC_CHANNEL 16

//Button Debounce
#define BUTTON_READY (0)
#define BUTTON_PRESSED (1)

//Number parsing
typedef unsigned char boolean;
#define TRUE (0xFF)
#define FALSE (0x00)

#define ABS(n) (n>0)?n:-n
#define MAX(m,n) (m>n)?m:n
#define MIN(m,n) (m<n)?m:n

//For button debounce in WDT and P1/2 interrupts
extern unsigned char buttonDebounce;

//Functions
void Init_GPIO(void);
void Init_Clock(void);
void Init_ADC(void);

void IR_Mode_Setting(void);
