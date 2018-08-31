/***************************
 * MAIN.C
 * Contains main code
 * 
 * Functions:
 *  	Init_GPIO: Initializes ports
 *	Init_Clock: Initializes XT1 OSC and DCO
 *	Init_ADC: Initializes ADC (NOT USED; TODO: Remove unless using temp function)
 *
 *	IR_Mode_Setting: Sets mode for IR mode accordingly
 * 
 * Connects to: 
 * 		LCD.c/h
 * 		IR_Board.c/h
 *
 *
 * 	IMPORTANT NOTE: Disconnect the UART RX Jumper on the Launchpad for the "3,6,9,Cool" column to work.
 * 	                BUT only do so after debugging, or CCS will say "No USB FET found".
 *
 * 	                When not debugging and the jumper still in, however, the 3rd column will still work.
 *
 * 	NOTE: To support one byte enums, change Properties > Advanced > Runtime options > enum to "packed"
 * 	      Change CCS Project Properties -> Debug -> MSP430 Properties -> Download Options -> Erase Options to "Erase and download necessary segements only (Differential Download)"
****************************/

#include "main.h"
#include "LCD.h"
#include "IR_Board.h"

extern const unsigned char POS[7];

unsigned char channel = MAX_ADC_CHANNEL;

enum boolean intro = FALSE; //NOTE: Make this 'FALSE' if you don't want the intro.

//IR Keypad Buttons
unsigned char button_num = TOTAL_KEYS+1;     //button number
unsigned char buttonDebounce = BUTTON_READY;

//Appliance modes
const char* MODE_NAMES[] = { "AIRCON", "TV 1", "TV 2" };
enum MODES {
    AIRCON,
    TV1,
    TV2
} mode = AIRCON;
#define TOTAL_MODES 3

//IR mode/status
enum IR_STATE {
    TRANSMITTING,
    RECEIVING,
    DISABLED
} IR_status;

unsigned char   code_num = 0;

//Copy mode
enum boolean copy_mode;

//RX,TX and Timer Counters
#pragma PERSISTENT(rx_cnt);     //store rx_cnt in FRAM | TODO: Better partition memory? Seems like the error comes from there
unsigned int    rx_cnt[3][14]={0}; //received bit counter
unsigned int    tx_cnt=1;       //transmitted bit counter

unsigned int    new_cnt=0;      //new timer counter
unsigned int    old_cnt=0;      //old timer counter
unsigned int    time_cnt=0;     //time interval between two edges

//FRAM Writing and Reading
const unsigned int FRAM_START_ADDRESSES[] = { 0xE000, 0xE102 , 0xE204 };
/* NOTE: 0xE000 to 0xE1000 is 256 addresses. (maximum of exactly 256 addresses will be used)
 *       Difference between each start address is 258, just to be safe
 *       This ensures that when writing the IR signals, they don't override that of another mode.
 */

unsigned int *FRAM_write_ptr = (unsigned int *)(0xE000);
unsigned int *FRAM_read_ptr  = (unsigned int *)(0xE000);

int main(void){
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
	
	Init_GPIO();
	Init_Clock();
//    Init_ADC();

    LCD_Init();

    Init_KeypadIO();    //Initialize Board

	__enable_interrupt();

	// Configure LED
    P4DIR |= BIT0;                              //Set P4.0 to toggle LED
    P4OUT &= ~BIT0;
	
	// Configure IR input pin
    P1DIR &= ~BIT6;                             //Set P1.6 as input
    P1SEL0|=  BIT6;                             //Set P1.6 as TA0.2 input

    copy_mode = FALSE;
    IR_status = DISABLED;
    mode = AIRCON;

    while(intro){
        LCD_Text("Press either push button to switch between appliance  To copy the signals from an appliance press  COPY  BUTTON  OK   Hold both push buttons to turn off and on the remote");

        if(buttonDebounce == BUTTON_PRESSED || button_num!=TOTAL_KEYS+1){
            intro = FALSE;
            break;
        }
    }

    LCD_Text( (char *)(MODE_NAMES[mode]) );

    while(1) {
        if(copy_mode == TRUE){
            if(IR_status == RECEIVING) {
                /* USER ASKS TO RECEIVE; BEGIN COPYING OVER THE SIGNAL
                 * 1. Start TA0.2 timer to enable the receive
                 * 2. Write to FRAM in the TA0.2 interrupt
                 * 3. Enter LPM3 to pause until the receive is complete
                 * 4. In the TA0.2 interrupt, when receiving is maxed out, or a button is pressed, it will flag as IR_status = disabled, and exit LPM3
                 * 5. Timer stops and code continues (actually it just enters LPM3 again).
                 */

                TA0CTL = TASSEL_2 + MC_2 + TACLR;   //SMCLK, Continuous mode
                TA0CCTL2=CM_3+SCS+CCIS_0+CAP+CCIE;  //set TA0.2 control register choose CCIxA

                // Pause by entering LPM3 until receiving complete.
                // note that button interrupt cannot cause it to exit out of LPM3 because P1/2 interrupts have been disabled
                //while(IR_status == RECEIVING);
                __bis_SR_register(LPM3_bits | GIE);     //enter LPM3

                TA0CTL = 0;
                TA0CCTL2 = 0;
            }
            else{
                //Waiting for user to press button to copy signal to
            }
        }
        else if(copy_mode == FALSE && IR_status == TRANSMITTING)
        {
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

            // Transmission complete: disable timer 0 and 1
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
        else{
            //Idle
            //LCD_Text( (char*)(MODE_NAMES[mode]) );
        }
        __bis_SR_register(LPM3_bits | GIE);     //enter low power mode
    }
}


void IR_Mode_Setting(){
    if(intro == TRUE) return;

    LCD_Clear();

    if(button_num == 2) {
        copy_mode = TRUE;
        LCD_Text("COPY");
        IR_status = DISABLED;
    }
    else if(button_num == 1) {
        if(copy_mode == TRUE)
        {
            copy_mode = FALSE; //Move out of copy mode into emitting mode
        }

        LCD_Text("OK");
        IR_status = DISABLED;

        P4OUT &= ~(BIT0);
    }
    else if(button_num > 2){ //Codeable button pressed
        code_num = button_num - 3;

        if(copy_mode == TRUE){  //copy mode => Perform copy
            P4OUT |= (BIT0);

            FRAM_write_ptr = (unsigned int *)(FRAM_START_ADDRESSES[mode] + ((code_num)<<9));    //set FRAM write address
            SYSCFG0 &= ~PFWP;
            rx_cnt[mode][code_num] = 0;
            SYSCFG0 |= PFWP;

            IR_status = RECEIVING;
        }
        else{ //transmit mode => Perform transmit
            if(rx_cnt[mode][code_num] > 0) {  // valid IR code
                LCD_IR_Buttons(button_num);
                IR_status = TRANSMITTING;
            }
            else {                           // no IR code
                LCD_Text("NONE");
                IR_status = DISABLED;
            }

            FRAM_read_ptr  = (unsigned int *)(FRAM_START_ADDRESSES[mode] + ((code_num)<<9));    //set FRAM read address
        }
    }
    else{
        LCD_Text( (char*)(MODE_NAMES[mode]) );
        IR_status = DISABLED;
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

//Init ADC
void Init_ADC(){
    //Initialize the ADC Module
    /*
     * Base Address for the ADC Module
     * Use Timer trigger 1 as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     * Repeat Single channel
     * (For digital voltmeter, use repeat multichannel)
     */
    ADCCTL0 &= ~(ADCON | ADCENC | ADCSC);//Set ADCENC as 0 so as to begin init.

    ADCCTL1 |= ADCSHS_2 | ADCDIV_0 | ADCSSEL_0 | ADCCONSEQ_3;//ADCCONSEQ_2;
    ADCIE &= 0x0000;

    ADCCTL0 |= ADCON;

    //Configure Memory Buffer
    /*
     * Base Address for the ADC Module
     * Use input A12 Temp Sensor
     * Use positive reference of Internally generated Vref
     * Use negative reference of AVss
     */

    ADCMCTL0 |= ADCINCH_3; //Digital Voltmeter
    ADCMCTL0 |= ADCINCH_12; //Temperature Sensor
    ADCMCTL0 |= ADCSREF_0; //REF0: V+ = 3.3V; REF1: V+ = 1.5V

    ADCCTL2 |= ADCRES_1; //10-bit

    //Enable and clear all interrupts
    ADCIE |= ADCIE0;
    ADCIFG &= 0x0000;

    //Start & Enable Conversion
    ADCCTL0 |= ADCENC | ADCSC;

    // Enable internal reference and temperature sensor
    PMMCTL0_H = PMMPW_H; //Need to set password to set the registers
    PMMCTL2 |= INTREFEN | TSENSOREN;
    PMMCTL0_H = 0x00;   //Reset, otherwise the FRAM will reset, and the code will start again from top.

    /*-------- INIT TA1.1 WHICH IS USED AS TIMER SOURCE FOR SAMPLING ----------*/
    TA1CCR0 = 0x1000; //Period

    TA1CTL |= TASSEL__ACLK | ID__1 | MC__UP | TACLR;
           // ACLK,    DIVIDER 1   UP_MODE,  CLEAR TIMER
           // ACLK/4 = 32768/1Hz ( ACLK frequency has been set under Init_Clock(). Linked to XT1)
    TA1CTL &= ~(TAIE | TAIFG); //DISABLE INTERRUPT, CLEAR FLAG

    //DISABLE INTERRUPT FOR TA1.1 and CCR0
    TA1CCTL0 &= ~(CCIE | CCIFG);
    TA1CCTL1 &= ~(CCIE | CCIFG);

    TA1CCTL1 &= ~(CAP); //COMPARE MODE
    TA1CCTL1 |= OUTMOD_7; //OUTPUT MODE 7 (Reset/set)
    TA1CCTL1 &= ~(CCIE | CCIFG); //DISABLE NTERRUPTS, CLEAR FLAG
    TA1CCR1 = 0x1000; //COMPARE VALUE
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

                if(intro == FALSE){
                    copy_mode = FALSE;
                    IR_status = DISABLED;

                    mode++;
                    if(mode>=TOTAL_MODES) mode = 0;

                    LCD_Text( (char*)(MODE_NAMES[mode]) );
                }
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

                if(intro == FALSE){
                    copy_mode = FALSE;
                    IR_status = DISABLED;

                    mode++;
                    if(mode>=TOTAL_MODES) mode = 0;

                    LCD_Text( (char*)(MODE_NAMES[mode]) );
                }
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

/*
 * ADC Interrupt Service Routine
 * Show the value of the temperature on the LCD Screen by exiting LPM3.
 */
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void) {
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            break;
        case ADCIV_ADCTOVIFG:
            break;
        case ADCIV_ADCHIIFG:
            break;
        case ADCIV_ADCLOIFG:
            break;
        case ADCIV_ADCINIFG:
            break;
        case ADCIV_ADCIFG:
        	    // Clear interrupt flag
			ADCIFG &= ~(ADCIFG0);
            if(channel) channel--;
            else channel = MAX_ADC_CHANNEL;

            switch(channel+1){
                case 3:
                //case 12:
                    __bic_SR_register_on_exit(LPM3_bits);                // Exit LPM3
                    break;
            }
            break;
        default:
            break;
    }
}

/* CONTROLLING OF IR TX AND RX
 * -    TIMER0.0 and 0.1:
 *
 */

//********Timer0.0 interrupt ISR*********//
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR (void)
{
    switch( TA0IV )
    {
        case TA0IV_NONE:
            if(tx_cnt < rx_cnt[mode][code_num]){ //Transmitting
                P4OUT |= BIT0;

                TA0CCTL2 ^= OUT;
                TA0CCR0 = *(FRAM_read_ptr+1);   //update emitting IR code
                *FRAM_read_ptr++;
                tx_cnt++;
            }
            else{ //Complete
                P4OUT &= ~BIT0;

                IR_status = DISABLED;   //disable IR
                TA0CCTL0 &= ~CCIE;      // disable timer_A0 interrupt
                tx_cnt = 1;

                LCD_Text( (char *)(MODE_NAMES[mode]) );

                __delay_cycles(800000); //TODO: Start a timer and use the timer interrupt to exit LPM3 instead
                __bic_SR_register_on_exit(LPM3_bits);                // Exit LPM3
            }
            break;
        default: break;
    }
}

//********Timer 0.1 and 0.2 interrupt ISR*********//
#pragma vector = TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR (void) {
    switch(__even_in_range(TA0IV,TA0IV_TAIFG)) {
        case TA0IV_NONE: break;
        case TA0IV_TACCR1: //TA0.1
            break;
        case TA0IV_TACCR2: //TA0.2
            if(IR_status == RECEIVING && rx_cnt[mode][code_num] < 255) {
                SYSCFG0 &= ~PFWP;
                rx_cnt[mode][code_num]++;
                SYSCFG0 |= PFWP;

                old_cnt = new_cnt;                  //Update the counter value
                new_cnt = TA0CCR2;
                time_cnt = new_cnt-old_cnt;        //Time interval

                SYSCFG0 &= ~PFWP;
                *FRAM_write_ptr = time_cnt;         //write FRAM to store data
                SYSCFG0 |= PFWP;

                FRAM_write_ptr++;

                P4OUT |= BIT0;
                P1OUT &= ~BIT0;
            }
            else{
                //No longer receiving anything...
                P1OUT |= BIT0;
                P4OUT &= ~BIT0;

                //IR_status = DISABLED;
                //__bic_SR_register_on_exit(LPM3_bits); //Exit LPM3
            }
            break;
        case TA0IV_TAIFG:
            break;
        default: break;
    }
}
