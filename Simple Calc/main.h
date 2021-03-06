/***************************
 * MAIN.H
 * Use this header file to attach functions and define constants
****************************/

#include "msp430fr4133.h"

//Modes
#define MODE_TYPING 0
#define MODE_TYPE MODE_TYPING
#define MODE_NUMBER MODE_TYPING

#define MODE_ADD 1
#define MODE_PLUS MODE_ADD

#define MODE_SUBTRACT 2
#define MODE_MINUS MODE_SUBTRACT

//Number parsing
#define TRUE 0xFF
#define FALSE 0x00

#define ABS(n) ((n>0)?n:-n)
#define MAX(m,n) ((m>n)?m:n)
#define MIN(m,n) ((m<n)?m:n)

//For button debounce in WDT and P1/2 interrupts
extern unsigned char buttonDebounce;

void Init_GPIO(void);
void Init_Clock(void);
void Init_ADC(void);
void clear_ans(void);
void write_ans(signed long);
void compute_ans(void);
