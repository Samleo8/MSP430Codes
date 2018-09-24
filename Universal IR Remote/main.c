/***************************
 * MAIN.C
 * Contains main code
 * 
 * Functions:
 *  	Init_GPIO: Initializes ports
 *	Init_Clock: Initializes XT1 OSC and DCO
 *
 *	IR_Mode_Setting: Sets mode for IR mode accordingly
 * 
 * Connects to: 
 * 		LCD.c/h
 * 		IR_Board.c/h
 * 		IR_Codes.h
 *
 *
 * 	NOTE: Disconnect the UART RX Jumper on the Launchpad for the "3,6,9,Cool" column to work.
 * 	                BUT only do so after debugging, or CCS will say "No USB FET found".
 *
 * 	                When not debugging and the jumper still in, however, the 3rd column will still work.
 *
 * 	NOTE: To support one byte enums, change Properties > Advanced > Runtime options > enum to "packed"
 * 	      Change Properties -> Debug -> MSP430 Properties -> Download Options -> Erase Options to "Erase and download necessary segements only (Differential Download)"
****************************/
#include "main.h"
#include "LCD.h"
#include "IR_Board.h"
#include "IR_Codes.h"

//IR Keypad Buttons
enum KEYPAD button_num = NONE;     //button number
unsigned char buttonDebounce = BUTTON_READY;

//NOTE: All IR codes and data is under IR_codes.h
enum MODES mode = AIRCON;

//Remote status
enum REMOTE_STATUSES {
    IDLE,
    TRANSMITTING
} remote_status;

unsigned char tx_cnt = 0;
unsigned char *FRAM_ptr  = (unsigned char *)(&CODES_TV1_POWER[0]);

int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	Init_GPIO();
	Init_Clock();

    LCD_Init();

    Init_KeypadIO();    //Initialize Keypad

	__enable_interrupt();

	// Configure LED
    P4DIR |= BIT0;                              //Set P4.0 to toggle LED
    P4OUT &= ~BIT0;
	
	// Configure IR input pin
    P1DIR &= ~BIT6;                             //Set P1.6 as input
    P1SEL0|=  BIT6;                             //Set P1.6 as TA0.2 input

    remote_status = IDLE;
    mode = TV1;

    LCD_Text( (char *)(MODE_NAMES[mode]) );

    while(1) {
        if(remote_status == TRANSMITTING) {
            /* USER ASKS TO TRANSMIT; BEGIN SENDING THE SIGNAL
             * 1. Configure IR output pins
             * 2. Disable all P1/2 interrupts: Prevents unwanted exit of LPM3 in the middle of signal send
             * 3. Configure IR modulation using ASK protocol
             * 3. Enter LPM3 to pause until the transmit is complete
             * 4. In the TA0.0 interrupt, when transmit is complete, exits LPM3.
             * 5. Stop all timers, renable interrupts and continue code (actually enters LPM3 again).
             */

            // Configure IR output pin
            P1SEL0 |= BIT0;                      // use internal IR modulator

            // Disable Port1 & Port2 interrupt
            P1IE = 0;
            P2IE = 0;

            // Configure IR modulation: ASK
            SYSCFG1 = IRDSSEL + IREN;
            TA0CCTL0 = CCIE;
            TA0CCTL2 = OUTMOD_0;                // output mode: output
            TA1CCTL2 = OUTMOD_7;                // output mode: reset/set

            // 38kHz 1/4 duty-cycle carrier waveform length setting
            TA1CCR0 = 104;
            TA1CCR2 = 25;

            // envelope signal length setting
            TA0CCR0 = 640;                      //the initial time of TA0 should be longer than TA1

            // set timer operation mode
            TA1CTL = TASSEL_2 + MC_1 + TACLR;   //SMCLK, UP mode
            TA0CTL = TASSEL_2 + MC_1 + TACLR;   //SMCLK, UP mode

            // stop until the end of IR code by entering LPM3
            // button interrupts MUST be disabled so it doesn't accidentally cause LPM3 exit
            __bis_SR_register(LPM3_bits | GIE);

            // Transmission complete: disable timer TA0 and TA1
            TA0CCTL0 = 0;
            TA0CCTL2 = 0;
            TA0CTL = 0;
            TA0CCR0 = 0;

            TA1CCTL0 = 0;
            TA1CCTL2 = 0;
            TA1CTL = 0;
            TA1CCR0 = 0;
            TA1CCR2 = 0;

            // Renable push button and keypad interrupt
            P1IE |= (BIT2 | BIT3 | BIT4 | BIT5);
            P2IE |= (BIT6 | BIT7);
        }
        else if(remote_status == IDLE){
            /* IDLE
             * Just wait for user input. Meanwhile, display the current mode (ie AIRCON, TV1 etc.)
             * TODO: Make an IDLE timer; if idle for too long, put it into extreme LPM4 and only wake up on button interrupt.
             */
            __delay_cycles(1600000);
            LCD_Text( (char*)(MODE_NAMES[mode]) );
        }

        __bis_SR_register(LPM3_bits | GIE);     //enter low power mode
    }
}


void IR_Mode_Setting(){
    LCD_Clear();

    if(remote_status != TRANSMITTING){
        tx_cnt = CODE_GET_COUNT_OR_TIME(mode, button_num, 0); //get count

        LCD_IR_Buttons(button_num);

        //Check if there's a valid IR code first...
        if(!tx_cnt) return;

        //If so, then start transmission process...
        remote_status = TRANSMITTING;

        switch(mode){
            case AIRCON:
                //FRAM_ptr = &tx_data_aircon[code_num][0];
                //TODO: Set an alternative for AC power off and on
                break;
            case TV1:
                switch(button_num){
                    case KEY_0: case KEY_1: case KEY_2: case KEY_3: case KEY_4: case KEY_5: case KEY_6: case KEY_7: case KEY_8: case KEY_9:
                        FRAM_ptr = &CODES_TV1_NUMBERS[index_to_keypad_num((unsigned char)(button_num))][0];
                        break;
                    case POWER:
                        FRAM_ptr = &CODES_TV1_POWER[0];
                        break;
                    default: break; //will return 0 anyway
                }
                break;
            case TV2:
                //FRAM_ptr = &tx_data_tv[1][code_num][0];
                break;
            default: return;
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
    /* SETUP IMPORTANT CLOCKS AND OSCILLATORS */
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

	/* SETUP TIMER A1.0 FOR COUNTING INTERVALS AND THROWING INTO LPM4
    TA1CTL |= MC__STOP; TA1CTL &= ~(TACLR); //Stop and reset timer
    TA1CTL &= ~(TAIFG); //Clear interrupt flag
    TA1CTL |= TASSEL__ACLK | ID__8; //ACLK is 32768Hz (low-power).
    TA1CTL |= MC__CONTINUOUS | TAIE; //start timer in continuous mode

    TA0CCR0 = 8192; //4096 = 1s => 8192 = 2s
    */
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
            P1IFG &= ~BIT2;

            if(buttonDebounce == BUTTON_READY) {
                buttonDebounce = BUTTON_PRESSED;
                P1OUT |= BIT0;
                Buttons_startWDT();

                remote_status = IDLE;

                mode++;
                if(mode>=TOTAL_MODES) mode = 0;
            }
            __bic_SR_register_on_exit(LPM3_bits); //exit LPM3
            break;
        case P1IV_P1IFG3:
        case P1IV_P1IFG4:
        case P1IV_P1IFG5:
            P1IFG &= ~(BIT3 | BIT4 | BIT5);

            if (buttonDebounce == BUTTON_READY){
                buttonDebounce = BUTTON_PRESSED;

                Buttons_startWDT();
                button_num = scan_key(); // scan the keypad
                IR_Mode_Setting();

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
            P2IFG &= ~BIT6;

            if(buttonDebounce == BUTTON_READY) {
                buttonDebounce = BUTTON_PRESSED;
                P1OUT |= BIT0;
                Buttons_startWDT();

                remote_status = IDLE;

                mode++;
                if(mode>=TOTAL_MODES) mode = 0;

                //LCD_Text( (char*)(MODE_NAMES[mode]) );
            }
            __bic_SR_register_on_exit(LPM3_bits); //exit LPM3
            break;
        case P2IV_P2IFG7:
            P2IFG &= ~BIT7;                                  // clear IFG
            if (buttonDebounce == BUTTON_READY){
                buttonDebounce = BUTTON_PRESSED;

                Buttons_startWDT();
                button_num = scan_key(); // scan the keypad
                IR_Mode_Setting();

                __bic_SR_register_on_exit(LPM3_bits); //exit LPM3
            }
            break;
    }
}

/* CONTROLLING OF IR TX AND RX
 * -    TA 0.0: TX
 */

//********Timer0.0 interrupt ISR*********//
//For transmission of signals
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
    switch( TA0IV )
    {
        case TA0IV_NONE:
            //TODO: instead of using this method, let tx_cnt = CODE_GET_COUNT_OR_TIME when transmission is first triggered, then count down
            if(tx_cnt>0){ //Transmitting
                P4OUT |= BIT0;

                TA0CCTL2 ^= OUT;

                TA0CCR0 = CODE_GET_COUNT_OR_TIME(mode,button_num,*FRAM_ptr); //Emit appropriate IR code
                *FRAM_ptr++;

                tx_cnt--;
            }
            else{ //Complete
                P4OUT &= ~BIT0;

                TA0CCTL0 &= ~CCIE;      // disable timer_A0 interrupt
                tx_cnt = 0;

                //TODO: Start a timer and use the timer interrupt to exit LPM3 instead

                remote_status = IDLE;   //disable IR
                __bic_SR_register_on_exit(LPM3_bits);                // Exit LPM3
            }
            break;
        default: break;
    }
}

/********Timer 1.0 interrupt ISR*********/
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR (void) {
    switch(__even_in_range(TA1IV,TA1IV_TAIFG)) {
        case TA1IV_NONE:
            P1OUT ^= BIT0;
            break;
        case TA1IV_TAIFG:
            P4OUT ^= BIT0;
            break;
        default: break;
    }
}
//*/

