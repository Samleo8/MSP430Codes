/***************************
 * MAIN.C
 * Contains main code
 * 
 * Functions:
 *  	Init_GPIO: Initializes ports
 *	Init_Clock: Initializes XT1 OSC and DCO
 * 
 * Connects to: 
 * 		LCD.c/h
 * 		IR_Board.c/h
 *
 *
 * 	IMPORTANT NOTE: Disconnect the UART RX Jumper on the Launchpad for the "3,6,9,Cool" column to work.
 * 	                BUT only do so after debugging, or CCS will say "No USB FET found".
****************************/

#include "main.h"
#include "LCD.h"
#include "IR_Board.h"

extern const unsigned char POS[7];

unsigned char buttonDebounce = 1;

unsigned char button_num = TOTAL_KEYS+1;     //button number
unsigned int keypad_digit = 0;

#pragma PERSISTENT(ans); //store ans in FRAM
signed long ans = 0;
signed long ans_mem = 0;

unsigned char mode = 0;
unsigned char prevInput = 0;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	/*--GENERAL INITIALIZATIONS--*/
	Init_GPIO();
	Init_Clock();

    LCD_Init();

    Init_KeypadIO();    //Initialize Board

    // Configure IR input pin
    P1DIR &= ~BIT6;                             //Set P1.6 as input
    P1SEL0|=  BIT6;                             //Set P1.6 as TA0.2 input

	__enable_interrupt();

	while(1){
	    LCD_Text("Cool to delete  Power to clear  OK for ans  Click any button to continue");

	    if(buttonDebounce==2 || button_num!=TOTAL_KEYS+1) break;
	}

    LCD_Number(ans);

    while(1){
        __bis_SR_register(LPM3_bits | GIE);     //enter low power mode, wait for keypad/button interrupts

        P4OUT &= ~BIT0;
        P1OUT &= ~BIT0;

        LCD_Clear();

        switch(button_num){
            case 1: //ANS
                compute_ans();
                prevInput = MODE_TYPE;
                mode = MODE_TYPE;
                break;
            //case 2: //COPY
            //    break;
            case 3: //-
                if(mode != MODE_TYPE){
                    if(prevInput == MODE_TYPE){ //continue calculating string
                        compute_ans();
                        ans_mem = ans;
                    }
                    else if(prevInput == MODE_MINUS){ //double click, ignore
                        break;
                    }
                    else if(prevInput == MODE_ADD){ //change from subtract to add
                        //just change the mode
                    }
                }
                else{
                   ans_mem = ans;
                   write_ans(0);
                }

                mode = MODE_MINUS;
                prevInput = MODE_MINUS;
                break;
            case 4: //+
                if(mode != MODE_TYPE){
                    if(prevInput == MODE_TYPE){ //continue calculating string
                        compute_ans();
                        ans_mem = ans;
                    }
                    else if(prevInput == MODE_ADD){ //double click, ignore
                        break;
                    }
                    else if(prevInput == MODE_MINUS){ //change from subtract to add
                        //just change the mode
                    }
                }
                else{
                   ans_mem = ans;
                   write_ans(0);
                }

                mode = MODE_ADD;
                prevInput = MODE_ADD;
                break;
            case 2:
            case 5: //Delete
                if(prevInput == MODE_TYPE){
                    if(ans < 10 && ans >-10){
                        write_ans(0);
                    }
                    else{
                        write_ans((signed long)(ans/10));
                    }
                    //write_ans((signed long)(ans/10));
                }
                else{
                    clear_ans();
                }
                break;
            case 13: //Clear All
                clear_ans();
                break;
            default: //Numbers
                keypad_digit = index_to_keypad_num(button_num);

                if(prevInput != MODE_TYPE) write_ans(0);

                if(keypad_digit < 10){
                    if(ans>-100000 && ans<100000){
                        write_ans(ans*10+index_to_keypad_num(button_num));
                    }
                }

                prevInput = MODE_TYPE;

                break;
        }

        LCD_Number(ans);
    }
}

void clear_ans(){
    write_ans(0);
    ans_mem = 0;

    prevInput = MODE_TYPE;
    mode = MODE_TYPE;
}

void write_ans(signed long value){
    SYSCFG0 &= ~(DFWP | PFWP);
    ans = value;
    SYSCFG0 |= (DFWP | PFWP);
}

void compute_ans(){
    if(prevInput == MODE_TYPE){ //give the ans
        switch(mode){
            case MODE_ADD:
                write_ans(ans_mem+ans);
                P4OUT |= BIT0;
                break;
            case MODE_MINUS:
                write_ans(ans_mem-ans);
                P4OUT |= BIT0;
                break;
            default:
                write_ans(0);
                P1OUT |= BIT0;
                break;
        }
    }
}

//Initialize GPIO
void Init_GPIO(){
	// Configure all GPIO to Output, Low
	// Make sure there is no pin conflict
	P1OUT = 0x00; P2OUT = 0x00; P3OUT = 0x00; P4OUT = 0x00;
	P5OUT = 0x00; P6OUT = 0x00; P7OUT = 0x00; P8OUT = 0x00;

	P1DIR = 0xFF; P2DIR = 0xFF; P3DIR = 0xFF; P4DIR = 0xFF;
	P5DIR = 0xFF; P6DIR = 0xFF; P7DIR = 0xFF; P8DIR = 0xFF; 
	
	//Configure button S1 (P1.2) interrupt
	P1DIR &= ~(BIT2); //Config as INPUT
	P1REN |= BIT2; //Set pull-up resistor
	P1OUT |= BIT2; // Resistor pulls up
	P1IE |= BIT2; //Allow interrupt
	P1IES |= BIT2; //High-to-low transition
	P1IFG &= ~(BIT2); //Clear interrupt flag
	
    //Configure button S2 (P2.6) interrupt
    P2DIR &= ~(BIT6); //Config as INPUT
    P2REN |= BIT6; //Pull-up resistor
    P2OUT |= BIT6; // Resistor pulls up
	P2IE |= BIT6; //Allow interrupt
	P2IES |= BIT6; //High-to-low transition
	P2IFG &= ~(BIT6); //Clear interrupt flag
	
	//Set P4.1 and P4.2 as Primary Module Function Input, LFXT.
	P4DIR &= ~(BIT1 | BIT2);
	P4SEL0 |= BIT1 | BIT2;
	
	PM5CTL0 &= ~LOCKLPM5;           			// Disable the GPIO power-on default high-impedance mode
												// to activate previously configured port setting
}

//Initialize Clock
void Init_Clock()
{
	P4SEL0 |= BIT1 | BIT2;                  // set XT1 pin as second function

	do
	{
		CSCTL7 &= ~(XT1OFFG | DCOFFG);      // Clear XT1 and DCO fault flag
		SFRIFG1 &= ~OFIFG;
	} while (SFRIFG1 & OFIFG);              // Test oscillator fault flag

	CSCTL3 |= SELREF__XT1CLK;               // Set XT1CLK as FLL reference source
	CSCTL1 &= ~(DCORSEL_7);                 // Clear DCO frequency select bits first
	CSCTL1 |= DCORSEL_3;                    // Set DCO = 8MHz
	CSCTL2 = FLLD_0 + 243;                  // DCODIV = 8MHz

	do
	{
		__delay_cycles(7 * 31 * 8);         // Requires 7 reference clock delay before
											// polling FLLUNLOCK bits
											// @8 MHz, ~1736 cycles
	} while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));// Poll until FLL is locked

	CSCTL4 = SELMS__DCOCLKDIV | SELA__XT1CLK;  // Set ACLK = XT1CLK = 32768Hz
												// DCOCLK = MCLK and SMCLK source
	CSCTL5 |= DIVM_0 | DIVS_1;              // MCLK = DCOCLK = 8MHZ,
											// SMCLK = MCLK/2 = 4MHz
}

/* PORT1 Interrupt Service Routine
 * Handles PORT1 interrupts:
 *      1. Push button interrupt (1.2)
 *      2. Keypad board interrupts (1.3, 1.4, 1.5)
 */

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
    switch(__even_in_range(P1IV, P1IV_P1IFG7))
    {
        case P1IV_NONE : break;
        case P1IV_P1IFG0 : break;
        case P1IV_P1IFG1 : break;
        case P1IV_P1IFG2 :
            break;
        case P1IV_P1IFG3:
        case P1IV_P1IFG4:
        case P1IV_P1IFG5:
            P1IFG &= ~(BIT3 | BIT4 | BIT5);
            P1OUT |= BIT0;

            if (buttonDebounce == 1)
            {
                buttonDebounce = 2;

                Buttons_startWDT();
                button_num = scan_key(); // scan the keypad

                __bic_SR_register_on_exit(LPM3_bits); //exit LPM3
            }
             break;
        case P1IV_P1IFG6 : break;
        case P1IV_P1IFG7 : break;
    }
}

/* PORT2 Interrupt Service Routine
 * Handles PORT2 interrupts:
 *      1. Push button interrupt (2.6)
 *      2. Keypad board interrupts (2.7)
 */
#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
    switch(__even_in_range(P2IV, P2IV_P2IFG7))
    {
        case P2IV_NONE : break;
        case P2IV_P2IFG0 : break;
        case P2IV_P2IFG1 : break;
        case P2IV_P2IFG2 : break;
        case P2IV_P2IFG3 : break;
        case P2IV_P2IFG4 : break;
        case P2IV_P2IFG5 : break;
        case P2IV_P2IFG6 :
            break;
        case P2IV_P2IFG7:
            P2IFG &= ~BIT7;                                  // clear IFG
            P1OUT |= BIT0;

            if (buttonDebounce == 1)
            {
                buttonDebounce = 2;

                Buttons_startWDT();
                button_num = scan_key(); // scan the keypad
                //IR_Mode_Setting();
                __bic_SR_register_on_exit(LPM3_bits); //exit LPM3
            }
            break;
    }
}

