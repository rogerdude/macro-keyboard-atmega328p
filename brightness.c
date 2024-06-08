/*
 * brightness.c
 *
 * TEAM 01 ENGG2800
 */

#include "brightness.h"
#include "16bitcolours.h"
#include "lcd.h"
#include "rgbled.h"
#include <avr/io.h>

/* initAutoBrightness()
 * --------------------
 * Initialises ADC to read ADC value from light sensor.
 */
void initAutoBrightness(void)
{
    // Set up ADC to AVCC reference.
    ADMUX |= (1 << REFS0);

    // Enable ADC and set clock divider to 128.
    ADCSRA |= (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

/* getAutoBrightnessLevel()
 * ------------------------
 * Reads the ADC value from light sensor and converts to a brightness level.
 *
 * Returns: a number between 0 and 9 (inclusive) indicating brightness level.
 */
uint8_t getAutoBrightnessLevel(void)
{
    // Start the ADC conversion.
    ADCSRA |= (1 << ADSC);

    // Wait until conversion finished.
    while (ADCSRA & (1 << ADSC)) { }

    uint16_t brightness = ADC;
    uint8_t brightnessLevel = 5; // Default is 5.

    // Determine brightness level based on ADC value.
    for (uint16_t i = 0; i < 10; i++) {
        if (brightness >= (i * 100) && brightness < ((i + 1) * 100)) {
            brightnessLevel = i;
            break;
        }
    }

    return brightnessLevel;
}

/* setBrightness()
 * ---------------
 * Sets the brightness level for LCD and also updates RGB LED colours with
 * updated colours and brightness
 *
 * brightnessLevel: the brightness level to set.
 */
void setBrightness(uint8_t brightnessLevel)
{
    setLcdBrightness(brightnessLevel);
    displayLedColours(brightnessLevel);
}
