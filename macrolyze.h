/*
 * macrolyze.h
 *
 * TEAM 01 ENGG2800
 */

#pragma once

#include <stdint.h>

#define F_CPU 11059200L

// Transfer modes.
#define RECEIVE_MODE 1
#define SEND_MODE 2
#define SOFTWARE_CONNECTED 3
#define SOFTWARE_DISCONNECTED 4
#define SEND_REPEAT_RATE 5
#define SEND_INITIAL_REPEAT_DELAY 6

// Time delays to compare with getCurrentTime().
#define START_SCREEN_DELAY 2000
#define DISPLAY_BRIGHTNESS_DELAY 1000
#define LED_BLINK_DELAY 50
#define RECEIVE_DELAY 1000
#define AUTO_BRIGHTNESS_DELAY 200

// Stores all configuration data on EEPROM.
void storeAllData(uint16_t initialRepeatDelay, uint16_t repeatPressDelay);