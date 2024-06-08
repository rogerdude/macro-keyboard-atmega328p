/*
 * keypad.c
 *
 * TEAM 01 ENGG2800
 */

#include "keypad.h"
#include <avr/io.h>
#include <stdlib.h>

/* keyPadInit()
 * ------------
 * Initialises the Pins and Ports for the keypad.
 */
void keyPadInit(void)
{
    // Set rows to inputs.
    DDRD &= ~(ROW_ONE | ROW_TWO | ROW_THREE);

    // Set columns to outputs.
    DDRC |= COL_ONE | COL_TWO | COL_THREE | COL_FOUR;
}

/* keyPress()
 * ----------
 * Gets the matrix location of the key pressed.
 *
 * key: a pointer to an array where the key location will be stored.
 *
 * Returns: a pointer to an array with 0th index as the column and 1st index
 *     as the row of the key pressed.
 */
uint8_t* keyPress(uint8_t* key)
{
    uint8_t row[ROWS] = { ROW_ONE, ROW_TWO, ROW_THREE };
    uint8_t col[COLS] = { COL_ONE, COL_TWO, COL_THREE, COL_FOUR };

    int num = 0;
    uint8_t keysPressed = 0;

    // Iterate through all columns of keypad.
    for (uint8_t i = 0; i < COLS; i++) {
        PORTC |= col[i]; // Set column high.

        // Check all rows for high input to see if any switch is pressed.
        for (uint8_t j = 0; j < ROWS; j++) {
            if (PIND & row[j]) {
                key[0] = i;
                key[1] = j;
                keysPressed++;
            }
        }

        PORTC &= ~(col[i]); // Set column low.
    }

    // Ensure only 1 key is pressed.
    if (keysPressed == 1) {
        return key;
    }

    // Return the invalid key press if 0 or more than 1 key is pressed.
    key[0] = IGNORE_PRESS;
    key[1] = IGNORE_PRESS;
    return key;
}

/* keyLocation()
 * -------------
 * Gets the matrix location of key pressed based on the key number provided.
 *
 * location: an array for where the location of key will be stored.
 * keyNum: the key number to find in the keypad matrix.
 *
 * Returns: a pointer to an array with matrix location of the key to search
 *     as [column, row].
 */
uint8_t* keyLocation(uint8_t location[2], uint8_t keyNum)
{
    uint8_t keyLocations[COLS][ROWS] = {
        { 1, 5, 9 },
        { 2, 6, 10 },
        { 3, 7, 11 },
        { 4, 8, 12 }
    };

    // Get the column and row position of the key based on keyNum.
    for (uint8_t col = 0; col < COLS; col++) {
        for (uint8_t row = 0; row < ROWS; row++) {
            if (keyNum == keyLocations[col][row]) {
                location[0] = col;
                location[1] = row;
                return location;
            }
        }
    }
    return location;
}
