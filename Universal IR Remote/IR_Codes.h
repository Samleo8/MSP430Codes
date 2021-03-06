/***************************
 * IR_CODES.H
 * This file contains all the (const) data for the transmission of IR remote codes
****************************/

//Appliance modes
static const char* MODE_NAMES[] = { "AIRCON", "TV 1", "TV 2" };
enum MODES {
    AIRCON,
    TV1,
    TV2
};
#define TOTAL_MODES 3

//IR Codes
static const unsigned char CODES_TV1_POWER[26] = { 4, 1, 3, 1, 2, 1, 3, 1, 2, 1, 3, 1, 2, 1, 2, 1, 3, 1, 2, 1, 2, 1, 2, 1, 2, 5 };
static const unsigned char CODES_TV1_NUMBERS[10][35] = {
    { 5, 3, 1, 3, 1, 6, 2, 3, 1, 4, 2, 6, 2, 4, 2, 3, 1, 3, 1, 6, 2, 6, 2, 4, 2, 6, 2, 3, 1, 3, 1, 3, 1, 3, 1 },
    { 5, 3, 1, 3, 1, 6, 2, 3, 1, 5, 1, 6, 2, 4, 2, 3, 1, 3, 1, 3, 1, 6, 2, 4, 2, 6, 2, 3, 1, 3, 1, 3, 1, 4, 2 },
    { 5, 3, 1, 3, 1, 6, 2, 3, 1, 4, 2, 6, 2, 4, 2, 3, 1, 3, 1, 6, 2, 6, 2, 4, 2, 6, 2, 3, 1, 3, 1, 3, 1, 6, 2 },
    { 5, 3, 1, 3, 1, 6, 2, 3, 1, 4, 2, 6, 2, 4, 2, 3, 1, 3, 1, 3, 1, 6, 2, 4, 2, 6, 2, 3, 1, 3, 1, 3, 1, 7, 2 },
    { 5, 3, 1, 3, 1, 6, 2, 3, 1, 4, 2, 6, 2, 4, 2, 3, 1, 3, 1, 6, 2, 6, 2, 4, 2, 6, 2, 3, 1, 3, 1, 4, 2, 3, 1 },
    { 5, 3, 1, 3, 1, 6, 2, 3, 1, 4, 2, 6, 2, 4, 1, 3, 1, 3, 1, 6, 2, 6, 2, 5, 2, 6, 2, 3, 1, 3, 1, 4, 1, 6, 2 },
    { 5, 3, 1, 3, 1, 6, 2, 3, 1, 4, 2, 6, 2, 4, 1, 3, 1, 3, 1, 6, 2, 6, 2, 5, 2, 6, 2, 3, 1, 3, 1, 4, 1, 6, 2 },
    { 5, 3, 1, 3, 1, 6, 2, 3, 1, 4, 2, 6, 2, 4, 2, 3, 1, 3, 1, 3, 1, 6, 2, 4, 2, 6, 2, 3, 1, 3, 1, 4, 2, 7, 2 },
    { 5, 3, 1, 3, 1, 6, 1, 3, 1, 4, 1, 6, 1, 4, 1, 3, 1, 3, 1, 6, 1, 6, 1, 4, 2, 6, 1, 3, 1, 3, 1, 6, 1, 3, 1 },
    { 5, 3, 1, 3, 1, 6, 2, 3, 1, 4, 1, 6, 2, 4, 2, 3, 1, 3, 1, 3, 1, 6, 1, 4, 2, 6, 2, 3, 1, 3, 1, 6, 2, 4, 2 }
};

//Functions
unsigned int CODE_GET_COUNT_OR_TIME(enum MODES, enum KEYPAD, unsigned char);
