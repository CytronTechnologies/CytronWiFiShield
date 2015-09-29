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

#include "util/ESP8266_AT.h"

class ESP8266Client : public Print
{
public:
	ESP8266Client();
	ESP8266Client(uint8_t sock);

	uint8_t status();
	
	bool connect(IPAddress ip, uint16_t port, uint32_t keepAlive=0);
	bool connect(String host, uint16_t port, uint32_t keepAlive = 0);
	bool connect(const char *host, uint16_t port, uint32_t keepAlive=0);
	
	int available();
	int read();
	int readBytes(char *buf, size_t size);
	int readBytes(uint8_t *buf, size_t size);
	String readString();
	String readStringUntil(char);
	int peek();
	void flush();
	void stop();
	bool connected();
	operator bool();

	friend class ESP8266Class;
	
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);
	
	uint8_t  _socket;
	
template<typename T> 
inline size_t write(T &src)
{
	size_t doneLen = 0;
	
	while (src.available() > 2048)
	{
		int i = 2048;
		char params[8];
		sprintf(params, "%d,%d", _socket, i);
		sendCommand(ESP8266_TCP_SEND, 1, params);

		int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, 5000);
		if (rsp > 0)
		{
			while(i--)
				doneLen += send(src.read());
			
			rsp = readForResponses(RESPONSE_OK, RESPONSE_FAIL, 5000);
			if(rsp <=0) return rsp;
		}
		else return rsp;
	}
	
	uint16_t leftLen = src.available();
	char params[8];
	sprintf(params, "%d,%d", _socket, leftLen);
	sendCommand(ESP8266_TCP_SEND, 1, params);
	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, 5000);
	if (rsp > 0)
	{
		while(leftLen--)
			doneLen += send(src.read());
		
		rsp = readForResponses(RESPONSE_OK, RESPONSE_FAIL, 5000);
		if(rsp > 0) 
			return doneLen;
	}
	
	return rsp;
}
	
private:

	uint8_t getSocket();
	
	//////////////////////////
	// Command Send/Receive //
	//////////////////////////
	size_t send(uint8_t);
	void sendCommand(const char * cmd, uint8_t type = 2, const char * params = NULL);
	int16_t readForResponses(const char * pass, const char * fail, unsigned int timeout);
};

#endif