/******************************************************************************
CytronWiFiServer.h
Cytron WiFi Shield Library Server Header File
Created by Ng Beng Chet @ Cytron Technologies Sdn Bhd 
Original Creation Date: Sept 11, 2015
https://github.com/CytronTechnologies/CytronWiFiShield

Modified from ESP8266 WiFi Shield Library Server Header File
Credit to Jim Lindblom @ SparkFun Electronics
Original Creation Date: June 20, 2015
http://github.com/sparkfun/SparkFun_ESP8266_AT_Arduino_Library

!!! Description Here !!!

Development environment specifics:
	IDE: Arduino 1.6.5
	Hardware Platform: Arduino Uno
	ESP8266 WiFi Shield Version: 1.0

Distributed as-is; no warranty is given.
******************************************************************************/

#ifndef _CYTRONWIFISERVER_H_
#define _CYTRONWIFISERVER_H_

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <IPAddress.h>
//#include <Server.h>
#include "CytronWiFiShield.h"
#include "CytronWiFiClient.h"

class ESP8266Server //: public Server 
{
public:
	ESP8266Server(uint16_t);
	ESP8266Client available(uint8_t wait = 0);
	bool setTimeout(uint16_t time);
	void begin();
	void end();
	//virtual size_t write(uint8_t);
	//virtual size_t write(const uint8_t *buf, size_t size);
	uint8_t status();

	//using Print::write;
	
private:
	uint16_t _port;
};

#endif