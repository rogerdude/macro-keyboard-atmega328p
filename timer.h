/*
 * timer.h
 *
 * Team 01 ENGG2800
 */

#pragma once

#include <stdint.h>

// Initialises timer 0 to count the time in milliseconds.
void initTimerZero(void);

// Initialises timer 1 to set back light brightness of LCD.
void initTimerOne(void);

// Returns the time since the MCU started running.
uint32_t getCurrentTime(void);