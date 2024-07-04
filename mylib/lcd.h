/*
 * lcd.h
 *
 * Team 01 ENGG2800
 */

#pragma once

#define F_CPU 11059200L
#define MAX_BRIGHTNESS 255 // Max OCR1A value for timer for LCD back light.

#include <stdint.h>

// Initialises the LCD.
void lcdInit(void);

// Clears the LCD screen and displays the 'connected' icon if necessary.
void fillScreen(uint16_t colour, uint8_t connected);

// Displays text on the LCD.
void drawText(char* text, uint8_t x, uint8_t y);

// Draws the 'connected' symbol on the LCD.
void drawConnected(uint16_t colour);

// Sets the brightness of the back light of the LCD.
void setLcdBrightness(uint8_t brightnessLevel);