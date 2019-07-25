/*	Author: wchan051 - Wayland Chang
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include "io.h" 
#include "timer.h"

unsigned char FINISH = 0;
unsigned char WRITE = 0;
unsigned char READY = 0;
unsigned char SINGLEPLAYER = 0;
unsigned char MULTIPLAYER = 0;

unsigned char P1SCORE = 0; 
unsigned char P2SCORE = 0; 
unsigned char P1BOOL = 0; 
unsigned char P2BOOL = 0; 

unsigned char P1ROW_MOVEMENT[6] = {0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0};
unsigned char P1index = 4;
unsigned char P1COL = 0xFE;

unsigned char P2ROW_MOVEMENT[6] = {0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0};
unsigned char P2index = 4;
unsigned char P2COL = 0x7F;

unsigned char BALLROW_MOVEMENT[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char BALLCOL_MOVEMENT[8] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};
unsigned char BRindex = 3;
unsigned char BCindex = 3;


#define read_eeprom_word(address) eeprom_read_word ((const uint16_t*)address)
#define write_eeprom_word(address,value) eeprom_write_word ((uint16_t*)address,(uint16_t)value)
#define update_eeprom_word(address,value) eeprom_update_word ((uint16_t*)address,(uint16_t)value)

enum Menus {home, wait, play, wait_score, writeP1, writeP2, winP1, winP2, record_win} Menu;
unsigned short counterwin = 0;
unsigned char EEMEM eeprom_array[6];
unsigned char tmpD;

void MenuScreen()
{
	switch(Menu)
	{
		case home:
			if((tmpD & 0x1F) == 0x00)
			{
				SINGLEPLAYER = 0;
				MULTIPLAYER = 0;
				P1SCORE = 0;
				P2SCORE = 0;
				Menu = home;
			}
			else if((tmpD & 0x1F) == 0x01)
			{
				SINGLEPLAYER = 1;
				MULTIPLAYER = 0;
				Menu = wait;
			}
			else if((tmpD & 0x1F) == 0x02)
			{
				SINGLEPLAYER = 0;
				MULTIPLAYER = 1;
				Menu = wait;
			}
			break;
		case wait:
			if((tmpD & 0x1F) != 0)
			{
				if((tmpD & 0x1F) == 0x01)
				{
					SINGLEPLAYER = 1;
					MULTIPLAYER = 0;
				}
				else if((tmpD & 0x1F) == 0x02)
				{
					SINGLEPLAYER = 0;
					MULTIPLAYER = 1;
				}
					
				Menu= wait;
			}
			else
				Menu = play;
			break;
		case play:
			Menu = wait_score;
			break;
		case wait_score:
			if(!P1BOOL && !P2BOOL)
				Menu = wait_score;
			else if(P1BOOL)
				Menu = writeP1;
			else if(P2BOOL)
				Menu = writeP2;
			break;
		case writeP1:
			if(P1SCORE == 4) 
				Menu = winP1;
			else
				Menu = wait_score;
			break;
		case writeP2:
			if(P2SCORE == 4)
				Menu = winP2;
			else
				Menu = wait_score;
			break;
		case winP1:
			Menu = record_win;
			break;
		case winP2:
			Menu = record_win;
			break;
		case record_win:
			SINGLEPLAYER = 0;
			MULTIPLAYER = 0;
			if(counterwin == 50000)
			{
				counterwin = 0;
				Menu = home;
			}
			else
				Menu = record_win;
			break;
	}
	switch(Menu)
	{
		case home:
			FINISH = 0;
			READY = 0;
			if(WRITE == 0)
			{
				LCD_ClearScreen();
				LCD_DisplayString(1, "B1) SINGLEPLAYERB2) MULTIPLAYER");
				WRITE = 1;
			}
			break;
		case wait:
			WRITE = 0;
			break;
		case play:
			READY = 1;
			if(WRITE == 0)
			{
				LCD_DisplayString(1, "Score:          P1)        P2)");
				WRITE = 1;
			}
			break;
		case wait_score:
			break;
		case writeP1:
			LCD_Cursor(20);
			LCD_WriteData(P1SCORE + '0');
			break;
		case writeP2:
			LCD_Cursor(31);
			LCD_WriteData(P2SCORE + '0');
			break;
		case winP1:
			SINGLEPLAYER = 0;
			MULTIPLAYER = 0;
			READY = 0;
			LCD_DisplayString(1, "    P1 WINS!");
			write_eeprom_word(&eeprom_array[0], read_eeprom_word(&eeprom_array[0]) + 1);
			write_eeprom_word(&eeprom_array[3], read_eeprom_word(&eeprom_array[3]) + 1);
			break;
		case winP2:
			SINGLEPLAYER = 0;
			MULTIPLAYER = 0;
			READY = 0;
			LCD_DisplayString(1, "    P2 WINS!");
			write_eeprom_word(&eeprom_array[0], read_eeprom_word(&eeprom_array[0]) + 1);
			write_eeprom_word(&eeprom_array[5], read_eeprom_word(&eeprom_array[5]) + 1);
			break;
		case record_win:
			SINGLEPLAYER = 0;
			MULTIPLAYER = 0;
			READY = 0;
			counterwin++;
			WRITE = 0;
			break;
	}	
}


enum MatrixUpdate {wait_Ready, wait_Ready2, updateP1, updateP2, updateBall} Matrix;
unsigned char a = 3;

unsigned char up = 1;
unsigned short countermatrix = 0;
void MatrixPlay()
{
	switch(Matrix)
	{
		case wait_Ready:
			if(!READY)
			{
				Matrix = wait_Ready;
			}
			else if(READY)
				Matrix = updateP1;
			break;
		case wait_Ready2:
			if(!READY)
			{
				Matrix = wait_Ready2;
			}
			else if(READY)
				Matrix = updateP1;
			break;
		case updateP1:
			if(!READY)
				Matrix = wait_Ready;
			else if(READY)
				Matrix = updateP2;
			break;
		case updateP2:
			if(!READY)
				Matrix = wait_Ready;
			else if(READY)
				Matrix = updateBall;
			break;
		case updateBall:
			if(!READY)
				Matrix = wait_Ready;
			else if(READY)
				Matrix = updateP1;
			break;
	}
	switch(Matrix)
	{
		case wait_Ready:
			countermatrix++;
			PORTB = P1ROW_MOVEMENT[a];
			PORTA = 0x7E;
			P1COL = 0xFE;
			P2COL = 0x7F;
			break;
		case wait_Ready2:
			countermatrix++;
			PORTB = P1ROW_MOVEMENT[a];
			PORTA = 0x7E;
			P1COL = 0xFE;
			P2COL = 0x7F;
		break;
		case updateP1:
			PORTB = P1ROW_MOVEMENT[P1index];
			PORTA = P1COL;
			break;
		case updateP2:
			PORTB = P2ROW_MOVEMENT[P2index];
			PORTA = P2COL;
			break;
		case updateBall:
			PORTB = BALLROW_MOVEMENT[BRindex];
			PORTA = BALLCOL_MOVEMENT[BCindex];
			break;	
	}
}

enum Movement_P1 {wait_Ready_p1, move_p1, up_p1, down_p1, holdp1} MovementP1;
unsigned short counter1 = 0;
void MoveP1()
{
	switch(MovementP1)
	{
		case wait_Ready_p1:
			if(!READY)
				MovementP1 = wait_Ready_p1;
			else if (READY)
				MovementP1 = move_p1;
			break;
		case move_p1:
			if(READY)
			{
				if((!(tmpD & 0x01) && !(tmpD & 0x02)) || ((tmpD & 0x01) && (tmpD & 0x02)))
					MovementP1 = move_p1;
				else if(tmpD & 0x01)
					MovementP1 = up_p1;
				else if(tmpD & 0x02)
					MovementP1 = down_p1;
			}
			else
				MovementP1 = wait_Ready_p1;
			break;
		case up_p1:
			MovementP1 = holdp1;
			break;
		case down_p1:
			MovementP1 = holdp1;
			break;
		case holdp1:
			if(counter1 == 2500)
				MovementP1 = move_p1;
			else
				MovementP1 = holdp1;
			break;
	}	
	switch(MovementP1)
	{
		case wait_Ready_p1:
			counter1 = 0;
			break;
		case move_p1:
			counter1 = 0;
			break;
		case up_p1:
			if(P1index < 5)
				P1index++;
			else
				P1index = P1index;
			break;
		case down_p1:
			if(P1index > 0)
				P1index--;
			else
				P1index = P1index;
			break;
		case holdp1:
			counter1++;
			break;
	}
}

enum Movement_P2 {wait_Ready_p2, move_p2, up_p2, down_p2, holdp2} MovementP2;
unsigned short counter2 = 0;
void MoveP2()
{
	switch(MovementP2)
	{
		case wait_Ready_p2:
			if(!READY && !MULTIPLAYER)
				MovementP2 = wait_Ready_p2;
			else if (READY && MULTIPLAYER && !SINGLEPLAYER)
				MovementP2 = move_p2;
			else
				MovementP2 = wait_Ready_p2;
			break;
		case move_p2:
			if(!READY)
				MovementP2 = wait_Ready_p2;
			else if(READY && MULTIPLAYER)
			{
				if((!(tmpD & 0x04) && !(tmpD & 0x08)) || ((tmpD & 0x04) && (tmpD & 0x08)))
					MovementP2 = move_p2;
				else if(tmpD & 0x04)
					MovementP2 = up_p2;
				else if(tmpD & 0x08)
					MovementP2 = down_p2;
			}
			break;
		case up_p2:
			MovementP2 = holdp2;
			break;
		case down_p2:
			MovementP2 = holdp2;
			break;
		case holdp2:
			if(counter2 == 2500)
				MovementP2 = move_p2;
			else
				MovementP2 = holdp2;
			break;
	}
	switch(MovementP2)
	{
		case wait_Ready_p2:
			counter2 = 0;
			break;
		case move_p2:
			counter2 = 0;
			break;
		case up_p2:
			if(P2index < 5)
				P2index++;
			else
				P2index = P2index;
			break;
		case down_p2:
			if(P2index > 0)
				P2index--;
			else
				P2index = P2index;
			break;
		case holdp2:
			counter2++;
			break;
	}
}

enum Movement_P2BOT {wait_Ready_p2bot, move_p2bot, up_p2bot, down_p2bot, holdp2bot} MovementP2bot;
unsigned short counter2bot = 0;
void MoveP2bot()
{
	switch(MovementP2bot)
	{
		case wait_Ready_p2bot:
			if(!READY && !SINGLEPLAYER)
				MovementP2bot = wait_Ready_p2bot;
			else if (READY && SINGLEPLAYER && !MULTIPLAYER)
				MovementP2bot = move_p2bot;
			else
				MovementP2bot = wait_Ready_p2bot;
			break;
		case move_p2bot:
			if(!READY)
				MovementP2 = wait_Ready_p2bot;
			else if(READY && SINGLEPLAYER)
			{
				if((BRindex < P2index))
					MovementP2bot = down_p2bot;
				else if((BRindex > P2index))
					MovementP2bot = up_p2bot;
				else
					MovementP2bot = move_p2bot;	
			}
			break;
		case up_p2bot:
			MovementP2bot = holdp2bot;
			break;
		case down_p2bot:
			MovementP2bot = holdp2bot;
			break;
		case holdp2bot:
			if(counter2bot == 3000)
				MovementP2bot = move_p2bot;
			else
				MovementP2bot = holdp2bot;
			break;
	}
	switch(MovementP2bot)
	{
		case wait_Ready_p2bot:
			counter2bot = 0;
			break;
		case move_p2bot:
			counter2bot = 0;
			break;
		case up_p2bot:
			if(P2index < 5)
				P2index++;
			else
				P2index = P2index;
			break;
		case down_p2bot:
			if(P2index > 0)
				P2index--;
			else
				P2index = P2index;
			break;
		case holdp2bot:
			counter2bot++;
			break;
	}
}

enum BallMoves {wait_ball, start, start_wait, move_ball, holdball} BallMove;
unsigned short counterball = 0;
unsigned short counterstart = 0;
char hit = -1;
char wall = -1;
void BallPlay()
{
	switch(BallMove)
	{
		case wait_Ready_ball:
			if(!READY)
				BallMove = wait_Ready_ball;
			else if(READY)
				BallMove = start;
			break;
		case start:
			if(!READY)
				BallMove = wait_Ready_ball;
			else
				BallMove = start_wait;
			break;
		case start_wait:
			if(counterstart != 10000)
				BallMove = start_wait;
			else if(counterstart == 10000)
			{
				counterstart = 0;
				BallMove = move_ball;
			}	
			break;
		case move_ball:
			counterstart = 0;
			if(BCindex == 0)
			{
				P2BOOL = 1;
				BallMove = start;
			}
			else if(BCindex == 7)
			{
				P1BOOL = 1;
				BallMove = start;
			}
			else
				BallMove = holdball;
			break;
		case holdball:
			if(counterball != (4000))
				BallMove = holdball;
			else
			{
				counterball = 0;
				if(P1BOOL)
					BallMove = start;
				else if(P2BOOL)
					BallMove = start;
				else
					BallMove = move_ball;
			}
			break;
	}	
	switch(BallMove)
	{
		case wait_Ready_ball:
			P1BOOL = 0;
			P2BOOL = 0;
			break;
		case start:
			if(!P1BOOL && !P2BOOL)
			{
				hit = -1;
				wall = 0;
				BRindex = 4;
				BCindex = 4;
				P1index = 3;
				P2index = 3;
				P1BOOL = 0;
				P2BOOL = 0;
			}
			else if(P1BOOL)
			{
				hit = 1;
				wall = 0;
				BRindex = 4;
				BCindex = 2;
				P1index = 3;
				P2index = 3;
				P1SCORE++;
			}
			else if(P2BOOL)
			{
				hit = -1;
				wall = 0;
				BRindex = 4;
				BCindex = 5;
				P1index = 3;
				P2index = 3;
				P2SCORE++;
			}
			break;
		case start_wait:
			P1BOOL = 0;
			P2BOOL = 0;
			counterstart++;
			break;
		case move_ball:
			counterstart = 0;
			BRindex = BRindex + wall;
			BCindex = BCindex + hit;
			
			if(BRindex > 6 || BRindex < 1)
				wall = wall * -1;
			
			if(BCindex < 2)
			{	
				if(BRindex == 0)
				{
					if(P1index == 0)
					{
						hit = 1;
						wall = 1;
					}
				}
				else if(BRindex == 1)
				{
					if(P1index == 0)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1index == 1)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 2)
				{
					if(P1index == 0)
					{
						hit = 1;
						wall = 1;
					}
					else if(P1index == 1)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1index == 2)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 3)
				{
					if(P1index == 1)
					{
						hit = 1;
						wall = 1;
					}
					else if(P1index == 2)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1index == 3)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 4)
				{
					if(P1index == 2)
					{
						hit = 1;
						wall = 1;
					}
					else if(P1index == 3)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1index == 4)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 5)
				{
					if(P1index == 3)
					{
						hit = 1;
						wall = 1;
					}
					else if(P1index == 4)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1index == 5)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 6)
				{
					if(P1index == 4)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1index == 5)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 7)
				{
					if(P1index == 5)
					{
						hit = 1;
						wall = -1;
					}
				}
				else
				{
					P2BOOL = 1;
					
				}
			}
			
			
			else if(BCindex > 5) 
			{
				if(BRindex == 0)
				{
					if(P2index == 0)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 1)
				{
					if(P2index == 0)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2index == 1)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 2)
				{
					if(P2index == 0)
					{
						hit = -1;
						wall = 1;
					}
					else if(P2index == 1)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2index == 2)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 3)
				{
					if(P2index == 1)
					{
						hit = -1;
						wall = 1;
					}
					else if(P2index == 2)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2index == 3)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 4)
				{
					if(P2index == 2)
					{
						hit = -1;
						wall = 1;
					}
					else if(P2index == 3)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2index == 4)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 5)
				{
					if(P2index == 3)
					{
						hit = -1;
						wall = 1;
					}
					else if(P2index == 4)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2index == 5)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 6)
				{
					if(P2index == 4)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2index == 5)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 7)
				{
					if(P2index == 5)
					{
						hit = -1;
						wall = -1;
					}
				}
				else
				{
					P1BOOL = 1;
				}
			}
			break;
		case holdball:
			counterball++;
			break;
	}
}


int main(void)
{
    DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00; 
	DDRD = 0xE0; PORTD = 0x1F; 
	LCD_init();
	
	TimerSet(1);
	TimerOn();
	
	if(read_eeprom_word(&eeprom_array[0]) + '0' == '/')
		write_eeprom_word(&eeprom_array[0], read_eeprom_word(&eeprom_array[0]) + 1);
	if(read_eeprom_word(&eeprom_array[5]) + '0' == '/')
		write_eeprom_word(&eeprom_array[5], read_eeprom_word(&eeprom_array[5]) + 1);
	if(read_eeprom_word(&eeprom_array[3]) + '0' == '/')
		write_eeprom_word(&eeprom_array[3], read_eeprom_word(&eeprom_array[3]) + 1);

	while(1)
    {
		
		tmpD = PIND;
		tmpD = ~tmpD;
		
		MenuScreen();
		MatrixPlay();
		MoveP1();
		MoveP2();
		MoveP2bot();
		BallPlay();
	

    }
}
