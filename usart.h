/*
 * usart.h
 *
 * Team 01 ENGG2800
 */

#pragma once

#include <stdint.h>

#define UBRR 5 // Baud rate register value
#define RECEIVE_BUFFER 770 // Max buffer for bytes received through USART.
#define NUMBER_BUFFER 10 // Buffer for converting brightness level to number.

// Initialises USART with the specified UBRR value.
void usartInit(uint8_t ubrr);

// Sends a byte through USART.
void usartTransmit(uint8_t data);