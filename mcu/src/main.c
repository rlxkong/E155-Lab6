/*
File: Lab_6_JHB.c
Author: Josh Brake
Email: jbrake@hmc.edu
Date: 9/14/19
*/


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "main.h"

/////////////////////////////////////////////////////////////////
// Provided Constants and Functions
/////////////////////////////////////////////////////////////////

//Defining the web page in two chunks: everything before the current time, and everything after the current time
char* webpageStart = "<!DOCTYPE html><html><head><title>E155 Web Server Demo Webpage</title>\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\
	</head>\
	<body><h1>E155 Web Server Demo Webpage</h1>";
char* ledStr = "<p>LED Control:</p>\
  <form action=\"ledon\"><input type=\"submit\" value=\"Turn the LED on!\"></form>\
	<form action=\"ledoff\"><input type=\"submit\" value=\"Turn the LED off!\"></form>";
char* tempResStr = "<p>Temperature Resolution Control:</p>\
  <form action=\"8\"><input type=\"submit\" value=\"8-bits\"></form>\
	<form action=\"9\"><input type=\"submit\" value=\"9-bits\"></form>\
  <form action=\"10\"><input type=\"submit\" value=\"10-bits\"></form>\
	<form action=\"11\"><input type=\"submit\" value=\"11-bits\"></form>\
  <form action=\"12\"><input type=\"submit\" value=\"12-bits\"></form>";
char* webpageEnd   = "</body></html>";

//determines whether a given character sequence is in a char array request, returning 1 if present, -1 if not present
int inString(char request[], char des[]) {
	if (strstr(request, des) != NULL) {return 1;}
	return -1;
}

int updateLEDStatus(char request[])
{
	int led_status = 0;
	// The request has been received. now process to determine whether to turn the LED on or off
	if (inString(request, "ledoff")==1) {
		digitalWrite(LED_PIN, PIO_LOW);
		led_status = 0;
	}
	else if (inString(request, "ledon")==1) {
		digitalWrite(LED_PIN, PIO_HIGH);
		led_status = 1;
	}

	return led_status;
}

int updateResConfig(char request[], char currentConfig)
{
  // changes the config sent to the sensor based on user selected configuration
  char config = currentConfig;

  // set resolution based on user input (r2 r1 r0)
  if(inString(request, "/REQ:8")==1){
    config = 0b11100000;
  }
  else if(inString(request, "/REQ:9")==1){
    config = 0b11100010;
  }
  else if(inString(request, "/REQ:10")==1){
    config = 0b11100100;
  }
  else if(inString(request, "/REQ:11")==1){
    config = 0b11100110;
  }
  else if(inString(request, "/REQ:12")==1){
    config = 0b11101000;
  }

  return config;
}

/////////////////////////////////////////////////////////////////
// Solution Functions
/////////////////////////////////////////////////////////////////

int main(void) {
  configureFlash();
  configureClock();    //default is 80Mhz

  gpioEnable(GPIO_PORT_A);
  gpioEnable(GPIO_PORT_B);
  gpioEnable(GPIO_PORT_C);

  pinMode(:ED_PIN, GPIO_OUTPUT);
  
  RCC->APB2ENR |= (RCC_APB2ENR_TIM15EN);
  initTIM(TIM15);
  
  USART_TypeDef * USART = initUSART(USART1_ID, 125000);

  // Initialize SPI
  // DS1722 max clk rate is 5MHz (DS1722 datasheet pg.11)
  // SCLK = fPLCK/br --> 80Mhz/2^(5+1) = 1.25 MHz
  // clock phase 1 (CPHA) based on DS1722 datasheet pg. 7
  initSPI(0b101, 0, 1);
  // Initialize temperature sensor (DS1722)
  initDS1722();

  // Initialize resolution
  char resConfig = 0b11100010;
  // Initalize buffers for text to be written into
  char resStr[20];
  char tempStr[20]; 
  char requestStr[20];

  while(1) {
    /* Wait for ESP8266 to send a request.
    Requests take the form of '/REQ:<tag>\n', with TAG begin <= 10 characters.
    Therefore the request[] array must be able to contain 18 characters.
    */

    // Receive web request from the ESP
    char request[BUFF_LEN] = "                  "; // initialize to known value
    int charIndex = 0;
  
    // Keep going until you get end of line character
    while(inString(request, "\n") == -1) {
      // Wait for a complete request to be transmitted before processing
      while(!(USART->ISR & USART_ISR_RXNE));
      request[charIndex++] = readChar(USART);
    }

    // Add SPI code here for reading temperature
    // Set temperature resolution based on user input
    char newConfig = updateResConfig(request, resConfig);

    if (newConfig != resConfig){
      resConfig = newConfig;
      // set the DS1722 configuration from user input from MCU
      writeDS1722(resConfig);
    }

    // read the temperature from DS1722
    float temp = readDS1722();
  
    // Update string with current LED state
    int led_status = updateLEDStatus(request);

    // convert temperature and configuration into string to print
    sprintf(tempStr, "%f C", temp);
    sprintf(resStr, "Resolution: 0x%02X", (unsigned char)resConfig);
    sprintf(requestStr, "Request: %s", request);

    char ledStatusStr[20];
    if (led_status == 1)
      sprintf(ledStatusStr,"LED is on!");
    else if (led_status == 0)
      sprintf(ledStatusStr,"LED is off!");

    // finally, transmit the webpage over UART
    sendString(USART, webpageStart); // webpage header code
    sendString(USART, ledStr); // button for controlling LED
    sendString(USART, tempResStr); // button for controlling resolution

    sendString(USART, "<h2>LED Status</h2>");
    sendString(USART, "<p>");
    sendString(USART, ledStatusStr);
    sendString(USART, "</p>");

    sendString(USART, "<h2>Temperature</h2>");
    sendString(USART, "<p>");
    sendString(USART, tempStr);
    sendString(USART, "</p>");

    sendString(USART, "<h2>Resolution</h2>");
    sendString(USART, "<p>");
    sendString(USART, resStr);
    sendString(USART, "</p>");
  
    sendString(USART, webpageEnd);
  }
}
