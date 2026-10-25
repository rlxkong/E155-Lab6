// DS1722.c
// Rebecca Kong
// rkong@g.hmc.edu
// 10/22/2026
// DS1722 functions

#include "STM32L432KC.h"
#include "STM32L432KC_GPIO.h"

// Initialize DS1722
void initDS1722(void){
    // GPIO configurations
    gpioEnable(GPIO_PORT_B);
    pinMode(CS_PIN, GPIO_OUTPUT);

    // Chip select high
    digitalWrite(CS_PIN, PIO_HIGH);

    // write address for configuration (byte 1)
    // DS1722 datasheet pg 8
    spiSendReceive(0x80);
    // write the value being sent (byte 2)
    // DS1722 datasheet pg 5-6 w/ 9-bit res and 0.15s max conversion time
    // 111 1SHOT R2 R1 R0 SD
    spiSendReceive(0b11100010);

    // Chip select low
    digitalWrite(CS_PIN, PIO_LOW);
}

// Write data from MCU
void writeDS1722(char data){
    // chip select high
    digitalWrite(CS_PIN, PIO_HIGH);

    // write address for configuration
    spiSendReceive(0x80);
    // read the data value 
    spiSendReceive(data);

    //chip select low
    digitalWrite(CS_PIN, PIO_LOW);
}

// Read data from DS1722
float readDS1722(void){
    // read the fraction part of the temeperature (LSB)
    // chip select high
    digitalWrite(CS_PIN, PIO_HIGH);

    // write address for configuration LSB
    spiSendReceive(0x01);
    // read the data value
    int temp_lsb = spiSendReceive(0x00);

    // chip select low
    digitalWrite(CS_PIN, PIO_LOW);


    // read the whole number part of the temeperature (MSB)
    // chip select high
    digitalWrite(CS_PIN, PIO_HIGH);

    // write address for LSB
    spiSendReceive(0x02);
    // read data value
    int temp_msb = spiSendReceive(0x00);

    // chip select low
    digitalWrite(CS_PIN, PIO_LOW);

    // logic to determine positive or negative temperature (due to it being in twos-complement)
    // sets it to negative when the first bit is 1 
    if(temp_lsb >= 128){
        temp_msb -= 256;
    }

    // combining integer and fraction portions of the temperature into one value
    // divide temp_lsb by 256 (float) to ensure that it is the actual decimal value (in bits it is just the integer)
    float temp = temp_msb + (temp_lsb / 256.0f)

    return temp
}