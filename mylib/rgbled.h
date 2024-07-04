/*
 * led.h
 *
 * Team 01 ENGG2800
 */

#pragma once

#include <stdint.h>

// Initialises the LEDs Port and sets all LEDs off.
void ledInit(void);

// Sets the colour of the LED in the specified matrix location.
void setLedColour(uint8_t column, uint8_t row,
    uint8_t red, uint8_t green, uint8_t blue);

// Updates colours in all LEDs based on brightness level.
void displayLedColours(uint8_t brightnessLevel);