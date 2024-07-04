/*
 * macros.h
 *
 * TEAM 01 ENGG2800
 */

#pragma once

#define F_CPU 11059200L
#define MAX_ACTIONS 20 // Max number of actions for macro.
#define KEYS_PER_ACTION 8 // Number of bytes for HID report.
#define BYTES_PER_ACTION 2 // Number of bytes to store per macro action.
#define MAX_CHARACTERS 31 // Length of macro name including '\0' character.

#define MODIFIER (1 << 7) // Bit that indicates whether key is modifier.
#define PRESSED (1 << 6) // Bit that indicates whether key is pressed or not.
#define RELEASE_ALL_KEYS 0xFF // Action to indicate to send release all keys.
#define EMPTY_KEY 0x00

#define HID_DELAY 20

#include <stdint.h>

// Struct to store macro data for each macro key.
struct MacroData {
    char name[MAX_CHARACTERS];
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t numOfActions;
    uint8_t actionsReport[MAX_ACTIONS][BYTES_PER_ACTION];
};

// Initialises SPI to send HID reports to seeediuno.
void macrosInit(void);

// Returns the number of actions in the specified macro key.
uint8_t getMacroNumActions(uint8_t col, uint8_t row);

// Sets the name of specified macro in macroData.
void setMacroName(uint8_t col, uint8_t row, char* name);

// Sets the colour of specified macro in macroData.
void setMacroColour(uint8_t col, uint8_t row, uint8_t r, uint8_t g, uint8_t b);

// Sets the number of actions for specified macro in macroData.
void setMacroNumActions(uint8_t col, uint8_t row, uint8_t numActions);

// Sets the actions for specified macro in macroData.
void setMacroAction(uint8_t col, uint8_t row,
    uint8_t report[MAX_ACTIONS][BYTES_PER_ACTION]);

// Sends all actions of macro as HID report to seeeduino.
void executeMacro(uint8_t col, uint8_t row);

// Sends a release of all keys as HID report to seeeduino.
void sendRelease(void);

// Turns off the LED for use when blinking them.
void turnOffLed(uint8_t col, uint8_t row);

// Turns on the LED for use when blinking them.
void turnOnLed(uint8_t col, uint8_t row);

// Displays the macro name on the LCD display.
void displayMacroName(uint8_t col, uint8_t row);

// Sends all the macro data to GUI through USART.
void sendMacroData(void);

// Receives and sorts through all macro data received from GUI through USART.
uint16_t receiveMacroData(uint8_t* data);

// Stores all macro data to EEPROM.
void storeMacroData(void);

// Retrieves all macro data from EEPROM.
void getMacroData(void);