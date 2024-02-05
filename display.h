/*
 * display.h
 *
 * Authors: Luke Kamols, Jarrod Bennett
 */ 


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include "pixel_colour.h"

// Offset for the LED matrix to cater for any game border offset to the edge
// of the LED matrix display.
#define MATRIX_X_OFFSET 0
#define MATRIX_Y_OFFSET 0

// Matrix colour definitions
#define MATRIX_COLOUR_EMPTY		COLOUR_BLACK
#define MATRIX_COLOUR_START_END	COLOUR_LIGHT_YELLOW
#define MATRIX_COLOUR_P1		COLOUR_ORANGE
#define MATRIX_COLOUR_P2		COLOUR_LIGHT_ORANGE
#define MATRIX_COLOUR_SNAKE		COLOUR_RED
#define MATRIX_COLOUR_LADDER	COLOUR_GREEN

// Initialise the display for the board, this creates the display
// for an empty board.
void initialise_display(void);

// Shows a starting display.
void start_display(void);

// Updates the colour at square (x, y) to be the colour
// of the object 'object'.
void update_square_colour(uint8_t x, uint8_t y, uint8_t object);


#endif /* DISPLAY_H_ */