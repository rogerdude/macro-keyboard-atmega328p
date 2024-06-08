/*
 * Macrolyze
 *
 * Team 01 ENGG2800
 */

#include "macrolyze.h"
#include "16bitcolours.h"
#include "addresses.h"
#include "brightness.h"
#include "keypad.h"
#include "lcd.h"
#include "macros.h"
#include "memory.h"
#include "rgbled.h"
#include "timer.h"
#include "usart.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

// Global variable for brightness level and autoBrightnessMode for easy
// access to interrupt.
uint8_t brightnessLevel;
uint8_t autoBrightnessMode;

// Global variable for interrupt to specify the transfer mode.
uint8_t transferMode;

// Global variable array for interrupt to store all received configuration
// data.
uint8_t data[RECEIVE_BUFFER];
uint16_t counter; // Stores index of last byte in data array.

int main(void)
{
    transferMode = 0;
    counter = 0;

    // Initialise USART.
    usartInit(UBRR);

    // Initialise LCD.
    lcdInit();
    uint8_t connected = 0;
    fillScreen(BLACK, connected);

    // Initialise LEDS.
    ledInit();
    uint8_t colourChanged = 0;
    
    // Initialise Timer 0 and 1.
    initTimerZero();
    initTimerOne();

    // Initialise initial brightness level.
    initAutoBrightness();
    brightnessLevel = 5;
    autoBrightnessMode = 0;
    brightnessLevel = eepromRead((uint16_t)BRIGHTNESS_ADDRESS);
    autoBrightnessMode = eepromRead((uint16_t)AUTOBRIGHT_ADDRESS);

    // Initialise variables to determine when to update LCD with brightness.
    uint8_t initialBrightnessLevel = brightnessLevel;
    uint8_t displayBrightness = 0;
    uint8_t brightnessKeyPressed = 0;
    uint32_t lastUpdateTime;

    // Initialise Keypad.
    keyPadInit();

    // Initialise SPI for macros.
    macrosInit();
    uint8_t previewMode = 0;

    // Initialise variable for storing current keypress.
    uint8_t* keyPressed;
    uint8_t matrixLocation[2];
    uint8_t initKeyCol = IGNORE_PRESS;
    uint8_t initKeyRow = IGNORE_PRESS;

    // Initialise variables for 'initial repeat delay'.
    uint8_t initialPress = 0;
    uint32_t initialPressTime = 0;
    uint16_t initialRepeatDelay = 0;
    initialRepeatDelay = (eepromRead((uint16_t)INIT_REPEAT_DELAY_ADDRESS) << 8)
        | eepromRead((uint16_t)INIT_REPEAT_DELAY_ADDRESS + 1);

    // Initialise variables for 'repeat rate'.
    uint8_t repeatPress = 0;
    uint32_t repeatPressTime = 0;
    uint16_t repeatPressDelay = 0;
    repeatPressDelay = (eepromRead((uint16_t)REPEAT_RATE_ADDRESS) << 8)
        | eepromRead((uint16_t)REPEAT_RATE_ADDRESS + 1);

    uint8_t ledBlinkOff = 0;
    uint32_t ledOffTime = 0;

    uint8_t ledBlinkOn = 0;
    uint32_t ledOnTime = 0;
    uint8_t blinkOnce = 0;

    // Enable global interrupts.
    sei();

    // Display team number and course code for 2 seconds.
    drawText("Team 01", 45, 45);
    drawText("ENGG2800", 39, 65);
    lastUpdateTime = getCurrentTime();
    while (1) {
        if (getCurrentTime() >= lastUpdateTime + START_SCREEN_DELAY) {
            fillScreen(BLACK, connected);
            lastUpdateTime = getCurrentTime();
            break;
        }
    }

    // Read macro data from EEPROM.
    getMacroData();
    if (!autoBrightnessMode)
        setBrightness(brightnessLevel);

    while (1) {
        if (transferMode == RECEIVE_MODE) {
            // Wait 1 second for transfer to complete.
            uint32_t startTransferTime = getCurrentTime();
            while (getCurrentTime() < startTransferTime + RECEIVE_DELAY) { }

            cli();

            // Decode data to get macro data and update counter to
            // decode other configuration data.
            counter = receiveMacroData(data);

            // Get initial repeat delay.
            counter++;
            uint8_t high = data[counter++];
            uint8_t low = data[counter++];
            initialRepeatDelay = (high << 8) | low;

            // Get repeat rate.
            counter++;
            high = data[counter++];
            low = data[counter++];
            repeatPressDelay = (high << 8) | low;

            // Get auto brightness mode.
            counter++;
            autoBrightnessMode = data[counter++];

            // Get brightness level.
            counter++;
            brightnessLevel = data[counter++];

            storeAllData(initialRepeatDelay, repeatPressDelay);

            if (!autoBrightnessMode)
                setBrightness(brightnessLevel);

            counter = 0;
            transferMode = 0;
            initKeyCol = IGNORE_PRESS;
            initKeyRow = IGNORE_PRESS;
            sei();
        }

        // Send macro data to GUI.
        if (transferMode == SEND_MODE) {
            cli();
            sendMacroData();
            transferMode = 0;
            sei();
        }

        if (transferMode == SOFTWARE_CONNECTED) {
            connected = 1;
            drawConnected(WHITE);
            transferMode = 0;
        }

        if (transferMode == SOFTWARE_DISCONNECTED) {
            connected = 0;
            drawConnected(BLACK);
            transferMode = 0;
        }

        // Send repeat rate to GUI.
        if (transferMode == SEND_REPEAT_RATE) {
            cli();
            usartTransmit('R');
            uint8_t highRepeat = repeatPressDelay >> 8;
            uint8_t lowRepeat = repeatPressDelay;
            usartTransmit(highRepeat);
            usartTransmit(lowRepeat);
            sei();
            transferMode = 0;
        }

        // Send initial repeat delay to GUI.
        if (transferMode == SEND_INITIAL_REPEAT_DELAY) {
            // Send initial repeat delay.
            cli();
            usartTransmit('D');
            uint8_t highDelay = initialRepeatDelay >> 8;
            uint8_t lowDelay = initialRepeatDelay;
            usartTransmit(highDelay);
            usartTransmit(lowDelay);
            sei();
            transferMode = 0;
        }

        // Check if any key is pressed.
        keyPressed = keyPress(matrixLocation);

        // Bring device and LED back to default state if no key is pressed.
        if (keyPressed[COL] == IGNORE_PRESS) {
            initialPress = 0;
            repeatPress = 0;
            brightnessKeyPressed = 0;
            for (uint8_t col = 0; col < COLS; col++) {
                for (uint8_t row = 0; row < ROWS; row++)
                    turnOnLed(col, row);
            }
        }

        // Check if key is pressed.
        if (keyPressed[COL] != IGNORE_PRESS && !initialPress) {
            // Check if brightness key was pressed.
            if (keyPressed[COL] == 2 && keyPressed[ROW] == 2) {
                if (autoBrightnessMode) {
                    autoBrightnessMode = 0;
                    brightnessLevel = 0;
                    eepromWrite((uint16_t)AUTOBRIGHT_ADDRESS, autoBrightnessMode);
                    eepromWrite((uint16_t)BRIGHTNESS_ADDRESS, brightnessLevel);
                } else if (brightnessLevel == 9) {
                    autoBrightnessMode = 1;
                    if (displayBrightness) {
                        fillScreen(BLACK, connected);
                        displayBrightness = 0;
                    }
                    eepromWrite((uint16_t)AUTOBRIGHT_ADDRESS, autoBrightnessMode);
                } else {
                    brightnessLevel++;
                    eepromWrite((uint16_t)BRIGHTNESS_ADDRESS, brightnessLevel);
                }
                brightnessKeyPressed = 1;
            }

            // Check if preview mode key was pressed.
            if (keyPressed[COL] == 3 && keyPressed[ROW] == 2) {
                if (previewMode) {
                    previewMode = 0;
                    setMacroNumActions(3, 2, 0);
                    colourChanged = 1;
                } else {
                    previewMode = 1;
                    setMacroNumActions(3, 2, 1);
                    colourChanged = 1;
                }
            }

            if (!previewMode && !brightnessKeyPressed) {
                executeMacro(keyPressed[COL], keyPressed[ROW]);
                ledBlinkOff = 1;
                ledOffTime = getCurrentTime();
            }

            initialPress = 1;
            initialPressTime = getCurrentTime();

            // Check if the macro name displayed on LCD was same as previous.
            if (initKeyCol != keyPressed[COL] || initKeyRow != keyPressed[ROW]) {
                blinkOnce = 1;
                initKeyCol = keyPressed[COL];
                initKeyRow = keyPressed[ROW];
            }
        }

        // Check if key is held for 'initial repeat delay'.
        if (keyPressed[COL] != IGNORE_PRESS && initialPress && !repeatPress) {
            if (getCurrentTime() >= initialPressTime + initialRepeatDelay) {
                if (!previewMode && !brightnessKeyPressed) {
                    executeMacro(keyPressed[COL], keyPressed[ROW]);
                    ledBlinkOff = 1;
                    ledOffTime = getCurrentTime();
                }
                repeatPress = 1;
                repeatPressTime = getCurrentTime();
            }
        }

        // Check if key is held for 'repeat rate'.
        if (keyPressed[COL] != IGNORE_PRESS && repeatPress) {
            if (getCurrentTime() >= repeatPressTime + repeatPressDelay) {
                if (!previewMode && !brightnessKeyPressed)
                    executeMacro(keyPressed[COL], keyPressed[ROW]);
                repeatPressTime = getCurrentTime();
            }
        }

        // Update colour and brightness.
        if (colourChanged) {
            setBrightness(brightnessLevel);
            colourChanged = 0;
        }

        // Blink LED every 50ms if key is pressed.
        if (ledBlinkOff) {
            turnOffLed(keyPressed[COL], keyPressed[ROW]);
            setBrightness(brightnessLevel);

            if (getCurrentTime() >= ledOffTime + LED_BLINK_DELAY) {
                turnOnLed(keyPressed[COL], keyPressed[ROW]);
                setBrightness(brightnessLevel);
                ledBlinkOff = 0;

                // Display macro name on LCD after LED is blinked to avoid
                // input delay to macro.
                if (blinkOnce) {
                    if (keyPressed[COL] != 3 || keyPressed[ROW] != 2) {
                        fillScreen(BLACK, connected);
                        displayMacroName(keyPressed[COL], keyPressed[ROW]);
                    }
                    blinkOnce = 0;
                }

                if (repeatPress)
                    ledBlinkOn = 1;
                ledOnTime = getCurrentTime();
            }
        }

        // Keep LED On for 50ms if key is in repeat mode.
        if (ledBlinkOn) {
            if (getCurrentTime() >= ledOnTime + repeatPressDelay) {
                ledBlinkOn = 0;
                if (repeatPress)
                    ledBlinkOff = 1;
                ledOffTime = getCurrentTime();
            }
        }

        // Display macro name if preview mode is on.
        if (previewMode) {
            if (blinkOnce) {
                if (keyPressed[COL] != 3 || keyPressed[ROW] != 2) {
                    fillScreen(BLACK, connected);
                    displayMacroName(keyPressed[COL], keyPressed[ROW]);
                }
                blinkOnce = 0;
            }
        }

        // Get brightness level from sensor every 200ms.
        if (autoBrightnessMode) {
            if (getCurrentTime() > lastUpdateTime + AUTO_BRIGHTNESS_DELAY) {
                brightnessLevel = getAutoBrightnessLevel();
                lastUpdateTime = getCurrentTime();
                colourChanged = 1;
            }
        }

        // Display brightness level on LCD display if brightness
        // has been changed.
        if ((brightnessLevel != initialBrightnessLevel)
            && !(autoBrightnessMode)) {
            if (!brightnessKeyPressed) {
                initKeyCol = IGNORE_PRESS;
                initKeyRow = IGNORE_PRESS;
            }
            setBrightness(brightnessLevel);
            fillScreen(BLACK, connected);
            drawText("Brightness: ", 10, 55);
            char buffer[NUMBER_BUFFER];
            sprintf(buffer, "%d", brightnessLevel);
            drawText(buffer, 135, 55);
            displayBrightness = 1;
            initialBrightnessLevel = brightnessLevel;
            lastUpdateTime = getCurrentTime();
        }

        // Display brightness level for 1 second.
        if (displayBrightness) {
            if (getCurrentTime() >= lastUpdateTime + DISPLAY_BRIGHTNESS_DELAY) {
                fillScreen(BLACK, connected);
                initialBrightnessLevel = brightnessLevel;
                displayBrightness = 0;
            }
        }
    }

    return 0;
}

/* storeAllData()
 * --------------
 * Stores all configuration data to EEPROM.
 *
 * initialRepeatDelay: the initial repeat delay value to store.
 * repeatPressDelay: the repeat rate value to store.
 */
void storeAllData(uint16_t initialRepeatDelay, uint16_t repeatPressDelay)
{
    eepromWrite((uint16_t)INIT_REPEAT_DELAY_ADDRESS, initialRepeatDelay >> 8);
    eepromWrite((uint16_t)INIT_REPEAT_DELAY_ADDRESS + 1, initialRepeatDelay);

    eepromWrite((uint16_t)REPEAT_RATE_ADDRESS, repeatPressDelay >> 8);
    eepromWrite((uint16_t)REPEAT_RATE_ADDRESS + 1, repeatPressDelay);

    eepromWrite((uint16_t)BRIGHTNESS_ADDRESS, brightnessLevel);

    eepromWrite((uint16_t)AUTOBRIGHT_ADDRESS, autoBrightnessMode);

    storeMacroData();
}

// Interrupt for receiving bytes through USART.
ISR(USART_RX_vect)
{
    uint8_t input;
    input = UDR0;

    // Store all bytes received into data array.
    if (transferMode == RECEIVE_MODE) {
        data[counter++] = input;
        return;
    }

    if (input == 'M') {
        // Initiate data transfer.
        transferMode = RECEIVE_MODE;
        data[counter++] = input;
        return;
    }

    if (input == 'm') {
        // Send all macro data through USART.
        transferMode = SEND_MODE;
        return;
    }

    if (input == 'C') {
        // Display 'connected' symbol
        transferMode = SOFTWARE_CONNECTED;
        return;
    }

    if (input == 'c') {
        // Remove 'connected' symbol
        transferMode = SOFTWARE_DISCONNECTED;
        return;
    }

    if (input == 'r') {
        transferMode = SEND_REPEAT_RATE;
        return;
    }

    if (input == 'i') {
        transferMode = SEND_INITIAL_REPEAT_DELAY;
        return;
    }

    if (input == 'b') {
        usartTransmit('B');
        usartTransmit(brightnessLevel);
        return;
    }

    if (input == 'a') {
        usartTransmit('A');
        if (autoBrightnessMode) {
            usartTransmit(1);
        } else {
            usartTransmit(0);
        }
        return;
    }
}
