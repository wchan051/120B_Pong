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
unsigned char START = 0;
unsigned char p1score = 0; 
unsigned char p2score = 0; 
unsigned char p1bool = 0; 
unsigned char p2bool = 0; 

enum Game_setup {wait_ready, wait, play, wait_score, writeP1, writeP2, winP1, winP2} Game;
unsigned char tmpD;

unsigned char screenStablizer = 0;

void ReadyScreen()
{
	switch(Game)
	{
		case wait_ready:
			if((tmpD & 0x1F) == 0x00) {
				START = 0;
				p1score = 0;
				p2score = 0;
				Game = wait_ready;
			}
			else if((tmpD & 0x1F) == 0x01) {
				START = 1;
				Game = wait;
			}
			break;
		case wait:
			if((tmpD & 0x1F) != 0) {
				if((tmpD & 0x1F) == 0x01) {
					START = 1;
				}
				Game= wait;
			}
			else
				Game = play;
			break;
		case play:
			Game = wait_score;
			break;
		case wait_score:
			if(!p1bool && !p2bool) {
				Game = wait_score;
			}
			else if(p1bool) {
				Game = writeP1;
			}
			else if(p2bool) {
				Game = writeP2;
			}
			break;
		case writeP1:
			if(p1score == 5) {
				Game = winP2;
			}
			else {
				Game = wait_score;
			}
			break;
		case writeP2:
			if(p2score == 5) {
				Game = winP1;
			}
			else {
				Game = wait_score;
			}
			break;
		case winP1:
			Game = wait_ready;
			break;
		case winP2:
			Game = wait_ready;
			break;
	}
	switch(Game)
	{
		case wait_ready:
			READY = 0;
			if(screenStablizer == 0) {
				LCD_ClearScreen();
				LCD_DisplayString(1, "Right Button to Start Pong PvP");
				screenStablizer = 1;
			}
			break;
		case wait:
			screenStablizer = 0;
			break;
		case play:
			READY = 1;
			if(screenStablizer == 0) {
				LCD_DisplayString(1, "Score:          P1)        P2)");
				screenStablizer = 1;
			}
			break;
		case wait_score:
			break;
		case writeP1:
			LCD_Cursor(31);
			LCD_WriteData(p1score + '0');
			break;
		case writeP2:
			LCD_Cursor(20);
			LCD_WriteData(p2score + '0');
			break;
		case winP1:
			START = 0;
			READY = 0;
			LCD_DisplayString(1, "    P1 WINS!");
			break;
		case winP2:
			START = 0;
			READY = 0;
			LCD_DisplayString(1, "    P2 WINS!");
			break;
	}	
}

unsigned char p1_row_movement[6] = {0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0};
unsigned char P1INDEX = 4;
unsigned char P1COL = 0xFE;
unsigned char p2_row_movement[6] = {0x07, 0x0E, 0x1C, 0x38, 0x70, 0xE0};
unsigned char P2INDEX = 4;
unsigned char P2COL = 0x7F;
unsigned char ballrow_movement[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char ballcol_movement[8] = {0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F};
unsigned char ball_row_index = 3;
unsigned char ball_col_index = 3;

enum Update {waitReadyP1, waitReadyP2, p1Update, p2Update, ballUpdate} LED;
unsigned char row_movement = 3;
void LED_Movement()
{
	switch(LED)
	{
		case waitReadyP1:
			if(!READY) {
				LED = waitReadyP1;
			}
			else if(READY)
				LED = p1Update;
			break;
		case waitReadyP2:
			if(!READY) {
				LED = waitReadyP2;
			}
			else if(READY)
				LED = p1Update;
			break;
		case p1Update:
			if(!READY) {
				LED = waitReadyP1;
			}
			else if(READY) {
				LED = p2Update;
			}
			break;
		case p2Update:
			if(!READY) {
				LED = waitReadyP1;
			}
			else if(READY) {
				LED = ballUpdate;
			}
			break;
		case ballUpdate:
			if(!READY) {
				LED = waitReadyP1;
			}
			else if(READY) {
				LED = p1Update;
			}
			break;
	}
	switch(LED)
	{
		case waitReadyP1:
			countermatrix++;
			PORTB = p1_row_movement[row_movement];
			PORTA = 0x7E;
			P1COL = 0xFE;
			P2COL = 0x7F;
			break;
		case waitReadyP2:
			countermatrix++;
			PORTB = p1_row_movement[row_movement];
			PORTA = 0x7E;
			P1COL = 0xFE;
			P2COL = 0x7F;
		break;
		case p1Update:
			PORTB = p1_row_movement[P1INDEX];
			PORTA = P1COL;
			break;
		case p2Update:
			PORTB = p2_row_movement[P2INDEX];
			PORTA = P2COL;
			break;
		case ballUpdate:
			PORTB = ballrow_movement[ball_row_index];
			PORTA = ballcol_movement[ball_col_index];
			break;	
	}
}

enum P1_Movement {p1_wait, p1_move, p1_up, p1_down, p1_stop} P1Movement;
unsigned short counterp1 = 0;
void MoveP1()
{
	switch(P1Movement)
	{
		case p1_wait:
			if(!READY) {
				P1Movement = p1_wait;
			}
			else if (READY)
				P1Movement = p1_move;
			break;
		case p1_move:
			if(READY) {
				if((!(tmpD & 0x01) && !(tmpD & 0x02)) || ((tmpD & 0x01) && (tmpD & 0x02))) {
					P1Movement = p1_move;
				}
				else if(tmpD & 0x01) {
					P1Movement = p1_up;
				}
				else if(tmpD & 0x02) {
					P1Movement = p1_down;
				}
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
			if(counterp1 == 2500) {
				P1Movement = p1_move;
			}
			else {
				P1Movement = p1_stop;
			}
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
			if(P1INDEX < 5) {
				P1INDEX++;
			}
			break;
		case p1_down:
			if(P1INDEX > 0) {
				P1INDEX--;
			}
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
			if(!READY && !START) {
				P2Movement = p2_wait;
			}
			else if (READY && START) {
				P2Movement = p2_move;
			}
			else {
				P2Movement = p2_wait;
			}
			break;
		case p2_move:
			if(!READY) {
				P2Movement = p2_wait;
			}
			else if(READY && START) {
				if((!(tmpD & 0x04) && !(tmpD & 0x08)) || ((tmpD & 0x04) && (tmpD & 0x08))) {
					P2Movement = p2_move;
				}
				else if(tmpD & 0x04) {
					P2Movement = p2_up;
				}
				else if(tmpD & 0x08) {
					P2Movement = p2_down;
				}
			}
			break;
		case p2_up:
			P2Movement = p2_stop;
			break;
		case p2_down:
			P2Movement = p2_stop;
			break;
		case p2_stop:
			if(counterp2 == 2500) {
				P2Movement = p2_move;
			}
			else {
				P2Movement = p2_stop;
			}
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
			if(P2INDEX < 5) {
				P2INDEX++;
			}
			break;
		case p2_down:
			if(P2INDEX > 0) {
				P2INDEX--; 
			}
			break;
		case p2_stop:
			counterp2++;
			break;
	}
}

enum BallMovement {wait_ball, start, start_wait, move_ball, stop_ball} Ball;
unsigned short ballcounter = 0;
unsigned short startcounter = 0;
char hit = -1;
char wall = -1;
void MoveBall()
{
	switch(Ball)
	{
		case wait_ball:
			if(!READY) {
				Ball = wait_ball;
			}
			else if(READY) {
				Ball = start;
			}
			break;
		case start:
			if(!READY) {
				Ball = wait_ball;
			}
			else { 
				Ball = start_wait; 
			}
			break;
		case start_wait:
			if(startcounter != 10000) {
				Ball = start_wait;
			}
			else if(startcounter == 10000) {
				startcounter = 0;
				Ball = move_ball;
			}	
			break;
		case move_ball:
			startcounter = 0;
			if(ball_col_index == 0) {
				p2bool = 1;
				Ball = start;
			}
			else if(ball_col_index == 7) {
				p1bool = 1;
				Ball = start;
			}
			else {
				Ball = stop_ball;
			}
			break;
		case stop_ball:
			if(ballcounter != (4000)) {
				Ball = stop_ball;
			}
			else {
				ballcounter = 0;
				if(p1bool)
					Ball = start;
				else if(p2bool)
					Ball = start;
				else
					Ball = move_ball;
			}
			break;
	}	
	switch(Ball) {
		case wait_ball:
			p1bool = 0;
			p2bool = 0;
			break;
		case start:
			if(!p1bool && !p2bool) {
				hit = -1;
				wall = 0;
				ball_row_index = 4;
				ball_col_index = 4;
				P1INDEX = 3;
				P2INDEX = 3;
				p1bool = 0;
				p2bool = 0;
			}
			else if(p1bool) {
				hit = 1;
				wall = 0;
				ball_row_index = 4;
				ball_col_index = 2;
				P1INDEX = 3;
				P2INDEX = 3;
				p1score++;
			}
			else if(p2bool) {
				hit = -1;
				wall = 0;
				ball_row_index = 4;
				ball_col_index = 5;
				P1INDEX = 3;
				P2INDEX = 3;
				p2score++;
			}
			break;
		case start_wait:
			p1bool = 0;
			p2bool = 0;
			startcounter++;
			break;
		case move_ball:
			startcounter = 0;
			ball_row_index = ball_row_index + wall;
			ball_col_index = ball_col_index + hit;
			
			if(ball_row_index > 6 || ball_row_index < 1) {
				wall = wall * -1;
			}
			if(ball_col_index < 2) {	
				if(ball_row_index == 0) {
					if(P1INDEX == 0) {
						hit = 1;
						wall = 1;
					}
				}
				else if(ball_row_index == 1) {
					if(P1INDEX == 0) {
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 1) {
						hit = 1;
						wall = -1;
					}
				}
				else if(ball_row_index == 2) {
					if(P1INDEX == 0) {
						hit = 1;
						wall = 1;
					}
					else if(P1INDEX == 1) {
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 2) {
						hit = 1;
						wall = -1;
					}
				}
				else if(ball_row_index == 3) {
					if(P1INDEX == 1) {
						hit = 1;
						wall = 1;
					}
					else if(P1INDEX == 2) {
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 3) {
						hit = 1;
						wall = -1;
					}
				}
				else if(ball_row_index == 4) {
					if(P1INDEX == 2) {
						hit = 1;
						wall = 1;
					}
					else if(P1INDEX == 3) {
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 4) {
						hit = 1;
						wall = -1;
					}
				}
				else if(ball_row_index == 5) {
					if(P1INDEX == 3) {
						hit = 1;
						wall = 1;
					}
					else if(P1INDEX == 4) {
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 5) {
						hit = 1;
						wall = -1;
					}
				}
				else if(ball_row_index == 6) {
					if(P1INDEX == 4) {
						hit = 1;
						wall = 0;
					}
					else if(P1INDEX == 5) {
						hit = 1;
						wall = -1;
					}
				}
				else if(ball_row_index == 7) {
					if(P1INDEX == 5) {
						hit = 1;
						wall = -1;
					}
				}
				else {
					p2bool = 1;
					
				}
			}
			
			
			else if(ball_col_index > 5)  {
				if(ball_row_index == 0) {
					if(P2INDEX == 0) {
						hit = -1;
						wall = -1;
					}
				}
				else if(ball_row_index == 1) {
					if(P2INDEX == 0) {
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 1) {
						hit = -1;
						wall = -1;
					}
				}
				else if(ball_row_index == 2) {
					if(P2INDEX == 0) {
						hit = -1;
						wall = 1;
					}
					else if(P2INDEX == 1) {
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 2) {
						hit = -1;
						wall = -1;
					}
				}
				else if(ball_row_index == 3) {
					if(P2INDEX == 1) {
						hit = -1;
						wall = 1;
					}
					else if(P2INDEX == 2) {
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 3) {
						hit = -1;
						wall = -1;
					}
				}
				else if(ball_row_index == 4) {
					if(P2INDEX == 2) {
						hit = -1;
						wall = 1;
					}
					else if(P2INDEX == 3) {
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 4) {
						hit = -1;
						wall = -1;
					}
				}
				else if(ball_row_index == 5) {
					if(P2INDEX == 3) {
						hit = -1;
						wall = 1;
					}
					else if(P2INDEX == 4) {
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 5) {
						hit = -1;
						wall = -1;
					}
				}
				else if(ball_row_index == 6) {
					if(P2INDEX == 4) {
						hit = -1;
						wall = 0;
					}
					else if(P2INDEX == 5) {
						hit = -1;
						wall = -1;
					}
				}
				else if(ball_row_index == 7) {
					if(P2INDEX == 5) {
						hit = -1;
						wall = -1;
					}
				}
				else {
					p1bool = 1;
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
	
    while(1)
    {
	tmpD = PIND;
	tmpD = ~tmpD;
	ReadyScreen();
	LED_Movement();
	MoveP1();
	MoveP2();
	MoveBall();
    }
}
