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
 *      LCD_IR_Keypad(btn): Prints the corresponding button that is pressed
 *
 * Header Files:
 *      MSP430FR4133.h
 *      main.h (for the constants)
 *      LCD.h
****************************/

#include "msp430fr4133.h"
#include "main.h"
#include "LCD.h"
//#include "string.h"

static const unsigned char POS[7] = {0, pos1, pos2, pos3, pos4, pos5, pos6};

//LCD digit display table
static const char digit[10] =
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
static const char alphabet[28][2] =
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

	//LCD Operation - Mode 2, internal 2.96v, charge pump 256Hz
	LCDVCTL = LCDCPEN | LCDSELVDD | VLCD_6 | (LCDCPFSEL0 | LCDCPFSEL1 | LCDCPFSEL2 | LCDCPFSEL3);

	LCDMEMCTL |= LCDCLRM;                             // Clear LCD memory

	LCDCSSEL0 = 0x000F;                               // Configure COMs and SEGs
	LCDCSSEL1 = 0x0000;                               // L0, L1, L2, L3: COM pins
	LCDCSSEL2 = 0x0000;

	LCDM0 = 0x21;                                     // L0 = COM0, L1 = COM1
	LCDM1 = 0x84;                                     // L2 = COM2, L3 = COM3

	LCDCTL0 |= LCD4MUX | LCDON;                       // Turn on LCD, 4-mux selected

}

// Clear the LCD segments
void LCD_Clear()
{
	unsigned char i = 18;
	while(i--)
		LCDMEM[i+2] = 0x00;

    //LCDMEMCTL |= LCDCLRM;   //Actually clear LCD memory
}

// LCD digit display function
void LCD_Digit(unsigned char dg, unsigned char pos)
{
	LCDMEM[pos] = digit[dg];
}

// LCD letter display function
void LCD_Letter(char ch, unsigned char pos)
{
    switch(ch){
        case ' ':
            LCDMEM[pos] = 0x00;
            LCDMEM[pos+1] = 0x00;
            break;
        case '.':
            LCDMEM[pos+1] = 0x01;
            break;
        case '+':
            LCDMEM[pos]   = alphabet[26][0];
            LCDMEM[pos+1] = alphabet[26][1];
            break;
        case '-':
            LCDMEM[pos]   = alphabet[27][0];
            LCDMEM[pos+1] = alphabet[27][1];
            break;
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
            LCDMEM[pos]   = alphabet[ch-'a'][0];
            LCDMEM[pos+1] = alphabet[ch-'a'][1];
            break;
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
            LCDMEM[pos]   = alphabet[ch-'A'][0];
            LCDMEM[pos+1] = alphabet[ch-'A'][1];
            break;
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
            LCDMEM[pos] = digit[ch-'0'];
            break;
        default:
            LCDMEM[pos] = 0x00;
            LCDMEM[pos+1] = 0x00;
            break;
    }
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
	
	unsigned int i = len;
	unsigned char lastDigit;
	while(i--){
		lastDigit = (unsigned char)(n%10);
		LCD_Digit(lastDigit,POS[i+1]);
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

void LCD_IR_Buttons(unsigned char btn) {
    switch(btn){
        case OK:
            LCD_Letter('O',pos1);
            LCD_Letter('K',pos2);
            break;
        case COPY:
            LCD_Letter('C',pos1);
            LCD_Letter('O',pos2);
            LCD_Letter('P',pos3);
            LCD_Letter('Y',pos4);
            break;
        case TEMP_MINUS:
            LCD_Letter('T',pos1);
            LCD_Letter('E',pos2);
            LCD_Letter('M',pos3);
            LCD_Letter('P',pos4);
            LCD_Letter('-',pos5);
            break;
        case TEMP_PLUS:
            LCD_Letter('T',pos1);
            LCD_Letter('E',pos2);
            LCD_Letter('M',pos3);
            LCD_Letter('P',pos4);
            LCD_Letter('+',pos5);
            break;
        case COOL:
            LCD_Letter('C',pos1);
            LCD_Letter('O',pos2);
            LCD_Letter('O',pos3);
            LCD_Letter('L',pos4);
            break;
        case KEY_9:
            LCD_Digit(9, pos1);
            break;
        case KEY_6:
            LCD_Digit(6, pos1);
            break;
        case KEY_3:
            LCD_Digit(3, pos1);
            break;
        case KEY_0:
            LCD_Digit(0, pos1);
            break;
        case KEY_8:
            LCD_Digit(8, pos1);
            break;
        case KEY_5:
            LCD_Digit(5, pos1);
            break;
        case KEY_2:
            LCD_Digit(2, pos1);
            break;
        case POWER:
            LCD_Letter('P',pos1);
            LCD_Letter('O',pos2);
            LCD_Letter('W',pos3);
            LCD_Letter('E',pos4);
            LCD_Letter('R',pos5);
            break;
        case KEY_7:
            LCD_Digit(7, pos1);
            break;
        case KEY_4:
            LCD_Digit(4, pos1);
            break;
        case KEY_1:
            LCD_Digit(1, pos1);
            break;
        default:
            LCD_Letter('E',pos1);
            LCD_Letter('R',pos2);
            LCD_Letter('R',pos3);
            LCD_Letter('O',pos4);
            LCD_Letter('R',pos5);
            break;
    }
}

void LCD_Text(char *msg){
    //unsigned int len = strlen(msg);
    unsigned int len = 0;
    while(msg[len]!='\0') len++; //gets length of string (didn't want to include heavy string.h library just for strlen)

    unsigned char i,j;

    LCD_Clear();

    if(len<7){
        for(i=0;i<len;i++){
            LCD_Letter(msg[i],POS[i+1]);
        }
        return;
    }

    for (i=0; i<len; i++){
        if(buttonDebounce == BUTTON_PRESSED) return;

        j=6;
        while(j--){
            LCD_Letter( (i+j<len)?msg[i+j]:' ', POS[j+1]);
        }

        //TODO: Instead of delay cycles, use a timer.
        __delay_cycles(2000000); //8000000 delay is 1s => this delay is 0.25s

        LCD_Clear();
    }

    __delay_cycles(1000000); //8000000 delay is 1s => this delay is 0.125s
}

