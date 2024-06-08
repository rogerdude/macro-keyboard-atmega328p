/*
 * usart.c
 *
 * Team 01 ENGG2800
 */

#include "usart.h"
#include "timer.h"
#include <avr/interrupt.h>
#include <avr/io.h>

/* usartInit()
 * -----------
 * Initialises USART with the specified UBRR value.
 *
 * ubrr: the value for the UBRR0 register to set the baud rate.
 */
void usartInit(uint8_t ubrr)
{
    // Set baud rate.
    UBRR0 = ubrr;

    // Enable receiver, transmitter, and reciever interrupt.
    UCSR0B |= (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);

    // Set frame format to 8 bits and 1 stop bit.
    UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
}

/* usartTransmit()
 * ---------------
 * Sends a byte through USART.
 *
 * data: the byte to send.
 */
void usartTransmit(uint8_t data)
{
    // Wait for transmit buffer to empty.
    while (!(UCSR0A & (1 << UDRE0))) { }

    // Send the data.
    UDR0 = data;
}
