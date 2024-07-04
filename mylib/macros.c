/*
 * macros.c
 *
 * TEAM 01 ENGG2800
 */

#include "macros.h"
#include "16bitcolours.h"
#include "addresses.h"
#include "keypad.h"
#include "lcd.h"
#include "memory.h"
#include "rgbled.h"
#include "usart.h"
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>
#include <util/delay.h>

// Global variable array to store all macro data for all keys to make it
// easier to access from macrolyze.c.
struct MacroData macros[COLS][ROWS];

/* macrosInit()
 * ------------
 * Initialises SPI to send HID reports to seeediuno.
 */
void macrosInit(void)
{
    // Set IDLE pin to input.
    DDRD &= ~(1 << 5);

    // Set SS pin for seeeduino to output.
    DDRC |= (1 << 1);

    // SPI is already setup by St7735 library, however clock speed must be
    // divided by 16 to avoid damaging seeeduino.
    SPCR |= (1 << SPR0);

    // Use Port C1 for SS and set to 1
    PORTC |= (1 << 1);

    // Initialise colour for brightness auxiliary key.
    setMacroColour(2, 2, 255, 255, 255);
    setMacroNumActions(2, 2, 1);

    // Initialise colour for preview mode auxiliary key.
    setMacroColour(3, 2, 255, 255, 255);
}

/* actionTransmit()
 * ----------------
 * Sends a byte through SPI.
 *
 * byte: the byte to send through SPI.
 */
void actionTransmit(uint8_t byte)
{
    // Start transmission
    SPDR = byte;

    // Wait until transmission completes
    while (!(SPSR & (1 << SPIF))) { }
}

/* setMacroName()
 * --------------
 * Sets the name of specified macro in macroData.
 *
 * col: the column of macro key to set.
 * row: the row of macro key to set.
 * name: a pointer to an array with the macro name to set.
 */
void setMacroName(uint8_t col, uint8_t row, char* name)
{
    uint8_t nameLength = strlen(name);

    for (uint8_t character = 0; character < nameLength; character++)
        macros[col][row].name[character] = name[character];
    macros[col][row].name[nameLength] = 0x00;
}

/* getMacroNumActions()
 * --------------------
 * Gets the number of actions in the specified macro key.
 *
 * col: the column of macro key to set.
 * row: the row of macro key to set.
 *
 * Returns: the number of actions of specified macro
 */
uint8_t getMacroNumActions(uint8_t col, uint8_t row)
{
    return macros[col][row].numOfActions;
}

/* setMacroColour()
 * ----------------
 * Sets the colour of specified macro in macroData.
 *
 * col: the column of macro key to set.
 * row: the row of macro key to set.
 * r: the amount of red in the colour.
 * g: the amount of green in the colour.
 * b: the amount of blue in the colour.
 */
void setMacroColour(uint8_t col, uint8_t row,
    uint8_t r, uint8_t g, uint8_t b)
{
    macros[col][row].red = r;
    macros[col][row].green = g;
    macros[col][row].blue = b;
    setLedColour(col, row, r, g, b);
}

/* setMacroNumActions()
 * --------------
 * Sets the number of actions for specified macro in macroData.
 *
 * col: the column of macro key to set.
 * row: the row of macro key to set.
 * numActions: the number of actions in the macro.
 */
void setMacroNumActions(uint8_t col, uint8_t row, uint8_t numActions)
{
    macros[col][row].numOfActions = numActions;
}

/* setMacroAction()
 * --------------
 * Sets the actions for specified macro in macroData.
 *
 * col: the column of macro key to set.
 * row: the row of macro key to set.
 * report: an array with all the actions of the macro.
 */
void setMacroAction(uint8_t col, uint8_t row,
    uint8_t report[MAX_ACTIONS][BYTES_PER_ACTION])
{
    uint8_t numActions = macros[col][row].numOfActions;

    // Iterate through all bytes in report and store to macroData.
    for (uint8_t action = 0; action < numActions; action++) {
        for (uint8_t key = 0; key < BYTES_PER_ACTION; key++)
            macros[col][row].actionsReport[action][key] = report[action][key];
    }
}

/* modifyReport()
 * --------------
 * Modifies the HID report based on data for the current action.
 *
 * hidReport: a pointer to an array of 8 bytes containing the HID report.
 * keyPressed: the HID code for the key in the current action.
 * keyData: the byte containing information about whether the keyPressed is a
 *    modifier or if it is pressed or released.
 */
void modifyReport(uint8_t* hidReport, uint8_t keyPressed, uint8_t keyData)
{
    uint8_t modifier = keyData & (1 << 7); // Indicates whether key is a modifier.
    uint8_t press = keyData & (1 << 6); // Indicates whether key is pressed.

    if ((keyPressed != RELEASE_ALL_KEYS) && modifier) {
        // Modify first byte of HID report if key is a modifier.
        if (press)
            hidReport[0] |= keyPressed;
        else
            hidReport[0] &= ~(keyPressed);
    } else if (keyPressed != RELEASE_ALL_KEYS) {
        // Modify remainder of HID report for standard keys.
        for (int i = 2; i < KEYS_PER_ACTION; i++) {
            if (press) {
                if (!hidReport[i]) {
                    hidReport[i] = keyPressed;
                    break;
                }
            } else {
                if (hidReport[i] == keyPressed) {
                    hidReport[i] = EMPTY_KEY;
                    break;
                }
            }
        }
    }
}

/* sendReport()
 * ------------
 * Sends a HID report of 8 bytes through SPI.
 *
 * hidReport: a pointer to an array of 8 bytes containing the HID report.
 */
void sendReport(uint8_t* hidReport)
{
    // Wait until IDLE pin is high
    while (!(PIND & (1 << 5))) { }

    // Set SS high to start transmission of 1 action.
    PORTC &= ~(1 << 1);
    for (uint8_t key = 0; key < KEYS_PER_ACTION; key++) {
        actionTransmit(hidReport[key]);
    }

    // Wait 20us
    _delay_us(HID_DELAY);

    // Set SS low to end transmission of 1 action.
    PORTC |= (1 << 1);
}

/* executeMacro()
 * --------------
 * Sends all actions of macro as HID reports to seeeduino.
 *
 * col: the column of macro key to set.
 * row: the row of macro key to set.
 */
void executeMacro(uint8_t col, uint8_t row)
{
    uint8_t numOfActions = macros[col][row].numOfActions;
    if (!numOfActions)
        return;

    // Return if key is one of the auxiliary keys
    if ((col == 3 && row == 2) || (col == 2 && row == 2))
        return;

    uint8_t keyPressed;
    uint8_t keyData;

    // Initialise empty HID report.
    uint8_t hidReport[KEYS_PER_ACTION];
    for (uint8_t i = 0; i < KEYS_PER_ACTION; i++)
        hidReport[i] = EMPTY_KEY;

    // Iterate through all actions in report.
    for (uint8_t action = 0; action < numOfActions; action++) {

        // Get the HID code for the key in the action.
        keyPressed = macros[col][row].actionsReport[action][0];

        // Get the encoded byte with data about the action.
        keyData = macros[col][row].actionsReport[action][1];

        // Empty the HID report if the action is to release all keys.
        if (keyPressed == RELEASE_ALL_KEYS) {
            for (uint8_t i = 0; i < KEYS_PER_ACTION; i++)
                hidReport[i] = EMPTY_KEY;
        }

        // Modify the HID report with data for current action and send it.
        modifyReport(hidReport, keyPressed, keyData);
        sendReport(hidReport);
    }
    sendRelease();
}

/* sendRelease()
 * -------------
 * Sends a release of all keys as HID report to seeeduino.
 */
void sendRelease(void)
{
    // Make empty HID report for release
    uint8_t hidReport[KEYS_PER_ACTION];
    for (uint8_t i = 0; i < KEYS_PER_ACTION; i++)
        hidReport[i] = EMPTY_KEY;

    sendReport(hidReport);
}

/* turnOffLed()
 * ------------
 * Turns off the LED for use when blinking them.
 *
 * col: the column of macro key LED to set.
 * row: the row of macro key LED to set.
 */
void turnOffLed(uint8_t col, uint8_t row)
{
    setLedColour(col, row, 0, 0, 0);
}

/* turnOnLed()
 * -----------
 * Turns on the LED for use when blinking them.
 *
 * col: the column of macro key LED to set.
 * row: the row of macro key LED to set.
 */
void turnOnLed(uint8_t col, uint8_t row)
{
    uint8_t red = macros[col][row].red;
    uint8_t green = macros[col][row].green;
    uint8_t blue = macros[col][row].blue;
    setLedColour(col, row, red, green, blue);
}

/* displayMacroName()
 * ------------------
 * Displays the macro name on the LCD display.
 *
 * col: the column of macro key to set.
 * row: the row of macro key to set.
 */
void displayMacroName(uint8_t col, uint8_t row)
{
    uint8_t nameLength = strlen(macros[col][row].name);
    if (!macros[col][row].numOfActions)
        return;

    uint8_t x = 80; // Default x position for text.
    uint8_t y = 55; // Default y position for text.

    uint8_t size = 3; // Default offset for positioning text.
    if (nameLength < 14)
        size = 5;

    x = x - (nameLength * size); // New x position based on name length.
    drawText(macros[col][row].name, x, y);
}

/* sendMacroData()
 * ---------------
 * Sends all the macro data to GUI through USART. Format to send is to first
 * send 'M', then send key number, number of actions, 30 characters for name,
 * 3 bytes for colour, then 2 bytes per action for every action.
 */
void sendMacroData(void)
{
    // Send macro data for all keys based on required format.
    for (uint8_t key = 1; key <= 10; key++) {
        usartTransmit('M');
        uint8_t* keyIndex;
        uint8_t matrixLocation[2];
        keyIndex = keyLocation(matrixLocation, key); // Get matrix location of key.
        uint8_t col = keyIndex[0];
        uint8_t row = keyIndex[1];

        usartTransmit(key);

        uint8_t numActions = macros[col][row].numOfActions;
        usartTransmit(numActions);

        // Send all character of name which are not 0x00.
        uint8_t length = strlen(macros[col][row].name);
        for (uint8_t i = 0; i < length; i++)
            usartTransmit(macros[col][row].name[i]);

        // Then send 0x00 for empty characters of name.
        for (uint8_t i = length; i < 30; i++)
            usartTransmit(0x00);

        // Send colour.
        usartTransmit(macros[col][row].red);
        usartTransmit(macros[col][row].green);
        usartTransmit(macros[col][row].blue);

        // Send actions.
        for (uint8_t i = 0; i < numActions; i++) {
            usartTransmit(macros[col][row].actionsReport[i][0]);
            usartTransmit(macros[col][row].actionsReport[i][1]);
        }
    }
}

/* receiveMacroData()
 * ------------------
 * Receives and sorts through all macro data received from GUI through USART.
 * Format to receive data is similar to how data is sent to GUI.
 *
 * data: a pointer to an array with all the data received from USART.
 *
 * Returns: the index of the last byte read from data.
 */
uint16_t receiveMacroData(uint8_t* data)
{
    uint16_t counter = 0;
    for (uint8_t keyNum = 1; keyNum <= 10; keyNum++) {
        // Ignore 'M' character
        counter++;

        // Get key's row and col based on received macro number
        uint8_t key = data[counter++];
        uint8_t* keyIndex;
        uint8_t matrixLocation[2];
        keyIndex = keyLocation(matrixLocation, key);
        uint8_t col = keyIndex[0];
        uint8_t row = keyIndex[1];

        setMacroNumActions(col, row, data[counter++]);
        uint8_t numActions = getMacroNumActions(col, row);

        char name[31];
        uint8_t colour[3];

        // Get name
        for (uint8_t i = 0; i < 30; i++)
            name[i] = data[counter++];
        name[30] = 0x00;

        // Get colour data
        for (uint8_t i = 0; i < 3; i++)
            colour[i] = data[counter++];

        // Get macro action data
        uint8_t macroActions[numActions][BYTES_PER_ACTION];
        for (uint8_t i = 0; i < numActions; i++) {
            macroActions[i][0] = data[counter++];
            macroActions[i][1] = data[counter++];
        }

        setMacroName(col, row, name);
        setMacroColour(col, row, colour[0], colour[1], colour[2]);
        setMacroAction(col, row, macroActions);
    }
    return counter;
}

/* storeMacroData()
 * ----------------
 * Stores all macro data to EEPROM.
 */
void storeMacroData(void)
{
    for (uint8_t key = 1; key <= 10; key++) {
        uint8_t* keyIndex;
        uint8_t matrixLocation[2];
        keyIndex = keyLocation(matrixLocation, key); // Get matrix location of key.
        uint8_t col = keyIndex[0];
        uint8_t row = keyIndex[1];

        // Store name.
        for (uint8_t i = 0; i < 30; i++) {
            eepromWrite(NAME_ADDRESS + ((key - 1) * 30) + i,
                macros[col][row].name[i]);
        }

        // Store colour.
        uint8_t colour[3];
        colour[0] = macros[col][row].red;
        colour[1] = macros[col][row].green;
        colour[2] = macros[col][row].blue;
        for (uint8_t i = 0; i < 3; i++)
            eepromWrite(COLOUR_ADDRESS + ((key - 1) * 3) + i, colour[i]);

        // Store number of actions.
        eepromWrite(NUM_ACTIONS_ADDRESS + ((key - 1) * 1),
            macros[col][row].numOfActions);

        // Store all actions.
        for (uint8_t i = 0; i < macros[col][row].numOfActions; i++) {
            eepromWrite(ACTIONS_ADDRESS + ((key - 1) * 40) + 0 + (i * 2),
                macros[col][row].actionsReport[i][0]);
            eepromWrite(ACTIONS_ADDRESS + ((key - 1) * 40) + 1 + (i * 2),
                macros[col][row].actionsReport[i][1]);
        }
    }
}

/* getMacroData()
 * --------------
 * Retrieves all macro data from EEPROM.
 */
void getMacroData(void)
{
    for (uint8_t key = 1; key <= 10; key++) {
        uint8_t* keyIndex;
        uint8_t matrixLocation[2];
        keyIndex = keyLocation(matrixLocation, key); // Get matrix location of key.
        uint8_t col = keyIndex[0];
        uint8_t row = keyIndex[1];

        // Get name.
        char name[31];
        for (uint8_t i = 0; i < 30; i++) {
            name[i] = eepromRead((uint16_t)NAME_ADDRESS + ((key - 1) * 30) + i);
        }
        name[30] = 0x00;
        setMacroName(col, row, name);

        // Get colour.
        uint8_t colour[3];
        for (uint8_t i = 0; i < 3; i++) {
            colour[i] = eepromRead((uint16_t)COLOUR_ADDRESS + ((key - 1) * 3) + i);
        }
        setMacroColour(col, row, colour[0], colour[1], colour[2]);

        // Get number of actions.
        uint8_t numActions = eepromRead((uint16_t)NUM_ACTIONS_ADDRESS + ((key - 1) * 1));
        setMacroNumActions(col, row, numActions);

        // Get all actions of macro.
        uint8_t macroActions[MAX_ACTIONS][BYTES_PER_ACTION];
        for (uint8_t i = 0; i < numActions; i++) {
            macroActions[i][0] = eepromRead((uint16_t)ACTIONS_ADDRESS
                + ((key - 1) * 40) + 0 + (i * 2));
            macroActions[i][1] = eepromRead((uint16_t)ACTIONS_ADDRESS
                + ((key - 1) * 40) + 1 + (i * 2));
        }
        setMacroAction(col, row, macroActions);
    }
}