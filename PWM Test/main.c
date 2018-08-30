/***************************
 * MAIN.C
 * Contains main code
 *
 * Functions:
 *      Init_GPIO: Initializes ports
 *      Init_Clock: Initializes XT1 OSC and DCO
 *
 * Connects to:
 *      LCD.c/h
 *      timer.c/h
****************************/

#include "msp430fr4133.h"
#include "main.h"
#include "LCD.h"

volatile int count = 0;
volatile int MotorPowers[2] = {0,0};

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	Init_GPIO();
    Init_Clock();

    LCD_Init();

    Init_Timer();

    __enable_interrupt();

    __bis_SR_register(LPM3_bits | GIE);                       // LPM3 with interrupts enabled
    __no_operation();                                         // Only for debugger

	return 0;
}
//Initialize GPIO
void Init_GPIO() {
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

    PM5CTL0 &= ~LOCKLPM5;                       // Disable the GPIO power-on default high-impedance mode
                                                // to activate previously configured port setting
}

//Initialize Clock
void Init_Clock() {
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

//Initialize Timer
void Init_Timer(){
    /*
     * Initialize Timers:
     *      TA0.1 at P1.7 for PWM1
     *      TA0.2 at P1.6 for PWM2
     *
     *  Use Clock Source: SMCLK
     *      TAxCLK: (input)
     *      ACLK: 32.768kHz
     *      SMCLK: MCLK/2 = 4MHz
     *      INCLK: ???
     *
     *  FOR PWM:
     *      - Up mode, Reset/Set, Compare Mode
     */

    TA0CTL |= MC__STOP; TA0CTL &= ~(TACLR); //Stop and reset timer
    TA0CTL |= MC__UP | TASSEL__SMCLK | ID__8 | TAIE;
            // Up mode, ACLK; Divide by 8; Enable interrupt
    TA0CTL &= ~(TAIFG); //Clear interrupt flag
    TA0EX0 |= TAIDEX_7; //Further divide by 8

    TA0CCR0 = PWM_ONE_SECOND;

    //TA0.1
    TA0CCTL1 |= OUTMOD_7 | CCIE;
    TA0CCTL1 &= ~(CAP | CCIFG); //Compare mode, clear interrupt flag

    TA0CCR1 = PWM_PERIOD*PWM_STOP;

    //TA0.2
    TA0CCTL2 |= OUTMOD_7 | CCIE;
    TA0CCTL2 &= ~(CAP | CCIFG); //Compare mode, clear interrupt flag

    TA0CCR2 = PWM_PERIOD*PWM_STOP;

    P1SEL0 |= BIT6 | BIT7; // allow for PWM output
}

/* MOTOR CONTROL
 *  motor [0 to N_MOTORS]: which motor to control; 0 implies all motors selected
 *  power [-10 to 10]: how fast motor should run at; sign indicates direction
 */
void MotorControl(unsigned int motor, int power){
    if(power<-10)  power = -10;
    if(power>10)   power = 10;

    //TODO: To conserve power by reducing multiplying consumption, create a hash table with pre-set values
    double PWM_Duty = PWM_STOP + power*PWM_MULTIPLIER;

    switch(motor){
        case 1: //TA0.1, P1.7
            TA0CCR1 = PWM_Duty*PWM_PERIOD;
            LCD_Number(power);

            LCD_Letter('M',pos5);
            LCD_Digit(1,pos6);
            break;
        case 2: //TA0.2, P1.6
            TA0CCR1 = PWM_Duty*PWM_PERIOD;
            LCD_Number(power);

            LCD_Letter('M',pos5);
            LCD_Digit(2,pos6);
            break;
        case 0: //Everything
            TA0CCR1 = PWM_Duty*PWM_PERIOD;
            LCD_Number(power);

            LCD_Letter('M',pos5);
            LCD_Digit(0,pos6);
            break;
        default: return;
    }
}

/*
 * TIMER 0.1/0.2 Interrupt Service Routine
 * Handles timer interrupts at CCR1/2
 */
#pragma vector = TIMER0_A1_VECTOR
__interrupt void Timer0_A1_ISR(void)
{
    switch(__even_in_range(TA0IV,TA0IV_TAIFG))
    {
        case TA0IV_NONE: break;
        case TA0IV_TACCR1: //TA0.1
            break;
        case TA0IV_TACCR2: //TA0.2
            break;
        case TA0IV_TAIFG: //CCRO Reached
            //count++;
            //LCD_Number(count);
            break;
        default: break;
    }
}


/*
 * PORT1 Interrupt Service Routine
 * Handles S1 button press interrupt
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
            //Do something on button press
            MotorPowers[0]++;
            if(MotorPowers[0]>10) MotorPowers[0]=-10;

            MotorControl(1,MotorPowers[0]);
            break;
        case P1IV_P1IFG3 : break;
        case P1IV_P1IFG4 : break;
        case P1IV_P1IFG5 : break;
        case P1IV_P1IFG6 : break;
        case P1IV_P1IFG7 : break;
    }
}

/*
 * PORT2 Interrupt Service Routine
 * Handles S2 button press interrupt
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
            //Do something on button press
            MotorPowers[1]++;
            if(MotorPowers[1]>10) MotorPowers[1]=-10;

            MotorControl(2,MotorPowers[1]);
            break;
        case P2IV_P2IFG7 : break;
    }
}
