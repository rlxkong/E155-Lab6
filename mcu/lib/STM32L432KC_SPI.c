// STM32L432KC_SPI.c
// Rebecca Kong
// rkong@g.hmc.edu
// 10/22/2026
// SPI Functions

#include "STM32L432KC.h"
#include "STM32L432KC_GPIO.h"

/* Enables the SPI peripheral and intializes its clock speed (baud rate), polarity, and phase.
 *    -- br: (0b000 - 0b111). The SPI clk will be the master clock / 2^(BR+1).
 *    -- cpol: clock polarity (0: inactive state is logical 0, 1: inactive state is logical 1).
 *    -- cpha: clock phase (0: data captured on leading edge of clk and changed on next edge, 
 *          1: data changed on leading edge of clk and captured on next edge)
 * Refer to the datasheet for more low-level details. */ 
void initSPI(int br, int cpol, int cpha){
    // Turn off SPI1_CR1 to set configurations 
    SPI1->CR1 &= ~SPI_CR1_SPE;

    // Configure clock tree
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;

    // GPIO configurations
    gpioEnable(GPIO_PORT_B);
    pinMode(MOSI_PIN, GPIO_ALT);
    pinMode(MISO_PIN, GPIO_ALT);
    pinMode(SCK_PIN, GPIO_ALT);

    // Write proper GPIO registers
    GPIOA->AF[0] &= ~GPIOA_AFRL_AFSEL7;
    GPIOA->AF[0] &= ~GPIOA_AFRL_AFSEL6;
    GPIOA->AF[0] &= ~GPIOA_AFRL_AFSEL5;
    GPIOA->AF[0] |= _VAL2FLD(GPIOA_AFRL_AFSEL7, 0b0101);
    GPIOA->AF[0] |= _VAL2FLD(GPIOA_AFRL_AFSEL6, 0b0101);
    GPIOA->AF[0] |= _VAL2FLD(GPIOA_AFRL_AFSEL5, 0b0101);

    // Set SPI parameters
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_BR, br);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPOL, cpol);
    SPI1->CR1 |= _VAL2FLD(SPI_CR1_CPHA, cpha);

    // Configure master mode
    SPI1->CR1 |= SPI_CR1_MSTR;

    // Write to SPI_CR2 register
    // Turn off SPI1_CR2 to set configurations 
    SPI1->CR2 &= 0b0;

    // Configure the DS[3:0] to select data length for transfer
    SPI1->CR2 |= _VAL2FLD(SPI1_CR2_DS, 0b0111);

    // Configure SSOE
    SPI1->CR2 |= SPI_CR2_SSOE;

    // Configure FRXTH bit (based on DS->DR which is 8-bit)
    SPI1->CR2 |= SPI_CR2_FRXTH;

    // Enable SPI
    SPI1->CR1 |= SPI_CR1_SPE;
}

/* Transmits a character (1 byte) over SPI and returns the received character.
 *    -- send: the character to send over SPI
 *    -- return: the character received over SPI */
char spiSendReceive(char send){
    // Send character
    // wait for space in TXFIFO through TXE flag
    // wait while TXE bit in SR is not 1 or buffer is not empty
    while (!(SPI1->SR & SPI_SR_TXE)); 
    // write 8-bit access in one frame
    *((volatile char *)(&SPI1->DR)) = send;

    // Recieve character
    // wait for RXFIFO to receive 8-bits through RXNE flag
    // wait while RXNE fit is not 1 or buffer is not empty
    while (!(SPI1->SR & SPI_SR_RXNE)); 
    // read 8-bit access in one frame
    char rec = *((volatile char *)(&SPI1->DR));

    return rec;
}