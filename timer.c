/*
 * timer.c
 *
 * Team 01 ENGG2800
 */

#include "timer.h"
#include <avr/common.h>
#include <avr/interrupt.h>
#include <avr/io.h>

// Initialise variable that stores current time in ms
uint32_t currentTime;

/* initTimerZero()
 * ---------------
 * Initialises timer 0 to count the time in milliseconds.
 */
void initTimerZero(void)
{
    // Set current time to 0.
    currentTime = 0L;

    // Clear timer.
    TCNT0 = 0;

    // Set OCR0A to 42 because required for 1000 Hz frequency.
    OCR0A = 42;

    // Set Timer 0 to CTC mode, with clock prescalar of 256.
    TCCR0A |= (1 << WGM01);
    TCCR0B |= (1 << CS02);

    // Enable interrupt so it adds 1ms to currentTime when timer is cleared.
    TIMSK0 |= (1 << OCIE0A);

    // Clear interrupt flag.
    TIFR0 = (1 << OCF0A);
}

/* initTimerOne()
 * --------------
 * Initialises timer 1 to set back light brightness of LCD.
 */
void initTimerOne(void)
{
    // Set Port B1 to output for LCD back light pin.
    DDRB |= (1 << 1);

    // Set Timer 1 to Fast PWM mode in non-inverting mode.
    TCCR1A |= (1 << COM1A1) | (0 << COM1A0) | (1 << WGM10);
    TCCR1B |= (1 << WGM12) | (1 << CS10);
}

/* getCurrentTime()
 * ----------------
 * Gets the time since the MCU started running.
 *
 * Returns: the time since MCU started running.
 */
uint32_t getCurrentTime(void)
{
    uint32_t time;

    // Check if interrupts were enabled.
    uint8_t interruptEnabled = bit_is_set(SREG, SREG_I);

    cli(); // Disable interrupt while currentTime is retrieved.

    time = currentTime;

    if (interruptEnabled)
        sei();

    return time;
}

// Add 1ms to currentTime whenever timer 0 is cleared.
ISR(TIMER0_COMPA_vect)
{
    currentTime++;
}