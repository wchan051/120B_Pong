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
#include "io.h"

unsigned char READY = 0;
unsigned char WRITE = 0;
unsigned char SINGLEPLAYER = 0;
unsigned char MULTIPLAYER = 0;


unsigned char P1SCORE = 0; 
unsigned char P2SCORE = 0; 
unsigned char P1BOOL = 0; 
unsigned char P2BOOL = 0; 

unsigned char P1ROW_MOVEMENT[6] = {0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0};
unsigned char P1INDEX = 4;
unsigned char P1COL = 0xFE;

unsigned char P2ROW_MOVEMENT[6] = {0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0};
unsigned char P2INDEX = 4;
unsigned char P2COL = 0x7F;

unsigned char BALLROW_MOVEMENT[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char BALLCOL_MOVEMENT[8] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};
unsigned char BRindex = 3;
unsigned char BCindex = 3;

enum Menus {homeMenu, wait, play, wait_score, writeP1, writeP2, winP1, winP2, record_win} Menu;
unsigned short counterwin = 0;
unsigned char tmpD;

void MenuScreen()
{
	switch(Menu)
	{
		case homeMenu:
			if((tmpD & 0x1F) == 0x00)
			{
				SINGLEPLAYER = 0;
				MULTIPLAYER = 0;
				P1SCORE = 0;
				P2SCORE = 0;
				Menu = homeMenu;
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
			if(P1SCORE == 5) 
				Menu = winP1;
			else
				Menu = wait_score;
			break;
		case writeP2:
			if(P2SCORE == 5)
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
				Menu = homeMenu;
			}
			else
				Menu = record_win;
			break;
	}
	switch(Menu)
	{
		case homeMenu:
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
			break;
		case winP2:
			SINGLEPLAYER = 0;
			MULTIPLAYER = 0;
			READY = 0;
			LCD_DisplayString(1, "    P2 WINS!");
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


enum UpdateMatrix {waitReadyP1, waitReadyP2, p1Update, p2Update, ballUpdate} Matrix;
unsigned char row_movement = 3;
unsigned char up = 1;
unsigned short countermatrix = 0;
void MatrixPlay()
{
	switch(Matrix)
	{
		case waitReadyP1:
			if(!READY)
			{
				Matrix = waitReadyP1;
			}
			else if(READY)
				Matrix = p1Update;
			break;
		case waitReadyP2:
			if(!READY)
			{
				Matrix = waitReadyP2;
			}
			else if(READY)
				Matrix = p1Update;
			break;
		case p1Update:
			if(!READY)
				Matrix = waitReadyP1;
			else if(READY)
				Matrix = p2Update;
			break;
		case p2Update:
			if(!READY)
				Matrix = waitReadyP1;
			else if(READY)
				Matrix = ballUpdate;
			break;
		case ballUpdate:
			if(!READY)
				Matrix = waitReadyP1;
			else if(READY)
				Matrix = p1Update;
			break;
	}
	switch(Matrix)
	{
		case waitReadyP1:
			countermatrix++;
			PORTB = P1ROW_MOVEMENT[row_movement];
			PORTA = 0x7E;
			P1COL = 0xFE;
			P2COL = 0x7F;
			break;
		case waitReadyP2:
			countermatrix++;
			PORTB = P1ROW_MOVEMENT[row_movement];
			PORTA = 0x7E;
			P1COL = 0xFE;
			P2COL = 0x7F;
		break;
		case p1Update:
			PORTB = P1ROW_MOVEMENT[P1INDEX];
			PORTA = P1COL;
			break;
		case p2Update:
			PORTB = P2ROW_MOVEMENT[P2INDEX];
			PORTA = P2COL;
			break;
		case ballUpdate:
			PORTB = BALLROW_MOVEMENT[BRindex];
			PORTA = BALLCOL_MOVEMENT[BCindex];
			break;	
	}
}

enum P1Movement {p1_wait, p1_move, p1_up, p1_down, p1_stop} P1Movement;
unsigned short counterp1 = 0;
void MoveP1()
{
	switch(P1Movement)
	{
		case p1_wait:
			if(!READY)
				P1Movement = p1_wait;
			else if (READY)
				P1Movement = p1_move;
			break;
		case p1_move:
			if(READY)
			{
				if((!(tmpD & 0x01) && !(tmpD & 0x02)) || ((tmpD & 0x01) && (tmpD & 0x02)))
					P1Movement = p1_move;
				else if(tmpD & 0x01)
					P1Movement = p1_up;
				else if(tmpD & 0x02)
					P1Movement = p1_down;
			}
			else
				P1Movement = p1_wait;
			break;
		case p1_up:
			P1Movement = p1_stop;
			break;
		case p1_down:
			P1Movement = p1_stop;
			break;
		case p1_stop:
			if(counterp1 == 2500)
				P1Movement = p1_move;
			else
				P1Movement = p1_stop;
			break;
	}	
	switch(P1Movement)
	{
		case p1_wait:
			counterp1 = 0;
			break;
		case p1_move:
			counterp1 = 0;
			break;
		case p1_up:
			if(P1INDEX < 5)
				P1INDEX++;
			else
				P1INDEX = P1INDEX;
			break;
		case p1_down:
			if(P1INDEX > 0)
				P1INDEX--;
			else
				P1INDEX = P1INDEX;
			break;
		case p1_stop:
			counterp1++;
			break;
	}
}

enum P2_Movement {p2_wait, p2_move, p2_up, p2_down, p2_stop} P2Movement;
unsigned short counterp2 = 0;
void MoveP2()
{
	switch(P2Movement)
	{
		case p2_wait:
			if(!READY && !MULTIPLAYER)
				P2Movement = p2_wait;
			else if (READY && MULTIPLAYER && !SINGLEPLAYER)
				P2Movement = p2_move;
			else
				P2Movement = p2_wait;
			break;
		case p2_move:
			if(!READY)
				P2Movement = p2_wait;
			else if(READY && MULTIPLAYER)
			{
				if((!(tmpD & 0x04) && !(tmpD & 0x08)) || ((tmpD & 0x04) && (tmpD & 0x08)))
					P2Movement = p2_move;
				else if(tmpD & 0x04)
					P2Movement = p2_up;
				else if(tmpD & 0x08)
					P2Movement = p2_down;
			}
			break;
		case p2_up:
			P2Movement = p2_stop;
			break;
		case p2_down:
			P2Movement = p2_stop;
			break;
		case p2_stop:
			if(counterp2 == 2500)
				P2Movement = p2_move;
			else
				P2Movement = p2_stop;
			break;
	}
	switch(P2Movement)
	{
		case p2_wait:
			counterp2 = 0;
			break;
		case p2_move:
			counterp2 = 0;
			break;
		case p2_up:
			if(P2INDEX < 5)
				P2INDEX++;
			else
				P2INDEX = P2INDEX;
			break;
		case p2_down:
			if(P2INDEX > 0)
				P2INDEX--;
			else
				P2INDEX = P2INDEX;
			break;
		case p2_stop:
			counterp2++;
			break;
	}
}

enum P2Bot_Movement {p2bot_wait, p2bot_move, p2bot_up, p2bot_down, p2bot_stop} P2BotMovement;
unsigned short counterp2bot = 0;
void MoveP2bot()
{
	switch(P2BotMovement)
	{
		case p2bot_wait:
			if(!READY && !SINGLEPLAYER)
				P2BotMovement = p2bot_wait;
			else if (READY && SINGLEPLAYER && !MULTIPLAYER)
				P2BotMovement = p2bot_move;
			else
				P2BotMovement = p2bot_wait;
			break;
		case p2bot_move:
			if(!READY)
				P2BotMovement = p2bot_wait;
			else if(READY && SINGLEPLAYER)
			{
				if((BRindex < P2INDEX))
					P2BotMovement = p2bot_down;
				else if((BRindex > P2INDEX))
					P2BotMovement = p2bot_up;
				else
					P2BotMovement = p2bot_move;	
			}
			break;
		case p2bot_up:
			P2BotMovement = p2bot_stop;
			break;
		case p2bot_down:
			P2BotMovement = p2bot_stop;
			break;
		case p2bot_stop:
			if(counterp2bot == 3000)
				P2BotMovement = p2bot_move;
			else
				P2BotMovement = p2bot_stop;
			break;
	}
	switch(P2BotMovement)
	{
		case p2bot_wait:
			counterp2bot = 0;
			break;
		case p2bot_move:
			counterp2bot = 0;
			break;
		case p2bot_up:
			if(P2INDEX < 5)
				P2INDEX++;
			else
				P2INDEX = P2INDEX;
			break;
		case p2bot_down:
			if(P2INDEX > 0)
				P2INDEX--;
			else
				P2INDEX = P2INDEX;
			break;
		case p2bot_stop:
			counterp2bot++;
			break;
	}
}

enum BallMoves {wait_ball, start, start_wait, move_ball, stop_ball} BallMove;
unsigned short ballcounter = 0;
unsigned short startcounter = 0;
char hit = -1;
char wall = -1;
void BallPlay()
{
	switch(BallMove)
	{
		case wait_ball:
			if(!READY)
				BallMove = wait_ball;
			else if(READY)
				BallMove = start;
			break;
		case start:
			if(!READY)
				BallMove = wait_ball;
			else
				BallMove = start_wait;
			break;
		case start_wait:
			if(startcounter != 10000)
				BallMove = start_wait;
			else if(startcounter == 10000)
			{
				startcounter = 0;
				BallMove = move_ball;
			}	
			break;
		case move_ball:
			startcounter = 0;
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
				BallMove = stop_ball;
			break;
		case stop_ball:
			if(ballcounter != (4000))
				BallMove = stop_ball;
			else
			{
				ballcounter = 0;
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
		case wait_ball:
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
				P1INDEX = 3;
				P2INDEX = 3;
				P1BOOL = 0;
				P2BOOL = 0;
			}
			else if(P1BOOL)
			{
				hit = 1;
				wall = 0;
				BRindex = 4;
				BCindex = 2;
				P1INDEX = 3;
				P2INDEX = 3;
				P1SCORE++;
			}
			else if(P2BOOL)
			{
				hit = -1;
				wall = 0;
				BRindex = 4;
				BCindex = 5;
				P1INDEX = 3;
				P2INDEX = 3;
				P2SCORE++;
			}
			break;
		case start_wait:
			P1BOOL = 0;
			P2BOOL = 0;
			startcounter++;
			break;
		case move_ball:
			startcounter = 0;
			BRindex = BRindex + wall;
			BCindex = BCindex + hit;
			
			if(BRindex > 6 || BRindex < 1)
				wall = wall * -1;
			
			if(BCindex < 2)
			{	
				if(BRindex == 0)
				{
					if(P1INDEX == 0)
					{
						hit = 1;
						wall = 1;
					}
				}
				else if(BRindex == 1)
				{
					if(P1INDEX == 0)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 1)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 2)
				{
					if(P1INDEX == 0)
					{
						hit = 1;
						wall = 1;
					}
					else if(P1INDEX == 1)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 2)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 3)
				{
					if(P1INDEX == 1)
					{
						hit = 1;
						wall = 1;
					}
					else if(P1INDEX == 2)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 3)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 4)
				{
					if(P1INDEX == 2)
					{
						hit = 1;
						wall = 1;
					}
					else if(P1INDEX == 3)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 4)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 5)
				{
					if(P1INDEX == 3)
					{
						hit = 1;
						wall = 1;
					}
					else if(P1INDEX == 4)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 5)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 6)
				{
					if(P1INDEX == 4)
					{
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 5)
					{
						hit = 1;
						wall = -1;
					}
				}
				else if(BRindex == 7)
				{
					if(P1INDEX == 5)
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
					if(P2INDEX == 0)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 1)
				{
					if(P2INDEX == 0)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 1)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 2)
				{
					if(P2INDEX == 0)
					{
						hit = -1;
						wall = 1;
					}
					else if(P2INDEX == 1)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 2)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 3)
				{
					if(P2INDEX == 1)
					{
						hit = -1;
						wall = 1;
					}
					else if(P2INDEX == 2)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 3)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 4)
				{
					if(P2INDEX == 2)
					{
						hit = -1;
						wall = 1;
					}
					else if(P2INDEX == 3)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 4)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 5)
				{
					if(P2INDEX == 3)
					{
						hit = -1;
						wall = 1;
					}
					else if(P2INDEX == 4)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 5)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 6)
				{
					if(P2INDEX == 4)
					{
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 5)
					{
						hit = -1;
						wall = -1;
					}
				}
				else if(BRindex == 7)
				{
					if(P2INDEX == 5)
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
		case stop_ball:
			ballcounter++;
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
