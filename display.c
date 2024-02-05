/*
 * display.c
 *
 * Authors: Luke Kamols, Jarrod Bennett
 */ 

#include "display.h"
#include <stdio.h>
#include <avr/pgmspace.h>
#include "pixel_colour.h"
#include "ledmatrix.h"
#include "game.h"

// constant value used to display 'SNKLD' on launch
static const uint8_t snkld_display[MATRIX_NUM_COLUMNS] = 
		{117, 85, 93, 124, 64, 124, 125, 17, 109, 0, 124, 4, 4, 125, 69, 57};

void initialise_display(void) {
	// start by clearing the LED matrix
	ledmatrix_clear();

	// create an array with the background colour at every position
	PixelColour col_colours[MATRIX_NUM_ROWS];
	for (int row = 0; row < MATRIX_NUM_ROWS; row++) {
		col_colours[row] = MATRIX_COLOUR_EMPTY;
	}

	// then add the bounds on the left
	for (int x = 0; x < MATRIX_X_OFFSET; x++) {
		ledmatrix_update_column(x, col_colours);
	}

	// and add the bounds on the right
	for (int x = MATRIX_X_OFFSET + WIDTH; x < MATRIX_NUM_COLUMNS; x++) {
		ledmatrix_update_column(x, col_colours);
	}
	
	// create an array with the background colour at every position
	PixelColour row_colours[MATRIX_NUM_COLUMNS];
	for (int col = 0; col < MATRIX_NUM_COLUMNS; col++) {
		row_colours[col] = MATRIX_COLOUR_EMPTY;
	}

	// then add the bounds on the bottom
	for (int y = 0; y < MATRIX_Y_OFFSET; y++) {
		ledmatrix_update_row(y, row_colours);
	}

	// and add the bounds on the right
	for (int y = MATRIX_Y_OFFSET + HEIGHT; y < MATRIX_NUM_ROWS; y++) {
		ledmatrix_update_row(y, row_colours);
	}
}

void start_display(void) {
	PixelColour colour;
	MatrixColumn column_colour_data;
	uint8_t col_data;
		
	ledmatrix_clear(); // start by clearing the LED matrix
	for (uint8_t col = 0; col < MATRIX_NUM_COLUMNS; col++) {
		col_data = snkld_display[col];
		// using the LSB as the colour determining bit, 1 is red, 0 is green
		if (col_data & 0x01) {
			colour = COLOUR_RED;
		} else {
			colour = COLOUR_GREEN;
		}
		// go through the top 7 bits (not the bottom one as that was our colour bit)
		// and set any to be the correct colour
		for(uint8_t i=7; i>=1; i--) {
			// If the relevant font bit is set, we make this a coloured pixel, else blank
			if(col_data & 0x80) {
				column_colour_data[i] = colour;
			} else {
				column_colour_data[i] = 0;
			}
			col_data <<= 1;
		}
		column_colour_data[0] = 0;
		ledmatrix_update_column(col, column_colour_data);
	}
}

// Update the square colour to the display. The object passed can be the object
// type or an object instance (which additionally has an ID number if 
// applicable -see get_object_type in game.c/h)
void update_square_colour(uint8_t x, uint8_t y, uint8_t object) {
	// determine which colour corresponds to this object
	PixelColour colour;
	object = get_object_type(object);
	
	switch (object) {
		case EMPTY_SQUARE:
			colour = MATRIX_COLOUR_EMPTY;
			break;
		case START_POINT:	/* FALLTHROUGH */
		case FINISH_LINE:
			colour = MATRIX_COLOUR_START_END;
			break;
		case PLAYER_1:
			colour = MATRIX_COLOUR_P1;
			break;
		case PLAYER_2:
			colour = MATRIX_COLOUR_P2;
			break;
			
		// All snakes should be the same colour
		case SNAKE_START:	/* FALLTHROUGH */
		case SNAKE_END:		/* FALLTHROUGH */
		case SNAKE_MIDDLE:
			colour = MATRIX_COLOUR_SNAKE;
			break;
			
		// All ladders should be the same colour
		case LADDER_START:	/* FALLTHROUGH */
		case LADDER_END:	/* FALLTHROUGH */
		case LADDER_MIDDLE:
			colour = MATRIX_COLOUR_LADDER;
			break;
		
		// An invalid object
		default:
			colour = MATRIX_COLOUR_EMPTY;
			break;
	}

	// Update the pixel at the given location with this colour
	ledmatrix_update_pixel(y, WIDTH - 1 - x, colour);
}