/*
 * project.c
 *
 * Main file
 *
 * Authors: Peter Sutton, Luke Kamols, Jarrod Bennett
 * Modified by <YOUR NAME HERE>
 */ 


#include <stdio.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "game.h"
#include "display.h"
#include "ledmatrix.h"
#include "buttons.h"
#include "serialio.h"
#include "terminalio.h"
#include "timer0.h"

// Function prototypes - these are defined below (after main()) in the order
// given here
void initialise_hardware(void);
void start_screen(void);
void new_game(void);
void play_game(void);
void handle_game_over(void);

int count;
int rolling;
int seven_seg_cc;
int num_turns;
int num_turns2;
int player_2_turn;
int board_number = 1;
int number_of_boards = 2;
int winner;
int time;
int ms = 9;
int timer;
int hard;
int time2;
int ms2 = 9;
int roll;

/////////////////////////////// main //////////////////////////////////
int main(void) {
	// Setup hardware and call backs. This will turn on 
	// interrupts.
	initialise_hardware();
	
	// Show the splash screen message. Returns when display
	// is complete.
	start_screen();
	
	// Loop forever and continuously play the game.
	while(1) {
		new_game();
		play_game();
		handle_game_over();
	}
}

void initialise_hardware(void) {
	ledmatrix_setup();
	init_button_interrupts();
	// Setup serial port for 19200 baud communication with no echo
	// of incoming characters
	init_serial_stdio(19200,0);
	
	init_timer0();
	
	// Turn on global interrupts
	sei();
}

void start_screen(void) {
	// Clear terminal screen and output a message
	clear_terminal();
	move_terminal_cursor(10,10);
	printf_P(PSTR("Snakes and Ladders"));
	move_terminal_cursor(10,12);
	printf_P(PSTR("CSSE2010/7201 A2 by Muhammad Sulaman Khan - s4751192"));
	move_terminal_cursor(10,14);
	printf_P(PSTR("Start game by pressing 1 (for 1 player) or 2	(for 2 player)"));
	move_terminal_cursor(10,16);
	printf_P(PSTR("Selected Board (press B to change): Board %d"), board_number);
	
	// Output the static start screen and wait for a push button 
	// to be pushed or a serial input of 's'
	
	start_display();
	
	// Wait until a button is pressed, or 's' is pressed on the terminal
	while(1) {
		// First check for if a 's' is pressed
		char serial_input = -1;
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
		}
		
		PORTC = 0x00;
		
		if (timer) {
			if (hard) {
				time = 45;
				time2 = 45;
			} else {
				time = 90;
				time = 90;
			}
		}
		
		// If the serial input is 's', then exit the start screen
		if (serial_input == 's' || serial_input == 'S' || serial_input == '1') {
			break;
		}
		
		if (serial_input == 'b' || serial_input == 'B') {
			if (board_number == number_of_boards) {
				board_number = 1;
			} else {
			board_number ++;	
			}
			move_terminal_cursor(52, 16);
			printf_P(PSTR("%d"), board_number);		
		}
		
		if (serial_input == 'e' || serial_input == 'E') {
			break;
		}
		
		if (serial_input == 'm' || serial_input == 'M') {
			timer = 1;
			time = 90;
			time2 = 90;
		}
		
		if (serial_input == 'h' || serial_input == 'H') {
			timer = 1;
			time = 45;
			time2 = 45;
			hard = 1;
		}
		
		if (serial_input == '2') {
			set_two_players(1);
			break;
		}
		
		// Next check for any button presses
		int8_t btn = button_pushed();
		if (btn != NO_BUTTON_PUSHED) {
			break;
		}
	}
}

void new_game(void) {
	// Clear the serial terminal
	clear_terminal();
	
	// Initialise the game and display
	initialise_game(board_number);
	
	// Clear a button push or serial input if any are waiting
	// (The cast to void means the return value is ignored.)
	(void)button_pushed();
	clear_serial_input_buffer();
}

void play_game(void) {
	
	uint32_t last_flash_time, current_time, current_time2, current_time3, current_time4, last_time;
	uint32_t last_second, last_ms, start_time, end_time;
	uint8_t btn; // The button pushed
	uint8_t seven_seg[10] = {63,6,91,79,102,109,125,7,127,111};
	uint8_t seven_seg_dice[10] = {63,6,91,79,102,109,125};
	int first = 1;
	int first2 = 1;
	int first3 = 1;
	roll = 0;
		
	DDRC = 0xFF;
	PORTC = 63;
	DDRD |= (1<<2);
	count = 0;
	
	last_flash_time = get_current_time();
	last_time = get_current_time();
	last_second = get_current_time();
	last_ms = get_current_time();
	
	// We play the game until it's over
	while(!is_game_over()) {
				
		// We need to check if any button has been pushed, this will be
		// NO_BUTTON_PUSHED if no button has been pushed
		btn = button_pushed();
		
		if (btn == BUTTON0_PUSHED) {
			// If button 0 is pushed, move the player 1 space forward
			move_player_n(1);
			last_flash_time = get_current_time();
		}
		
		if (btn == BUTTON1_PUSHED) {
			// If button 1 is pushed, move the player 2 space forward
			move_player_n(2);
			last_flash_time = get_current_time();
		}
		
		// Check if WASD is pressed and implement
		char serial_input = -1;
			if (serial_input_available()) {
				serial_input = fgetc(stdin);
			}
		if (serial_input == 'w' || serial_input == 'W' ) {
			move_player(0, 1);
			last_flash_time = get_current_time();
		}
		if (serial_input == 'a' || serial_input == 'A' ) {
			move_player(-1, 0);
			last_flash_time = get_current_time();
		}
		if (serial_input == 's' || serial_input == 'S' ) {
			move_player(0, -1);
			last_flash_time = get_current_time();
		}
		if (serial_input == 'd' || serial_input == 'D' ) {
			move_player(1, 0);
			last_flash_time = get_current_time();
		}
		
		if (serial_input == 'e' || serial_input == 'E') {
			timer = 0;
			clear_terminal();
		}
		
		if (serial_input == 'm' || serial_input == 'M') {
			time = 90;
			time2 = 90;
			timer = 1;
		}
		
		if (serial_input == 'h' || serial_input == 'H') {
			time = 45;
			time2 = 45;
			timer = 1;
		}
		
		// Update time for flash, dice and timer
		current_time = get_current_time();
		current_time2 = get_current_time();
		current_time3 = get_current_time();
		current_time4 = get_current_time();
		
		if (timer) {
			if (get_two_players() == 0){
				if (current_time3 >= last_second + 1000){
					time --;
					last_second = get_current_time();
					if (time >= 10) {
						if (first) {
							move_terminal_cursor(10,12);
							printf_P(PSTR("Time remaining: %d s"), time);
							first = 0;
						} else {
							move_terminal_cursor(26,12);
							clear_to_end_of_line();
							printf_P(PSTR("%d s"), time);
						}
					}
				}
				if (time < 10) {
					if (current_time4 >= last_ms + 100) {
						if (ms > 0){
							ms --;
							} else if (ms == 0) {
							ms = 9;
						}
						move_terminal_cursor(27,12);
						clear_to_end_of_line();
						printf_P(PSTR("%d . %d s"), time, ms);
						last_ms = get_current_time();
					}
					if (time <= 0) {
						winner = 1 - (get_player_2_turn()) + 1;
						handle_game_over();
					}
				}
			}
			
			if (get_two_players()) {
				if (first3) {
					move_terminal_cursor(10,12);
					printf("Player 1 time remaining: %d s", time);
					move_terminal_cursor(10,14);
					printf("Player 2 time remaining: %d s", time2);
					first3 = 0;
				}
				if ((current_time3 >= last_second + 1000)) {
					if (get_player_2_turn() == 0){
						time --;
						if (time >= 10) {
							if (first) {
								move_terminal_cursor(10,12);
								printf_P(PSTR("Player 1 time remaining: %d s"), time);
								first = 0;
								} else {
								move_terminal_cursor(35,12);
								clear_to_end_of_line();
								printf_P(PSTR("%d s"), time);
							}
						}
					} else if (get_player_2_turn()) {
						time2 --;
						if (time2 >= 10) {
							if (first2) {
								move_terminal_cursor(10,14);
								printf_P(PSTR("Player 2 time remaining: %d s"), time2);
								first = 0;
								} else {
								move_terminal_cursor(35,14);
								clear_to_end_of_line();
								printf_P(PSTR("%d s"), time);
							}
						}
					}

					last_second = get_current_time();
				}
				if ((time < 10) & !(get_player_2_turn())) {
					if (current_time4 >= last_ms + 100) {
						move_terminal_cursor(35,12);
						if (ms > 0){
							ms --;
							} else if (ms == 0) {
							ms = 9;
						}
						printf_P(PSTR("%d . %d s"), time, ms);
						last_ms = get_current_time();
					}
				}
				if ((time2 < 10) & (get_player_2_turn())) {
					if (current_time4 >= last_ms + 100) {
						move_terminal_cursor(35,14);
						if (ms2 > 0){
							ms2 --;
							} else if (ms2 == 0) {
							ms2 = 9;
						}
						printf_P(PSTR("%d . %d s"), time2, ms2);
						last_ms = get_current_time();
					}
				}
				if (time <= 0) {
					winner = 2;
					handle_game_over();
				}
				if (time2 <= 0) {
					winner = 1;
					handle_game_over();
				}
		}
				}
		
		if (current_time >= last_flash_time + 500) {
			// 500ms (0.5 second) has passed since the last time we
			// flashed the cursor, so flash the cursor
			flash_player_cursor();
			
			// Update the most recent time the cursor was flashed
			last_flash_time = current_time;
		}
		
		// Dice roll
		if (btn == BUTTON2_PUSHED || serial_input == 'R' || serial_input == 'r') {
			PORTD ^= (1<<2);
			if (rolling == 1) {
				move_player_n(count);
				if (get_player_2_turn() == 0) {
					if (num_turns == 9) {
						num_turns = 0;
						} else {
						num_turns ++;
					}
				} else if (get_player_2_turn()) {
					if (num_turns2 == 9) {
						num_turns2 = 0;
						} else {
						num_turns2 ++;
					}
				}
				
			}
			rolling = 1 ^ rolling;
		}
		
		if (rolling) {
			seven_seg_cc = 0;
			if (current_time2 >= last_time + 50) {
				if (count == 6) {
					count = 1;
				} else {
					count ++;
				}
				last_time = current_time2;
			}
			if (!roll) {
				move_terminal_cursor(10, 10);
				printf_P(PSTR("Dice value: %d"), count);
				roll = 1;
				} else {
				move_terminal_cursor(22, 10);
				printf_P(PSTR("%d"), count);
			}
		}	
		
		// Toggles CC pin
		if (seven_seg_cc == 0) {
			PORTC = seven_seg_dice[count];
			PORTC |= (seven_seg_cc<<7);
			seven_seg_cc = 1 ^ seven_seg_cc;
			_delay_ms(5);
		}
		
		if (seven_seg_cc == 1) {
			PORTC =seven_seg[num_turns];
			PORTC |= (seven_seg_cc<<7);
			seven_seg_cc = 1 ^ seven_seg_cc;
			_delay_ms(5);
		}
		
		// Game Pause
		if (btn == BUTTON3_PUSHED || serial_input == 'p' || serial_input == 'P'){
			start_time = get_current_time();
			// Keep the cc toggling going
			while (serial_input != 'p' || serial_input != 'P') {
				if (seven_seg_cc == 0) {
					PORTC = seven_seg_dice[count];
					PORTC |= (seven_seg_cc<<7);
					seven_seg_cc = 1 ^ seven_seg_cc;
					_delay_ms(5);
				}
				
				if (seven_seg_cc == 1) {
					PORTC =seven_seg[num_turns];
					PORTC |= (seven_seg_cc<<7);
					seven_seg_cc = 1 ^ seven_seg_cc;
					_delay_ms(5);
				}
				
				char serial_input = -1;
				if (serial_input_available()) {
					serial_input = fgetc(stdin);
				}
				if ((serial_input == 'p' || serial_input == 'P') & (start_time != get_current_time())) {
					break;
				}
			}
			// After while loop is over, update times
			end_time = get_current_time();
			last_flash_time = last_flash_time + (start_time - end_time);
		}
		// Check if game is over
		if (is_game_over()) {
			winner = (!get_player_2_turn()) + 1;
			handle_game_over();
		}
	}	
	// We get here if the game is over.
}

void handle_game_over(void) {
	end_game();
	
	// Terminal stuff
	clear_terminal();
	if (get_two_players()) {
		move_terminal_cursor(10,12);
		printf_P(PSTR("PLAYER %d WINS!!!"), winner);
		move_terminal_cursor(10,14);
		printf_P(PSTR("Press a button to start again"));
	} else {
		move_terminal_cursor(10,12);
		printf_P(PSTR("GAME OVER"));
		move_terminal_cursor(10,14);
		printf_P(PSTR("Press a button to start again"));
	}
	
	PORTC = 0x00;
	
	while (button_pushed() == NO_BUTTON_PUSHED) {
		char serial_input = -1;
		if (serial_input_available()) {
			serial_input = fgetc(stdin);
		}
		if (serial_input == 's' || serial_input == 'S' || button_pushed() != NO_BUTTON_PUSHED) {
			main();
		}
	}
	main();
}
