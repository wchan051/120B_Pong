#ifndef io_h
#define io_h

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#define SET_BIT(p,i) ((p) |= (1 << (i)))
#define CLR_BIT(p,i) ((p) &= ~(1 << (i)))
#define GET_BIT(p,i) ((p) & (1 << (i)))

#define DATA_BUS PORTC	
#define CONTROL_BUS PORTD
#define RS 6	
#define E 7	


void LCD_ClearScreen(void) {
	LCD_WriteCommand(0x01);
}

void LCD_init(void) {

	delay_ms(100);
	LCD_WriteCommand(0x38);
	LCD_WriteCommand(0x06);
	LCD_WriteCommand(0x0f);
	LCD_WriteCommand(0x01);
	delay_ms(10);
}

void LCD_WriteCommand (unsigned char Command) {
	CLR_BIT(CONTROL_BUS,RS);
	DATA_BUS = Command;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(2); 
}

void LCD_WriteData(unsigned char Data) {
	SET_BIT(CONTROL_BUS,RS);
	DATA_BUS = Data;
	SET_BIT(CONTROL_BUS,E);
	asm("nop");
	CLR_BIT(CONTROL_BUS,E);
	delay_ms(1);
}

void LCD_DisplayString( unsigned char column, const unsigned char* string) {
	LCD_ClearScreen();
	unsigned char c = column;
	while(*string) {
		LCD_Cursor(c++);
		LCD_WriteData(*string++);
	}
}

void LCD_Cursor(unsigned char column) {
	if ( column < 17 ) { 
		LCD_WriteCommand(0x80 + column - 1);
		} else {
		LCD_WriteCommand(0xB8 + column - 9);
	}
}

void delay_ms(int miliSec)

{
	int i,j;
	for(i=0;i<miliSec;i++)
	for(j=0;j<775;j++)
	{
		asm("nop");
	}
}

#endif
