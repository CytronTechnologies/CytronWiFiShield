/******************************************************************************
CytronWiFiClient.h
Cytron WiFi Shield Library Client Main Header File
Created by Ng Beng Chet @ Cytron Technologies Sdn Bhd 
Original Creation Date: Sept 11, 2015
https://github.com/CytronTechnologies/CytronWiFiShield

Modified from ESP8266 WiFi Shield Library Client Header File
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

#ifndef _CYTRONWIFICLIENT_H_
#define _CYTRONWIFICLIENT_H_

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <IPAddress.h>
//#include <Client.h>
#include "CytronWiFiShield.h"

class ESP8266Client : public Print
{
public:
	ESP8266Client();
	ESP8266Client(uint8_t sock);

	uint8_t status();
	
	bool connect(IPAddress ip, uint16_t port, uint32_t keepAlive=0);
	bool connect(String host, uint16_t port, uint32_t keepAlive = 0);
	bool connect(const char *host, uint16_t port, uint32_t keepAlive=0);
	
	/*virtual*/ int available();
	/*virtual*/ int read();
	/*virtual*/ int readBytes(uint8_t *buf, size_t size);
				String readString();
				String readStringUntil(char);
	/*virtual*/ int peek();
	/*virtual*/ void flush();
	/*virtual*/ void stop();
	/*virtual*/ uint8_t connected();
	//virtual operator bool();

	//friend class WiFiServer;
	
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);
	uint16_t  _socket;

private:
	static uint16_t _srcport;
	bool ipMuxEn;
	

	uint8_t getFirstSocket();
};

#endif