/*
 * keypad.h
 *
 * TEAM 01 ENGG2800
 */

#pragma once

#include <stdint.h>

// Pins for keypad rows on PIN D
#define ROW_ONE (1 << 2)
#define ROW_TWO (1 << 3)
#define ROW_THREE (1 << 4)

// Pins for keypad columns on PORT C
#define COL_ONE (1 << 5)
#define COL_TWO (1 << 4)
#define COL_THREE (1 << 3)
#define COL_FOUR (1 << 2)

#define ROWS 3 // Total number of rows in keypad.
#define COLS 4 // Total number of columns in keypad.
#define IGNORE_PRESS 4 // Default row or columns for invalid key presses.

#define COL 0 // Index for column in array returned by keyPress().
#define ROW 1 // Index for row in array returned by keyPress().

// Initialises the Pins and Ports for the keypad.
void keyPadInit(void);

// Returns the matrix location of the key pressed.
uint8_t* keyPress(uint8_t* key);

// Returns the matrix location of key pressed based on the keyNum provided.
uint8_t* keyLocation(uint8_t location[2], uint8_t keyNum);