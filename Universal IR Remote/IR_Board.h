/***************************
 * IR_Board.h
 * Use this header file to attach functions and define constants for IR_Board.c
****************************/

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
#define TOTAL_KEYS 16

extern void Init_KeypadIO(void);
extern enum KEYPAD scan_key(void);
unsigned char index_to_keypad_num(unsigned char);
extern void Buttons_startWDT(void);
