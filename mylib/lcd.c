/*
 * lcd.c
 *
 * Team 01 ENGG2800
 */

#include "lcd.h"
#include "st7735.h"
#include <avr/io.h>
#include <string.h>
#include <util/delay.h>

// Initialise global variable for LCD struct to make it easier manipulate LCD
// from macrolyze.c.
struct signal Cs = { .ddr = &DDRB, .port = &PORTB, .pin = 2 }; // Chip Select.
struct signal Bl = { .ddr = &DDRB, .port = &PORTB, .pin = 1 }; // Back Light.
struct signal Dc = { .ddr = &DDRB, .port = &PORTB, .pin = 0 }; // D/C Pin.
struct signal Rs = { .ddr = &DDRD, .port = &PORTD, .pin = 7 }; // Reset.

// LCD struct.
struct st7735 Lcd = { .cs = &Cs, .bl = &Bl, .dc = &Dc, .rs = &Rs };

/* lcdInit()
 * ---------
 * Initialises the LCD.
 */
void lcdInit(void)
{
    // Initialise LCD.
    ST7735_Init(&Lcd);

    // Set LCD brightness to 5.
    OCR1A = 125;
}

/* fillScreen()
 * ------------
 * Clears the LCD screen and displays the 'connected' icon if necessary.
 *
 * colour: the colour to fill the screen with.
 * connected: a 1 or 0 to indicate whether connected icon should be displayed.
 */
void fillScreen(uint16_t colour, uint8_t connected)
{
    // Increase SPI CLK speed.
    SPCR &= ~(1 << SPR0);
    SPSR |= (1 << SPI2X);

    ST7735_ClearScreen(&Lcd, colour);
    if (connected) {
        drawConnected(WHITE);
    }

    // Revert SPI CLK speed back to normal.
    SPSR &= ~(1 << SPI2X);
    SPCR |= (1 << SPR0);
}

/* drawText()
 * ----------
 * Displays text on the LCD.
 *
 * text: the text to display.
 * x: the position in x-axis for where to display the text.
 * y: the position in y-axis for where to display the text.
 */
void drawText(char* text, uint8_t x, uint8_t y)
{
    // Increase SPI CLK speed so divider is 2.
    SPCR &= ~(1 << SPR0);
    SPSR |= (1 << SPI2X);

    // Get text length and check if its valid.
    uint8_t textLength = strlen(text);
    if (textLength > 30) {
        return;
    }

    // Determine size for text.
    uint8_t textSize = X1; // Default size is X1.
    if (textLength < 14) {
        textSize = X3;
    } else if (textLength < 25) {
        textSize = X2;
    }

    ST7735_SetPosition(x, y);
    ST7735_DrawString(&Lcd, text, WHITE, textSize);

    // Revert SPI CLK speed back to normal.
    SPSR &= ~(1 << SPI2X);
    SPCR |= (1 << SPR0);
}

/* drawConnected()
 * ---------------
 * Draws the 'connected' symbol on the LCD.
 *
 * colour: the 16 bit colour of the symbol.
 */
void drawConnected(uint16_t colour)
{
    // Draw bottom horizontal line.
    for (uint8_t x = 130; x < 145; x++) {
        ST7735_DrawPixel(&Lcd, x, 85, colour);
    }

    // Draw top horizontal line.
    for (uint8_t x = 130; x < 145; x++) {
        ST7735_DrawPixel(&Lcd, x, 101, colour);
    }

    // Draw left vertical line.
    for (uint8_t y = 86; y < 100; y++) {
        ST7735_DrawPixel(&Lcd, 130, y, colour);
    }
    // Draw right vertical line.
    for (uint8_t y = 86; y < 100; y++) {
        ST7735_DrawPixel(&Lcd, 145, y, colour);
    }

    // Draw a horizontal line on the left.
    for (uint8_t x = 120; x < 130; x++) {
        ST7735_DrawPixel(&Lcd, x, 93, colour);
    }

    // Draw two horizontal lines on the right.
    for (uint8_t x = 145; x < 154; x++) {
        ST7735_DrawPixel(&Lcd, x, 89, colour);
    }
    for (uint8_t x = 145; x < 154; x++) {
        ST7735_DrawPixel(&Lcd, x, 96, colour);
    }
}

/* setLcdBrightness()
 * ------------------
 * Sets the brightness of the back light of the LCD.
 *
 * brightnessLevel: the brightness level to set.
 */
void setLcdBrightness(uint8_t brightnessLevel)
{
    OCR1A = (MAX_BRIGHTNESS / 9) * brightnessLevel;
}