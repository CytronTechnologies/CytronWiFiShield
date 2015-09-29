/******************************************************************************
CytronWiFiClient.cpp
Cytron WiFi Shield Library Client Source File
Created by Ng Beng Chet @ Cytron Technologies Sdn Bhd 
Original Creation Date: Sept 11, 2015
https://github.com/CytronTechnologies/CytronWiFiShield

Modified from ESP8266 WiFi Shield Library Client Source File
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

#include "CytronWiFiShield.h"
#include "CytronWiFiClient.h"

ESP8266Client::ESP8266Client()
{
	ESP8266Client(ESP8266_SOCK_NOT_AVAIL);
}

ESP8266Client::ESP8266Client(uint8_t sock)
{
	_socket = sock;
}

uint8_t ESP8266Client::status()
{
	return wifi.status();
}

bool ESP8266Client::connect(String host, uint16_t port, uint32_t keepAlive)
{
	return connect(host.c_str(), port, keepAlive);
}
	
bool ESP8266Client::connect(IPAddress ip, uint16_t port, uint32_t keepAlive) 
{
	char ipAddress[16];
	sprintf(ipAddress, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	
	return connect((const char *)ipAddress, port, keepAlive);
}
	
bool ESP8266Client::connect(const char* host, uint16_t port, uint32_t keepAlive) 
{
	_socket = getSocket();
	//Serial.println(_socket);
    if (_socket != ESP8266_SOCK_NOT_AVAIL)
    {
		wifi._state[_socket] = TAKEN;
		return wifi.tcpConnect(_socket, host, port, keepAlive);
	}
	return false;
}

size_t ESP8266Client::write(uint8_t c)
{
	return write(&c, 1);
}

size_t ESP8266Client::write(const uint8_t *buf, size_t size)
{
	return wifi.tcpSend(_socket, buf, size);
}

int ESP8266Client::available()
{
	if(_socket==ESP8266_SOCK_NOT_AVAIL) return 0;
	
	int available = wifi.available();
	if (available == 0)
	{
		// Delay for the amount of time it'd take to receive one character
		//delayMicroseconds((1 * 10 * 1E6)/9600);
		delay(5);
		// Check again just to be sure:
		available = wifi.available();
	}
	
	//if(_socket==ESP8266_SOCK_NOT_AVAIL) return 0;
	//unsigned long timeIn = millis();
	//while(wifi.available()<=0&&(timeIn + 50)> millis());
	return wifi.available();
}

int ESP8266Client::read()
{
	return wifi.read();
}

int ESP8266Client::readBytes(char *buf, size_t size)
{
	return wifi.readBytes(buf,size);
}

int ESP8266Client::readBytes(uint8_t *buf, size_t size)
{
	return wifi.readBytes(buf,size);
}

String ESP8266Client::readString()
{
	return wifi.readString();
}

String ESP8266Client::readStringUntil(char c)
{
	return wifi.readStringUntil(c);
}

int ESP8266Client::peek()
{
	return wifi.peek();
}

void ESP8266Client::flush()
{
	wifi.flush();
}

void ESP8266Client::stop()
{
	if(_socket < ESP8266_MAX_SOCK_NUM)
	{
		wifi.close(_socket);
		wifi._state[_socket] = AVAILABLE;
	}
}

bool ESP8266Client::connected()
{
	// If data is available, assume we're connected. Otherwise,
	// we'll try to send the status query, and will probably end 
	// up timing out if data is still coming in.
	if (_socket == ESP8266_SOCK_NOT_AVAIL)
		return 0;
	else if (available() > 0)
		return 1;
	else if (status() == ESP8266_STATUS_CONNECTED)
		return 1;
	
	return 0;
}

ESP8266Client::operator bool()
{
	return connected();
}

// Private Methods
uint8_t ESP8266Client::getSocket()
{
	wifi.updateStatus();
	
	for (int i = ESP8266_MAX_SOCK_NUM; i >0; i--) 
	{
		if (wifi._state[i-1] == AVAILABLE)
		{
			return (i-1);
		}
	}
	return ESP8266_SOCK_NOT_AVAIL;
}

size_t ESP8266Client::send(uint8_t c)
{
	return wifi.write(c);
}
void ESP8266Client::sendCommand(const char * cmd, uint8_t type, const char * params)
{
	return wifi.sendCommand(cmd, type, params);
}

int16_t ESP8266Client::readForResponses(const char * pass, const char * fail, unsigned int timeout)
{
	return wifi.readForResponses(pass, fail, timeout);
}
