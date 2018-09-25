/***************************
 * IR_Board.C
 * Contains functions for handling returning of constants for IR_Codes
****************************/

#include "main.h"
#include "IR_Codes.h"

//TODO: Use pragmas? Or use #define to inline the function
/* Function: CODE_GET_COUNT_OR_TIME
 * Arguments:
 *      curr_mode [enum MODES]: What is the current mode?
 *      curr_num  [enum KEYPAD]: What keypad button is pressed?
 *      code [0 to MAX_OF_127]:
 *          0: Implies we actually want the COUNT not the time
 *          1-127: We want the TIME not the count; `time` then represents the actual time converted from the code.
 */
unsigned int CODE_GET_COUNT_OR_TIME(enum MODES curr_mode, enum KEYPAD curr_num, unsigned char time){
    switch(curr_mode){
        case AIRCON:
            break;
        case TV1:
            switch(curr_num){
                case KEY_0: case KEY_1: case KEY_2: case KEY_3: case KEY_4: case KEY_5: case KEY_6: case KEY_7: case KEY_8: case KEY_9:
                    switch(time){
                        case 0: return 35; //count
                        case 1: return 740; //actual time from coded 1...
                        case 2: return 855;
                        case 3: return 1030;
                        case 4: return 1585;
                        case 5: return 1855;
                        case 6: return 2260;
                        case 7: return 2940;
                        default: return 0;
                    }
                case POWER:
                    switch(time){
                        case 0: return 26; //count
                        case 1: return 2212; //actual time from coded 1...
                        case 2: return 2568;
                        case 3: return 4967;
                        case 4: return 9748;
                        case 5: return 36712;
                        default: return 0;
                    }
                default: break; //will return 0 anyway
            }
            break;
        case TV2:
            break;
    }

    return 0;
}
