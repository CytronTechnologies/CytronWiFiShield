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
#include "CytronWiFiShield.h"
#include "CytronWiFiClient.h"

enum HTTPMethod { HTTP_ANY, HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_PATCH, HTTP_DELETE };

#define CONTENT_LENGTH_UNKNOWN ((size_t) -1)
#define CONTENT_LENGTH_NOT_SET ((size_t) -2)

class ESP8266Server : public Print
{
public:
	ESP8266Server(uint16_t port = 80);
	~ESP8266Server();
	ESP8266Client available();
	bool setTimeout(uint16_t time);
	void begin();
	void end();
	uint8_t status();
	bool hasClient();	
	void closeClient();
	String arg(const char* name);   // get request argument value by name
	String arg(int i);              // get request argument value by number
	String argName(int i);          // get request argument name by number
	int args();                     // get arguments count
	bool hasArg(const char* name);  // check if argument exists
	ESP8266Client client() { return _currentClient; }
	String uri() { return _currentUri; }
	HTTPMethod method() { return _currentMethod; }
	virtual size_t write(uint8_t);
	virtual size_t write(const uint8_t *buf, size_t size);
	
	template<typename T> 
	inline size_t write(T &src)
	{
		return _currentClient.write(src);
	}
	
protected:
  bool _parseRequest(ESP8266Client& client);
  void _parseArguments(String data);
  
  struct RequestArgument {
    String key;
    String value;
  };

  ESP8266Client  _currentClient;
  HTTPMethod  _currentMethod;
  String      _currentUri;

  size_t           _currentArgCount;
  RequestArgument* _currentArgs;
  
  size_t           _contentLength;
	
private:
	uint16_t _port;
};

#endif