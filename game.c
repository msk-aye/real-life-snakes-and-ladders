/*
 * game.c
 *
 * Functionality related to the game state and features.
 *
 * Author: Jarrod Bennett
 */ 


#include "game.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "display.h"
#include "terminalio.h"
#define F_CPU 8000000UL
#include <util/delay.h>

uint8_t board[WIDTH][HEIGHT];

// The initial game layout. Note that this is laid out in such a way that
// starting_layout[x][y] does not correspond to an (x,y) coordinate but is a
// better visual representation (but still somewhat messy).
// In our reference system, (0,0) is the bottom left, but (0,0) in this array
// is the top left.
static const uint8_t starting_layout[HEIGHT][WIDTH] =
{
	{FINISH_LINE, 0, 0, 0, 0, 0, 0, 0},
	{0, SNAKE_START | 4, 0, 0, LADDER_END | 4, 0, 0, 0},
	{0, SNAKE_MIDDLE, 0, LADDER_MIDDLE, 0, 0, 0, 0},
	{0, SNAKE_MIDDLE, LADDER_START | 4, 0, 0, 0, 0, 0},
	{0, SNAKE_END | 4, 0, 0, 0, 0, SNAKE_START | 3, 0},
	{0, 0, 0, 0, LADDER_END | 3, 0, SNAKE_MIDDLE, 0},
	{SNAKE_START | 2, 0, 0, 0, LADDER_MIDDLE, 0, SNAKE_MIDDLE, 0},
	{0, SNAKE_MIDDLE, 0, 0, LADDER_START | 3, 0, SNAKE_END | 3, 0},
	{0, 0, SNAKE_END | 2, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, SNAKE_START | 1, 0, 0, 0, LADDER_END | 1},
	{0, LADDER_END | 2, 0, SNAKE_MIDDLE, 0, 0, LADDER_MIDDLE, 0},
	{0, LADDER_MIDDLE, 0, SNAKE_MIDDLE, 0, LADDER_START | 1, 0, 0},
	{0, LADDER_START | 2, 0, SNAKE_MIDDLE, 0, 0, 0, 0},
	{START_POINT, 0, 0, SNAKE_END | 1, 0, 0, 0, 0}
};

static const uint8_t starting_layout_2[HEIGHT][WIDTH] =
{
	{FINISH_LINE, 0, 0, 0, 0, SNAKE_START | 4, 0, 0},
	{0, 0, 0, 0, 0, 0, SNAKE_MIDDLE, 0},
	{0, 0, LADDER_END | 4, 0, 0, 0, 0, SNAKE_MIDDLE},
	{0, 0, LADDER_MIDDLE, 0, 0, 0, 0, SNAKE_MIDDLE},
	{0, 0, LADDER_START | 4, 0, 0, 0, 0, SNAKE_END | 4},
	{0, 0, 0, SNAKE_START | 3, 0, 0, 0, 0},
	{LADDER_END | 2, 0, LADDER_END | 3, 0, SNAKE_MIDDLE, 0, 0, 0},
	{LADDER_START | 2, 0, 0, LADDER_MIDDLE, 0, SNAKE_END | 3, 0, 0},
	{0, 0, 0, 0, LADDER_MIDDLE, 0, 0, 0},
	{0, SNAKE_START | 2, 0, 0, 0, LADDER_MIDDLE, 0, 0},
	{0, SNAKE_MIDDLE, 0, 0, 0, LADDER_START | 3, 0, 0},
	{0, SNAKE_MIDDLE, 0, 0, 0, 0, 0, 0},
	{0, SNAKE_END | 2, 0, 0, 0, 0, SNAKE_START | 1, 0},
	{0, 0, 0, LADDER_END | 1, 0, 0, SNAKE_MIDDLE, 0},
	{0, 0, 0, 0, LADDER_MIDDLE, 0, 0, SNAKE_END | 1},
	{START_POINT, 0, 0, 0, 0, LADDER_START | 1, 0, 0},
};

static const uint8_t game_over[HEIGHT][WIDTH] =
{
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
	{0, 0, 0, LADDER_MIDDLE, LADDER_MIDDLE, 0, 0, 0},
	{0, 0, 0, LADDER_MIDDLE, LADDER_MIDDLE, 0, 0, 0},
	{0, 0, 0, LADDER_MIDDLE, LADDER_MIDDLE, 0, 0, 0},
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, 0, 0, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, 0, 0, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, 0, 0, 0, 0, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, 0, LADDER_MIDDLE, LADDER_MIDDLE, 0, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, 0, LADDER_MIDDLE, LADDER_MIDDLE, 0, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
	{LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE, LADDER_MIDDLE},
};

// The player is not stored in the board itself to avoid overwriting game
// elements when the player is moved.
int two_players;
int brek = 0;

int8_t player_x = 0;
int8_t player_y = 0;

int8_t player_1_x = 0;
int8_t player_1_y = 0;

int8_t player_2_x = 0;
int8_t player_2_y = 0;

int player_2_turn;

uint8_t colour = PLAYER_1;

// For flashing the player icon
uint8_t player_visible;

int get_two_players(void) {
	return two_players;
}

void set_two_players(int x) {
	two_players = x;
}

int get_player_2_turn(void) {
	return player_2_turn;
}

void initialise_game(int board_number) {
	
	// initialise the display we are using.
	initialise_display();
		
	// start the player icon at the bottom left of the display
	player_x = 0;
	player_y = 0;
	
	player_1_x = 0;
	player_1_y = 0;

	player_2_x = 0;
	player_2_y = 0;
	
	player_visible = 0;

	// Go through and initialise the state of the playing_field
	for (int x = 0; x < WIDTH; x++) {
		for (int y = 0; y < HEIGHT; y++) {
			// initialise this square based on the starting layout
			// the indices here are to ensure the starting layout
			// could be easily visualised when declared
			if (board_number == 1) {
				board[x][y] = starting_layout[HEIGHT - 1 - y][x];
				update_square_colour(x, y, get_object_type(board[x][y]));
			} else if (board_number == 2 ) {
				board[x][y] = starting_layout_2[HEIGHT - 1 - y][x];
				update_square_colour(x, y, get_object_type(board[x][y]));
			}
		}
	}
	
	update_square_colour(player_x, player_y, PLAYER_1);
}

// Return the game object at the specified position (x, y). This function does
// not consider the position of the player token since it is not stored on the
// game board.
uint8_t get_object_at(uint8_t x, uint8_t y) {
	// check the bounds, anything outside the bounds
	// will be considered empty
	if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) {
		return EMPTY_SQUARE;
	} else {
		//if in the bounds, just index into the array
		return board[x][y];
	}
}

// Extract the object type of a game element (the upper 4 bits).
uint8_t get_object_type(uint8_t object) {
	return object & 0xF0;
}

// Get the identifier of a game object (the lower 4 bits). Not all objects
// have an identifier, in which case 0 will be returned.
uint8_t get_object_identifier(uint8_t object) {
	return object & 0x0F;
}

// Function to get player position 
uint8_t get_player_x(void) {
	return player_x;
}

uint8_t get_player_y(void) {
	return player_y;
}

// Checks if player is on a snake or ladder and implements if needed
void check_snake_ladder(void) {
	brek =0;
	if (get_object_type(get_object_at(player_x, player_y)) == SNAKE_START) {
		int id = 0;
		id = get_object_identifier(get_object_at(player_x, player_y));
		/*for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				// Find end of snake
				if ((get_object_type(get_object_at(x, y)) == SNAKE_END) & (get_object_identifier(get_object_at(x, y)) == id)) {
					player_x = x;
					player_y = y;
				}
			}
		}*/
		for (int y = player_y; y >= 0; y--) {
			for (int x = -1; x <= 1; x++){
				if (get_object_type(get_object_at(player_x + x, y - 1)) == SNAKE_MIDDLE) {
					update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
					player_x += x;
					player_y --;
					update_square_colour(player_x, player_y, colour);
					_delay_ms(50);
				} else if ((get_object_type(get_object_at(player_x + x, y - 1)) == SNAKE_END) & (get_object_identifier(get_object_at(player_x + x, y-1)) == id)) {
					update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
					player_x += x;
					player_y --;
					update_square_colour(player_x, player_y, colour);
					brek = 1;
					break;
					}
			}
			if (brek) {
				break;
			}
			}
		}
	
	if (get_object_type(get_object_at(player_x, player_y)) == LADDER_START) {
		int id;
		id = get_object_identifier(get_object_at(player_x, player_y));
		/*for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				// Find end of ladder
				if ((get_object_type(get_object_at(x, y)) == LADDER_END) & (get_object_identifier(get_object_at(x, y)) == id)) {
					player_x = x;
					player_y = y;
				}
			}
		}*/
		for (int y = player_y; y <= 15; y++) {
			for (int x = -1; x <= 1; x++){
				if (get_object_type(get_object_at(player_x + x, player_y + 1)) == LADDER_MIDDLE) {
					update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
					player_x += x;
					player_y ++;
					update_square_colour(player_x, player_y, colour);
					_delay_ms(50);
					} else if ((get_object_type(get_object_at(player_x + x, player_y + 1)) == LADDER_END) && (get_object_identifier(get_object_at(player_x + x, player_y+1)) == id)) {
					update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
					player_x += x;
					player_y ++;
					update_square_colour(player_x, player_y, colour);
					brek = 1;
					break;
					}
			}
			if (brek) {
				break;
				}
			}
			}
		}

// Move the player by the given number of spaces forward.
void move_player_n(uint8_t num_spaces) {
	// If player 2 turn, variables player_x and player_y are loaded with player 2 position.
	if (two_players) {
		if (player_2_turn) {
			player_x = player_2_x;
			player_y = player_2_y;
			colour = PLAYER_2;
			update_square_colour(player_1_x, player_1_y, PLAYER_1);
		} else {
			player_x = player_1_x;
			player_y = player_1_y;
			colour = PLAYER_1;
			update_square_colour(player_2_x, player_2_y, PLAYER_2);
		}
	}
	
	// Updates the square color back to whatever was on the square
	update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
	
	// Changes player positional knowledge
	int count = 0;
	for (count = 0; count < num_spaces; count++) {
		if (player_y % 2 == 0) {
			if (player_x == 7) {
				update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
				player_y ++;
				update_square_colour(player_x, player_y, colour);
				_delay_ms(50);
				} else {
				update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
				player_x ++;
				update_square_colour(player_x, player_y, colour);
				_delay_ms(50);
			}
		} else if (player_y % 2) {
			if (player_x == 0) {
				update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
				player_y ++;
				update_square_colour(player_x, player_y, colour);
				_delay_ms(50);
				} else {
				update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
				player_x --;
				update_square_colour(player_x, player_y, colour);
				_delay_ms(50);
			}
		}
	}
	
	// Checks if on snake or ladder
	check_snake_ladder();
	is_game_over();
	
	// Update positions
	if (two_players) {
		if (player_2_turn) {
			player_2_x = player_x;
			player_2_y = player_y;
			} else {
			player_1_x = player_x;
			player_1_y = player_y;
		}
	}
	
	// Updates new square player is on
	update_square_colour(player_x, player_y, colour);
	
	// Toggles player turn
	if (two_players) {
		player_2_turn ^= 1;
	}
		
	// If player 2 turn, variables player_x and player_y are loaded with player 2 position.
	if (two_players) {
		if (player_2_turn) {
			player_x = player_2_x;
			player_y = player_2_y;
			colour = PLAYER_2;
			} else {
			player_x = player_1_x;
			player_y = player_1_y;
			colour = PLAYER_1;
		}
	}
}

// Move the player one space in the direction (dx, dy). The player should wrap
// around the display if moved 'off' the display.
void move_player(int8_t dx, int8_t dy) {
	// If player 2 turn, variables player_x and player_y are loaded with player 2 position.
	if (two_players) {
		if (player_2_turn) {
			player_x = player_2_x;
			player_y = player_2_y;
			colour = PLAYER_2;
			} else {
			player_x = player_1_x;
			player_y = player_1_y;
			colour = PLAYER_1;
		}
	}
	// Updates the square colour back to whatever was on the square
	update_square_colour(player_x, player_y, get_object_type(board[player_x][player_y]));
	
	// Changes player positional knowledge
	player_x += dx;
	player_y += dy;
	
	// Corrects if the player moved off board
	if (player_x > 7){
		player_x = 0;
	}
	if (player_x < 0){
		player_x = 7;
	}
	if (player_y > 15){
		player_y = 0;
	}
	if (player_y < 0l){
		player_y = 15;
	}
	
	// Checks if on snake or ladder
	check_snake_ladder();
	is_game_over();

	// Update positions
	if (two_players) {
		if (player_2_turn) {
			player_2_x = player_x;
			player_2_y = player_y;
			} else {
			player_1_x = player_x;
			player_1_y = player_y;
		}
	}
	
	// Updates new square player is on
	update_square_colour(player_x, player_y, colour);
	
	// Toggles player turn
	if (two_players) {
		player_2_turn ^= 1;
	}
	
	// If player 2 turn, variables player_x and player_y are loaded with player 2 position.
	if (two_players) {
		if (player_2_turn) {
			player_x = player_2_x;
			player_y = player_2_y;
			colour = PLAYER_2;
			} else {
			player_x = player_1_x;
			player_y = player_1_y;
			colour = PLAYER_1;
		}
	}
}

// Flash the player icon on and off. This should be called at a regular
// interval (see where this is called in project.c) to create a consistent
// 500 ms flash.
void flash_player_cursor(void) {
	if (player_2_turn) {
		colour = PLAYER_2;
	} else {
		colour = PLAYER_1;
	}
	if (player_visible) {
		// we need to flash the player off, it should be replaced by
		// the colour of the object which is at that location
		uint8_t object_at_cursor = get_object_at(player_x, player_y);
		update_square_colour(player_x, player_y, object_at_cursor);
			
		} else {
		// we need to flash the player on
		update_square_colour(player_x, player_y, colour);
	}
	player_visible = 1 - player_visible; //alternate between 0 and 1
}

// Returns 1 if the game is over, 0 otherwise.
uint8_t is_game_over(void) {
	// Detect if the game is over i.e. if a player has won.
	if (!two_players) {
		if (((player_x == 0) & (player_y == 15)) | (player_y > 15) | (player_x < 0)) {
			return 1;
		}
		return 0;
	}
	
	if (two_players) {
		if (((player_1_x == 0) & (player_1_y == 15)) | (player_1_y > 15) | (player_1_x < 0)) {
			return 1;
		} else if (((player_2_x == 0) & (player_2_y == 15)) | (player_2_y > 15) | (player_2_x < 0)) {
		return 1;
		}
		return 0;
	}
	return 0;
}

void end_game(void) {
		// Board animation
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				_delay_ms(20);
				update_square_colour(x, y, SNAKE_MIDDLE);
			}
		}
		
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				_delay_ms(20);
				board[x][y] = game_over[HEIGHT - 1 - y][x];
				update_square_colour(x, y, get_object_type(board[x][y]));
			}
		}
		
		_delay_ms(1000);
		for (int x = 0; x < WIDTH; x++) {
			for (int y = 0; y < HEIGHT; y++) {
				update_square_colour(x, y, EMPTY_SQUARE);
			}
		}
		
	}