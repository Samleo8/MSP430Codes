#define INT_LEN(n) (n>-10 && n<10)?1:\
				   	(n>-100 && n<100)?2:\
					(n>-1000 && n<1000)?3:\
					(n>-10000 && n<10000)?4:\
					(n>-100000 && n<100000)?5:\
					(n>-1000000 && n<1000000)?6:\
					(n>-10000000 && n<10000000)?7:\
					(n>-100000000 && n<100000000)?8:\
					(n>-1000000000 && n<100000000)?9:\
					(n>-10000000000 && n<10000000000)?10:0\

//LCD display definition
#define pos1 4   // Digit A1 - L4
#define pos2 6   // Digit A2 - L6
#define pos3 8   // Digit A3 - L8
#define pos4 10  // Digit A4 - L10
#define pos5 2   // Digit A5 - L2
#define pos6 18  // Digit A6 - L18

extern void LCD_Init();
extern void LCD_Clear();
extern void LCD_Digit(unsigned char, unsigned char);
extern void LCD_Letter(char, unsigned char);
extern void LCD_Number(long);
extern void LCD_Negative_Sign(void);
extern void LCD_Decimal_Point(unsigned char);
extern void LCD_Degree_Symbol(void);
extern void LCD_IR_Buttons(unsigned char);
extern void LCD_Text(char*);
