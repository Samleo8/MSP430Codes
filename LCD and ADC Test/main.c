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
 * 		TempSensor.c/h
****************************/

#include "main.h"
#include "LCD.h"
#include "TempSensor.h"

extern const unsigned char POS[7];

unsigned int channel = MAX_ADC_CHANNEL;

long count = 0;
signed short temp = 0;
long voltage = 0;
long degC = 0;

long adc_val;

extern volatile unsigned char tempSensorRunning;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	Init_GPIO();
	Init_Clock();
    Init_Temp();

    LCD_Init();
	
	__enable_interrupt();
	
	while(1){
	    //if(tempSensorRunning==FALSE) continue;

	    __bis_SR_register(LPM3_bits | GIE);                       // LPM3 with interrupts enabled
		__no_operation();                                         // Only for debugger

		P4OUT ^= BIT0;

		switch(channel+1){
		    case 3:
                adc_val = ADCMEM0;

                if(adc_val == 1023){
                    LCD_Letter('M',pos1);
                    LCD_Letter('A',pos2);
                    LCD_Letter('X',pos3);
                    break;
                }

                voltage = adc_val*ADC_TO_V_REF_3V3;

                if(voltage<100){ //0.xx voltage
                    voltage+=100;
                    LCD_Number(voltage);
                    LCD_Digit(0,pos1);
                    LCD_Decimal_Point(pos1);
                }
                else if(voltage<1000){ //x.yy voltage
                    LCD_Number(voltage);
                    LCD_Decimal_Point(pos1);
                }
                else{ //xx.yy voltage
                    LCD_Number(voltage);
                    LCD_Decimal_Point(pos2);
                }
                LCD_Letter('V', pos6);
		        break;
		    case 12:
                //ADC is reading INCH12 (Temperature sensor)
                temp = (ADCMEM0-CALADC_15V_30C);

                degC = 300 + temp*ADC_TO_TEMP;

                LCD_Number(degC);

                LCD_Degree_Symbol();
                LCD_Decimal_Point(pos2);
                LCD_Letter('C',pos6);
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
	
	//Configure ADC Positive Reference as Input
	P1DIR &= ~(BIT1);
	
	//Configure P1.3 as input for digital voltmeter
	P1DIR &= ~(BIT3);

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

    //*-------- INIT TA1.1 WHICH IS USED AS TIMER SOURCE FOR SAMPLING ----------*//
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
            tempSensorRunning ^= TRUE; //toggle tempSensorRunning
            P1OUT ^= BIT0; //toggle red LED
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
            break;
        case P2IV_P2IFG7 : break;
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

/*
 * TIMER 0.1/0.2 Interrupt Service Routine
 * Handles timer interrupts at CCR1/2
 */
#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer1_A1_ISR(void)
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
