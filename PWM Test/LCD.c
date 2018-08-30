/***************************
 * LCD.C
 * Contains code for LCD
 *
 * Functions:
 *      LCD_Init: Initializes ports for LCD
 *      LCD_Clear(): Clears the LCD Screen
 *      LCD_Digit(digit, pos): Outputs `digit` at `pos`
 *      LCD_Letter(letter, pos): Outputs 'letter' at `pos`
 *      LCD_Number(num): Outputs up-to-6-digit `num` (starting from left-most pos)
 *      LCD_Negative_Sign(): Prints negative sign
 *      LCD_Decimal_Point(pos): Prints decimal point at`pos`
 *      LCD_Degree_Symbol(): Prints the degree symbol for temperature display
 *
 * Header Files:
 *      MSP430FR4133.h
 *      main.h (for the constants)
 *      LCD.h
****************************/

#include "msp430fr4133.h"
#include "main.h"
#include "LCD.h"

const unsigned char POS[7] = {0, pos1, pos2, pos3, pos4, pos5, pos6};

//LCD digit display table
const char digit[10] =
{
		0xFC,  /* "0" */
		0x60,  /* "1" */
		0xDB,  /* "2" */
		0xF3,  /* "3" */
		0x67,  /* "4" */
		0xB7,  /* "5" */
		0xBF,  /* "6" */
		0xE4,  /* "7" */
		0xFF,  /* "8" */
		0xF7   /* "9" */
};

//LCD alphabet display table
const char alphabet[28][2] =
{
		{0xEF, 0x00},  /* "A" */ //0
		{0xF1, 0x50},  /* "B" */ //1
		{0x9C, 0x00},  /* "C" */ //2
		{0xF0, 0x50},  /* "D" */ //3
		{0x9F, 0x00},  /* "E" */ //4
		{0x8F, 0x00},  /* "F" */ //5
		{0xBD, 0x00},  /* "G" */ //6
		{0x6F, 0x00},  /* "H" */ //7
		{0x90, 0x50},  /* "I" */ //8
		{0x78, 0x00},  /* "J" */ //9
		{0x0E, 0x22},  /* "K" */ //10
		{0x1C, 0x00},  /* "L" */ //11
		{0x6C, 0xA0},  /* "M" */ //12
		{0x6C, 0x82},  /* "N" */ //13
		{0xFC, 0x00},  /* "O" */ //14
		{0xCF, 0x00},  /* "P" */ //15
		{0xFC, 0x02},  /* "Q" */ //16
		{0xCF, 0x02},  /* "R" */ //17
		{0xB7, 0x00},  /* "S" */ //18
		{0x80, 0x50},  /* "T" */ //19
		{0x7C, 0x00},  /* "U" */ //20
		{0x0C, 0x28},  /* "V" */ //21
		{0x6C, 0x0A},  /* "W" */ //22
		{0x00, 0xAA},  /* "X" */ //23
		{0x00, 0xB0},  /* "Y" */ //24
		{0x90, 0x28},  /* "Z" */ //25
		{0x03, 0x50},  /* "+" */ //26
		{0x03, 0x00}   /* "-" */ //27
};

// Initialize LCD
void LCD_Init()
{
	//Ensure no GPIO Pin conflict
	//5.1 is an LCD Drive PIN
	//--Output HIGH, Direction INPUT
	P5OUT |= BIT2;
	P5OUT &= ~(BIT2);
	
	// Configure LCD pins
	SYSCFG2 |= LCDPCTL;                              // R13/R23/R33/LCDCAP0/LCDCAP1 pins selected

	LCDPCTL0 = 0xFFFF;
	LCDPCTL1 = 0x07FF;
	LCDPCTL2 = 0x00F0;                               // L0~L26 & L36~L39 pins selected

	LCDCTL0 = LCDSSEL_0 | LCDDIV_7;                  // flcd ref freq is xtclk

	//LCD Operation - Mode 2, internal 3.08v, charge pump 256Hz
	LCDVCTL = LCDCPEN | LCDSELVDD | VLCD_8 | (LCDCPFSEL0 | LCDCPFSEL1 | LCDCPFSEL2 | LCDCPFSEL3);
	/*
	// LCD Operation - Mode 3, internal 3.08v, charge pump 256Hz
	LCDVCTL = LCDCPEN | LCDREFEN | VLCD_8 | (LCDCPFSEL0 | LCDCPFSEL1 | LCDCPFSEL2 | LCDCPFSEL3);
	 */
	LCDMEMCTL |= LCDCLRM;                             // Clear LCD memory

	LCDCSSEL0 = 0x000F;                               // Configure COMs and SEGs
	LCDCSSEL1 = 0x0000;                               // L0, L1, L2, L3: COM pins
	LCDCSSEL2 = 0x0000;

	LCDM0 = 0x21;                                     // L0 = COM0, L1 = COM1
	LCDM1 = 0x84;                                     // L2 = COM2, L3 = COM3

	LCDCTL0 |= LCD4MUX | LCDON;                       // Turn on LCD, 4-mux selected

}

// Clear all the LCD display
void LCD_Clear()
{
	unsigned char i;
	for(i=2; i<20; i++)
		LCDMEM[i] = 0x00;
}

// LCD digit display function
void LCD_Digit(unsigned char dg, unsigned char pos)
{
	LCDMEM[pos] = digit[dg];
}

// LCD letter display function
void LCD_Letter(char ch, unsigned char pos)
{
	LCDMEM[pos]   = alphabet[ch-'A'][0];
	LCDMEM[pos+1] = alphabet[ch-'A'][1];
}

void LCD_Number(long n){
	LCD_Clear();
	
	if(n<0){
		LCD_Negative_Sign();
		n = -n;	
	}
	
	unsigned int len = INT_LEN(n);
	
	if(len==0 || len>6){
		LCD_Letter('I',pos1);
		LCD_Letter('N',pos2);
		LCD_Letter('F',pos3);
		return;
	}
	
	unsigned int i;
	unsigned char lastDigit;
	for(i=len;i>0;i--){
		lastDigit = (unsigned char)(n%10);
		LCD_Digit(lastDigit,POS[i]);
		n/=10;
	}
}

void LCD_Negative_Sign(){
	LCDMEM[pos1+1] |= 0x04;
}

void LCD_Decimal_Point(unsigned char pos){
	LCDMEM[pos+1] |= 0x01;
}

void LCD_Degree_Symbol(){
	LCDMEM[pos5+1] |= 0x04;
}
