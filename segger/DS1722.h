// DS1722.h
// Rebecca Kong
// rkong@g.hmc.edu
// 10/22/2026
// Header for DS1722 temperature sensor functions

#ifndef DS1722_H
#define DS1722_H

#include <stdint.h>
#include <stm32l432xx.h>
#include "STM32L432KC_GPIO.h"

// DS1722 pin definition
// chip select pin
#define CS_PIN PB0

///////////////////////////////////////////////////////////////////////////////
// Function prototypes
///////////////////////////////////////////////////////////////////////////////

// Initialize DS1722
void initDS1722(void);

// Write data from MCU
void writeDS1722(char data);

// Read data from DS1722
float readDS1722(void);

#endif