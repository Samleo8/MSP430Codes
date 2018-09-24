/***************************
 * MAIN.H
 * Use this header file to attach functions and define constants
****************************/

#include "msp430fr4133.h"

//Keypad enum
enum KEYPAD{
    NONE = 0,

    POWER = 13,
    OK = 1,
    COPY = 2,

    TEMP_MINUS = 3,
    TEMP_PLUS = 4,
    COOL = 5,

    KEY_0 = 9,
    KEY_1 = 16,
    KEY_2 = 12,
    KEY_3 = 8,
    KEY_4 = 15,
    KEY_5 = 11,
    KEY_6 = 7,
    KEY_7 = 14,
    KEY_8 = 10,
    KEY_9 = 6
};

//Button Debounce
enum BUTTON_DEBOUNCE_STATUS{
    BUTTON_READY,
    BUTTON_PRESSED
};

//Number parsing and self-defined functions
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

void IR_Mode_Setting(void);
