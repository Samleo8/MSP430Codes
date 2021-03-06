/***************************
 * IR_Board.C
 * Contains handlers for keypad GPIO and scanning
 *
 * Functions:
 *      Init_KeypadIO: Initializes keypad i/o
 *      Scan_Key: Scans to see which key on the matrix is pressed
 *
 * Connects to:
 *      IR_Board.c/h
 *      LCD.c/h
****************************/

#include "main.h"
#include "LCD.h"
#include "IR_Board.h"

/* KEYPAD SCAN METHOD
 * To scan which button is being pressed, a 4x4 matrix is used.
 * In pins (rows). Without any buttons pressed, the columns and rows are not connected to each other.
 * When a button is pressed, it connects its corresponding column pin with its corresponding row pin.
 * Configure the Keypad Out (column) pins as outputs, and Keypad In (row) pins as inputs.
 * The Keypad Out pins are toggled high one at a time, while the Keypad In pins are read for any changes.
 *  KEY-IN  = { 1.3 , 1.4 , 1.5 , 2.7 };
 *  KEY-OUT = { 8.1 , 1.1 , 8.0 , 2.5 };
 */

unsigned char key_num = 0;

void Init_KeypadIO() {
    // Configure Keypad GPIO
    //  KEYPAD IN  = { 1.3 , 1.4 , 1.5 , 2.7 };
    //  KEYPAD OUT = { 8.1 , 1.1 , 8.0 , 2.5 };

    //KEYPAD INPUT (P1.3/1.4/1.5/2.7)
    P1DIR &= ~(BIT3 | BIT4 | BIT5);             //Set as keypad input
    P2DIR &= ~BIT7;
    P1REN |= (BIT3 | BIT4 | BIT5);              //Set pull up resistor
    P2REN |= BIT7;
    P1OUT |= (BIT3 | BIT4 | BIT5);              //Pull-up to Vcc
    P2OUT |= BIT7;
    P1IES |= (BIT3 | BIT4 | BIT5);              //Enable high to low interrupt
    P2IES |= BIT7;
    P1IE  |= (BIT3 | BIT4 | BIT5);              //Enable P1.3/1.4/1.5/2.7 interrupt
    P2IE  |= BIT7;


    P8DIR |= (BIT0 | BIT1);                     //Set P8.0/8.1/1.1/2.5 as keypad output
    P1DIR |= BIT1;
    P2DIR |= BIT5;
    P8OUT &= ~(BIT0 | BIT1);                    //P8.0/8.1/1.1/2.5 output low
    P1OUT &= ~BIT1;
    P2OUT &= ~BIT5;

    //Clear Port 1 IFG
    P1IFG = 0;
    P2IFG = 0;
}

/* KEYPAD SCAN FUNCTION
 * To scan which button is being pressed, a 4x4 matrix is used.
 * In pins (rows). Without any buttons pressed, the columns and rows are not connected to each other.
 * When a button is pressed, it connects its corresponding column pin with its corresponding row pin.
 * Configure the Keypad Out (column) pins as outputs, and Keypad In (row) pins as inputs.
 * The Keypad Out pins are toggled high one at a time, while the Keypad In pins are read for any changes.
 *  KEY-IN  = { 1.3 , 1.4 , 1.5, 2.7 };
 *  KEY-OUT = { 8.1 , 1.1 , 8.0, 2.5 };
 *
 *  RETURNS:
 *      Key number from 1 to 16
 *      16 to 13: First col (16 is the first button on the top-left)
 *      12 to 9: Second col
 *      8 to 5: Third col
 *      4 to 1: Fourth col
 */

//*
enum KEYPAD scan_key(void){
    unsigned char row_sel=0;
    unsigned char key_in=0;
    unsigned char keycol=0;
    unsigned char keyrow=0;
    unsigned char key_array[TOTAL_KEYS+1];
    unsigned char i=0;
    unsigned char j=0;

    // clear the array before scan
    i = TOTAL_KEYS+1;
    while(i--) key_array[i] = 0;

    keycol = BIT0;

    for (i=0;i<KEYPAD_COLS;i++) {
        P8DIR &= ~(BIT0 + BIT1);
        P1DIR &= ~BIT1;
        P2DIR &= ~BIT5;
        switch(i) {
            case 0:
                P8OUT &= ~BIT1;
                P8DIR |= BIT1;                      // select only one column
            case 1:
                P1OUT &= ~BIT1;
                P1DIR |= BIT1;                      // select only one column
            case 2:
                P8OUT &= ~BIT0;
                P8DIR |= BIT0;                      // select only one column
            case 3:
                P2OUT &= ~BIT5;
                P2DIR |= BIT5;                      // select only one column
        }
        __delay_cycles(100);

        key_in = (P1IN & 0x38) | (P2IN & 0x80); // get key input value

        if((key_in&0x08)!=0)                    // find the pressed button row
            row_sel|=0x08;
        if((key_in&0x10)!=0)
            row_sel|=0x04;
        if((key_in&0x20)!=0)
            row_sel|=0x02;
        if((key_in&0x80)!=0)
            row_sel|=0x01;
        keyrow = BIT3;

        for (j=0;j<KEYPAD_ROWS;j++){
            if ((row_sel & keyrow) == 0)
                key_array[(j + i * KEYPAD_ROWS  + 1)] = 1;
            keyrow = keyrow >> 1;
        }
        keycol = keycol << 1 ;
        row_sel=0;
    }

    i = TOTAL_KEYS+1;
    while(i--) { // get pressed button number
        if (key_array[i]) {
            key_num = i;
            break;
        }
    }

    P8DIR |= BIT0 + BIT1; //Set P8.0/8.1/1.1/2.5 as keypad output
    P1DIR |= BIT1;
    P2DIR |= BIT5;
    P8OUT &= ~(BIT0 + BIT1); //P8.0/8.1/1.1/2.5 output low
    P1OUT &= ~BIT1;
    P2OUT &= ~BIT5;

    return key_num;
}
//*/

//NOTE: Putting this as an inline `#define` function actually uses MORE memory!!!
unsigned char index_to_keypad_num(unsigned char ind) {
    switch(ind){
        case 6: return 9;
        case 7: return 6;
        case 8: return 3;
        case 9: return 0;
        case 10: return 8;
        case 11: return 5;
        case 12: return 2;
        case 14: return 7;
        case 15: return 4;
        case 16: return 1;
        default: return 10;
    }
}

/* NOTE: INTERESTING DEBOUNCE TIMER METHOD
 * Uses Watchdog timer as the button debouncer
 * When one of the keypad buttons are pressed (P1 and P2 interrupt, in main.c), the watchdog timer is started
 * Then 250ms later, interrupt occurs, and watchdog timer is reset.
 */

// Sets up the WDT as a button debouncer, only activated once a
// button interrupt has occurred.
void Buttons_startWDT() {
    // WDT as 250ms interval counter
    SFRIFG1 &= ~WDTIFG;
    WDTCTL = WDTPW + WDTSSEL_1 + WDTTMSEL + WDTCNTCL + WDTIS_5;
    SFRIE1 |= WDTIE;
}

// Handles Watchdog Timer interrupts.
#pragma vector=WDT_VECTOR
__interrupt void WDT_ISR(void) {
    if(buttonDebounce == BUTTON_PRESSED) {
        buttonDebounce = BUTTON_READY; //button has cooled down

        //Reset watchdog timer
        SFRIFG1 &= ~WDTIFG;
        SFRIE1 &= ~WDTIE;
        WDTCTL = WDTPW + WDTHOLD;

        P1OUT &= ~BIT0;
    }
}

