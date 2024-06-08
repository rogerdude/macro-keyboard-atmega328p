/*
 * led.c
 *
 * Team 01 ENGG2800
 */

#define F_CPU 11059200L

#include "rgbled.h"
#include "keypad.h"
#include "light_ws2812.h"
#include "macros.h"
#include "ws2812_config.h"
#include <util/delay.h>

// Global variable struct for all 10 LEDs to make it easier to set LED
// colours in the whole program.
struct cRGB led[COLS][ROWS];

/* ledInit()
 * ---------
 * Initialises the LEDs Port and sets all LEDs off.
 */
void ledInit(void)
{
    // Set LED pin to output.
    DDRD |= (1 << 6);

    // Set all colours to black (off).
    for (uint8_t row = 0; row < ROWS; row++) {
        for (uint8_t col = 0; col < COLS; col++) {
            setLedColour(col, row, 0, 0, 0);
        }
    }

    // Set colours of LEDS of auxiliary keys to white.
    setLedColour(2, 2, 255, 255, 255);
    setLedColour(3, 2, 255, 255, 255);
}

/* setLedColour()
 * --------------
 * Sets the colour of the LED in the specified matrix location.
 *
 * column: the column of LED to set.
 * row: the row of LED to set.
 * red: the amount of red in the colour.
 * green: the amount of green in the colour.
 * blue: the amount of blue in the colour.
 */
void setLedColour(uint8_t column, uint8_t row,
    uint8_t red, uint8_t green, uint8_t blue)
{
    led[column][row].r = red;
    led[column][row].g = green;
    led[column][row].b = blue;
}

/* displayLedColours()
 * -------------------
 * Updates colours in all LEDs based on brightness level.
 *
 * brightnessLevel: the brightness level to set.
 */
void displayLedColours(uint8_t brightnessLevel)
{
    // Iterate through every LED.
    for (uint8_t row = 0; row < ROWS; row++) {
        for (uint8_t col = 0; col < COLS; col++) {

            // Store original colours of LED.
            uint8_t origRed = led[col][row].r;
            uint8_t origGreen = led[col][row].g;
            uint8_t origBlue = led[col][row].b;

            // Send colours based on whether macro has any actions and
            // also based on brightness level.
            if (!getMacroNumActions(col, row)) {
                led[col][row].r = 0;
                led[col][row].g = 0;
                led[col][row].b = 0;
            } else {
                led[col][row].r = (origRed / 9) * brightnessLevel;
                led[col][row].g = (origGreen / 9) * brightnessLevel;
                led[col][row].b = (origBlue / 9) * brightnessLevel;
            }
            ws2812_sendarray((uint8_t*)&led[col][row], 3);

            // Revert colours back to original.
            led[col][row].r = origRed;
            led[col][row].g = origGreen;
            led[col][row].b = origBlue;
        }
    }
    _delay_us(80); // Delay required to update LEDS.
}
