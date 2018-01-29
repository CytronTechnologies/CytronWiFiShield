/******************************************************************************
CytronWiFiShield.cpp
Cytron WiFi Shield Library Main Source File
Created by Ng Beng Chet @ Cytron Technologies Sdn Bhd 
Original Creation Date: Sept 11, 2015
https://github.com/CytronTechnologies/CytronWiFiShield

Modified from ESP8266 WiFi Shield Library Main Source File
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
#include "util/ESP8266_AT.h"

#define ESP8266_DISABLE_ECHO 0
#define SERVER_TIMEOUT 5
#define baudRate 9600

char esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
unsigned int bufferHead;
////////////////////
// Initialization //
////////////////////

ESP8266Class wifi;

ESP8266Class::ESP8266Class()
{
	for (int i=0; i<ESP8266_MAX_SOCK_NUM; i++)
		_state[i] = AVAILABLE;
}

bool ESP8266Class::begin(HardwareSerial &hSerial){
	
	hSerial.begin(baudRate);
	while(!hSerial);
	_serial = &hSerial;
	isHardwareSerial = true;
	delayMicroseconds(100000);
	
	return init();
}

bool ESP8266Class::begin(uint8_t rx_pin, uint8_t tx_pin)
{
	if (rx_pin==0&&tx_pin==1)
	{
		Serial.begin(baudRate);
		_serial = &Serial;
		isHardwareSerial = true;
	}
	
	else
	{
		swSerial = new SoftwareSerial(rx_pin, tx_pin);
		swSerial->begin(baudRate);
		_serial = swSerial;
		isHardwareSerial = false;
	}

	return init();
}

bool ESP8266Class::init()
{
	if (reset()&&test())
	{
#ifdef ESP8266_DISABLE_ECHO
		if (!echo(false))
			return false;
#endif
		if (!setMux(true))
			return false;
		
		//TODO: list to do if successfully talking to esp8266
		if(!setMode(WIFI_STA))
			return false;
		
		if(!showInfo(true))
			return false;
		
		if(!setAutoConn(false))
			return false;
		
		return true;
	}
	
	return false;
}

///////////////////////
// Basic AT Commands //
///////////////////////
bool ESP8266Class::setAutoConn(bool enable)
{
	sendCommand(ESP8266_AUTO_CONNECT, ESP8266_CMD_SETUP, (enable==true) ? "1" : "0"); // Send AT

	if (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0)
		return true;
	
	return false;
}

bool ESP8266Class::showInfo(bool enable)
{
	sendCommand(ESP8266_SHOW_INFO, ESP8266_CMD_SETUP, (enable==true) ? "1" : "0"); // Send AT

	if (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0)
		return true;
	
	return false;
}

bool ESP8266Class::test()
{
	sendCommand(ESP8266_TEST); // Send AT

	if (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0)
		return true;
	
	return false;
}

bool ESP8266Class::reset()
{
	sendCommand(ESP8266_RESET); // Send AT+RST
	
	if (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0)
		clearBuffer();
	
	int resp = readForResponses("ready", "invalid", COMMAND_RESET_TIMEOUT);	

	if (!(resp > 0 || resp == -3))
		return false;
	if (!echo(false))
		return false;
	return true;
}

bool ESP8266Class::echo(bool enable)
{
	if (enable)
		sendCommand(ESP8266_ECHO_ENABLE);
	else
		sendCommand(ESP8266_ECHO_DISABLE);
	
	if (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0)
		return true;
	
	return false;

}
/*
bool ESP8266Class::setBaud(unsigned long baud)
{
	char parameters[16];
	memset(parameters, 0, 16);
	// Constrain parameters:
	baud = constrain(baud, 110, 115200);
	
	// Put parameters into string
	sprintf(parameters, "%d,8,1,0,0", baud);
	
	// Send AT+UART=baud,databits,stopbits,parity,flowcontrol
	sendCommand(ESP8266_UART, ESP8266_CMD_SETUP, parameters);
	
	if (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0)
		return true;
	
	return false;
}*/

String ESP8266Class::firmwareVersion()
{
	sendCommand(ESP8266_VERSION); // Send AT+GMR
	// Example Response: AT version:0.30.0.0(Jul  3 2015 19:35:49)\r\n (43 chars)
	//                   SDK version:1.2.0\r\n (19 chars)
	//                   compile time:Jul  7 2015 18:34:26\r\n (36 chars)
	//                   OK\r\n
	// (~101 characters)
	// Look for "OK":

	String version = "";
	unsigned int timeout = COMMAND_RESPONSE_TIMEOUT;
	while (timeout--) // While we haven't timed out
	{
		if (_serial->available()) // If data is available on UART RX
		{
			version += (char)_serial->read();
			
			if(strstr(version.c_str(), RESPONSE_OK))	// Search the buffer for goodRsp
			{
				version.replace("\r\n\r\n","\r\n");
				version.replace("\r\nOK\r\n","");
				return version;
			}	// Return how number of chars read				
		}
		delay(1);
	}
	return "";
}

////////////////////
// WiFi Functions //
////////////////////

// getMode()
// Input: None
// Output:
//    - Success: 1, 2, 3 (WIFI_STA, WIFI_AP, WIFI_BOTH)
//    - Fail: <0 (esp8266_cmd_rsp)
int8_t ESP8266Class::getMode()
{
	sendCommand(ESP8266_WIFI_MODE, ESP8266_CMD_QUERY);
	
	// Example response: \r\nAT+CWMODE_CUR?\r+CWMODE_CUR:2\r\n\r\nOK\r\n
	// Look for the OK:
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	
	if (rsp > 0)
	{
		char * p = strchr(esp8266RxBuffer, ':');
		if (p != NULL)
		{
			char mode = *(p+1);
			if ((mode >= '1') && (mode <= '3'))
				return (mode - 48); // Convert ASCII to decimal
		}
		
		return ESP8266_RSP_UNKNOWN;
	}
	
	return rsp;
}

// setMode()
// Input: 1, 2, 3 (WIFI_STA, WIFI_AP, WIFI_BOTH)
// Output:
//    - Success: >0
//    - Fail: <0 (esp8266_cmd_rsp)
bool ESP8266Class::setMode(esp8266_wifi_mode mode)
{
	char modeChar[2] = {0, 0};
	sprintf(modeChar, "%d", mode);

	sendCommand(ESP8266_WIFI_MODE, ESP8266_CMD_SETUP, (const char*)modeChar);
	
	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT)> 0 ? true : false );
}

bool ESP8266Class::connectAP(const char * ssid)
{
	return connectAP(ssid, "");
}

// connect()
// Input: ssid and pwd const char's
// Output:
//    - Success: >0
//    - Fail: <0 (esp8266_cmd_rsp)
bool ESP8266Class::connectAP(const char * ssid, const char * pwd)
{
	//_serial->print((const __FlashStringHelper*)ESP8266_AT);
	_serial->write(0x41);_serial->write(0x54);
	_serial->print((const __FlashStringHelper*)ESP8266_CONNECT_AP);
	//_serial->print(F("=\""));
	_serial->write(0x3d);_serial->write(0x22);
	_serial->print(ssid);
	//_serial->print(F("\""));
	_serial->write(0x22);
	if (pwd != NULL)
	{
		//_serial->print(F(","));
		_serial->write(0x2c);
		//_serial->print(F("\""));
		_serial->write(0x22);
		_serial->print(pwd);
		//_serial->print(F("\""));
		_serial->write(0x22);
	}
	//_serial->print(F("\r\n"));
	_serial->write(0x0d);_serial->write(0x0a);
	
	return (readForResponses(RESPONSE_OK, RESPONSE_FAIL, WIFI_CONNECT_TIMEOUT) > 0 ? true : false );
}

// update SSID and RSSI
String ESP8266Class::SSID()
{
	sendCommand(ESP8266_CONNECT_AP, ESP8266_CMD_QUERY); // Send "AT+CWJAP?"

	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	// Example Responses: No AP\r\n\r\nOK\r\n
	// - or -
	// +CWJAP:"WiFiSSID","00:aa:bb:cc:dd:ee",6,-45\r\n\r\nOK\r\n
	
	if (rsp > 0)
	{
		// Look for "No AP"
		if (searchBuffer("No AP") != NULL) return "";
	
		// Look for "+CWJAP"
		char * p = searchBuffer("+CWJAP:");
		if (p != NULL)
		{
			p += strlen(ESP8266_CONNECT_AP) + 2;
			char * q = strchr(p, '"');
			if (q == NULL) return "";
			char ssid[50]={0};
			strncpy(ssid, p, q-p); // Copy string to temp char array:
			return(String)ssid;
		}
	}
}

int ESP8266Class::RSSI()
{
	sendCommand(ESP8266_CONNECT_AP, ESP8266_CMD_QUERY); // Send "AT+CWJAP?"

	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	// Example Responses: No AP\r\n\r\nOK\r\n
	// - or -
	// +CWJAP:"WiFiSSID","00:aa:bb:cc:dd:ee",6,-45\r\n\r\nOK\r\n

	if (rsp > 0)
	{
		// Look for "No AP"
		if (strstr(esp8266RxBuffer, "No AP") != NULL) return 0;
	
		// Look for "+CWJAP"
		char * p = strstr(esp8266RxBuffer, "+CWJAP:");
		if (p != NULL)
		{
			p += strlen(ESP8266_CONNECT_AP) + 2;
			char * q = strchr(p, '"');
			if (q == NULL) return 0;
			q+=3;
			q = strchr(q, '"');
			if (q == NULL) return 0;
			q+=2;
			q = strchr(q, ',');
			if (q == NULL) return 0;
			q+=1;
			char *r = strchr(q, '\r');
			if (r == NULL) return 0;
			char rssi[10]={0};
			strncpy(rssi, q, r-q); // Copy string to temp char array:
			return atoi(rssi); // Move the temp char into IP Address octet		
			//return (String)ssid;
		}
	}
}

bool ESP8266Class::disconnectAP()
{
	sendCommand(ESP8266_DISCONNECT); // Send AT+CWQAP
	// Example response: \r\n\r\nOK\r\nWIFI DISCONNECT\r\n
	// "WIFI DISCONNECT" comes up to 500ms _after_ OK. 
	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0 ? true : false);
}

// localIP()
// Input: none
// Output:
//    - Success: Device's local IPAddress
//    - Fail: 0
IPAddress ESP8266Class::localIP()
{
	sendCommand(ESP8266_SET_STA_IP, ESP8266_CMD_QUERY); //Send AT+CIFSTA?
	// Example Response: +CIPSTA:ip,"192.168.0.114"\r\n
	//                   +CIPSTA:gateway,"192.168.1.1"\r\n
	//					 +CIPSTA:subnet,"255.255.255.0"\r\n
	//                   \r\n
	//                   OK\r\n
	// Look for the OK:
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rsp > 0)
	{
		// Look for "ip" in the esp8266RxBuffer
		char * p = strstr(esp8266RxBuffer, "ip");
		
		if (p != NULL)
		{
			IPAddress returnIP;
			
			p += 4; // Move p four places. (skip "ip,")
			for (uint8_t i = 0; i < 4; i++)
			{
				char tempOctet[4];
				memset(tempOctet, 0, 4); // Clear tempOctet
				
				size_t octetLength = strspn(p, "0123456789"); // Find length of numerical string:
				if (octetLength >= 4) // If it's too big, return an error
					return ESP8266_RSP_UNKNOWN;
				
				strncpy(tempOctet, p, octetLength); // Copy string to temp char array:
				returnIP[i] = atoi(tempOctet); // Move the temp char into IP Address octet
				
				p += (octetLength + 1); // Increment p to next octet
			}
			
			return returnIP;
		}
	}
	
	return rsp;
}

bool ESP8266Class::config(IPAddress ip, IPAddress gateway, IPAddress subnet)
{
	//_serial->print((const __FlashStringHelper*)ESP8266_AT);
	_serial->write(0x41);_serial->write(0x54);
	_serial->print((const __FlashStringHelper*)ESP8266_SET_STA_IP);
	//_serial->print(F("=\""));
	_serial->write(0x3d);_serial->write(0x22);
	_serial->print(ip);
	//_serial->print(F("\",\""));
	_serial->write(0x22);_serial->write(0x2c);_serial->write(0x22);
	_serial->print(gateway);
	//_serial->print(F("\",\""));
	_serial->write(0x22);_serial->write(0x2c);_serial->write(0x22);
	_serial->print(subnet);
	//_serial->print(F("\"\r\n"));
	_serial->write(0x22);
	_serial->write(0x0d);_serial->write(0x0a);
	
	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0 ? true : false);
	
}

bool ESP8266Class::softAP(const char * ssid, const char * pwd, uint8_t channel_id, uint8_t enc)
{
	char params[50] = {0};
	if(strlen(pwd)<8 || strlen(pwd) > 64) return false;
	sprintf(params, "\"%s\",\"%s\",%i,%i", ssid, pwd, channel_id, enc);
	sendCommand(ESP8266_AP_CONFIG, ESP8266_CMD_SETUP, (const char*)params);
	
	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0 ? true : false);
	
}

IPAddress ESP8266Class::softAPIP()
{
	sendCommand(ESP8266_SET_AP_IP, ESP8266_CMD_QUERY);
	// Example Response: +CIPAP:ip,"192.168.0.114"\r\n
	//                   +CIPAP:gateway,"192.168.1.1"\r\n
	//					 +CIPAP:subnet,"255.255.255.0"\r\n
	//                   \r\n
	//                   OK\r\n
	// Look for the OK:
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rsp > 0)
	{
		// Look for "ip" in the esp8266RxBuffer
		char * p = strstr(esp8266RxBuffer, "ip");
		
		if (p != NULL)
		{
			IPAddress returnIP;
			
			p += 4; // Move p four places. (skip "ip,")
			for (uint8_t i = 0; i < 4; i++)
			{
				char tempOctet[4];
				memset(tempOctet, 0, 4); // Clear tempOctet
				
				size_t octetLength = strspn(p, "0123456789"); // Find length of numerical string:
				if (octetLength >= 4) // If it's too big, return an error
					return ESP8266_RSP_UNKNOWN;
				
				strncpy(tempOctet, p, octetLength); // Copy string to temp char array:
				returnIP[i] = atoi(tempOctet); // Move the temp char into IP Address octet
				
				p += (octetLength + 1); // Increment p to next octet
			}
			
			return returnIP;
		}
	}
	
	return rsp;
}

// status()
// Input: none
// Output:
//    - Success: 2, 3, 4, or 5 (ESP8266_STATUS_GOTIP, ESP8266_STATUS_CONNECTED, ESP8266_STATUS_DISCONNECTED, ESP8266_STATUS_NOWIFI)
//    - Fail: <0 (esp8266_cmd_rsp)
int8_t ESP8266Class::status()
{
	int8_t statusRet = updateStatus();
	if (statusRet > 0) return _status;
	else return statusRet;
}

int8_t ESP8266Class::updateStatus()
{
	sendCommand(ESP8266_TCP_STATUS); // Send AT+CIPSTATUS\r\n
	// Example response: (connected as client)
	// STATUS:3\r\n
	// +CIPSTATUS:0,"TCP","93.184.216.34",80,0\r\n\r\nOK\r\n 
	// - or - (clients connected to ESP8266 server)
	// STATUS:3\r\n
	// +CIPSTATUS:0,"TCP","192.168.0.100",54723,1\r\n
	// +CIPSTATUS:1,"TCP","192.168.0.101",54724,1\r\n\r\nOK\r\n 
	
	String s = wifi.readString();
	//Serial.println(s);//add debug line
	if(strstr(s.c_str(),RESPONSE_OK)==NULL) return ESP8266_RSP_FAIL;
	
	char * p = strstr(s.c_str(),"STATUS:");
	if (p == NULL)
			return ESP8266_RSP_UNKNOWN;
	p += 7; //"STATUS:"
	_status = *p - 48;
	
	for (int i=0; i<ESP8266_MAX_SOCK_NUM; i++)
	{
		char params[15];
		//char params[15] = {"+CIPSTATUS:"};
		char *p;
		//params[11] = i + '0';
		//params[12] = '\0';
		sprintf(params, "+CIPSTATUS:%d", i);
		if(p=strstr(s.c_str(),(const char *)params))
		{
			_state[i] = TAKEN; 
			p+=20;		
			IPAddress currentIP;
			for (uint8_t j = 0; j < 4; j++)
			{
				char tempOctet[4];
				memset(tempOctet, 0, 4); // Clear tempOctet
				
				size_t octetLength = strspn(p, "0123456789"); // Find length of numerical string:
				if (octetLength >= 4) // If it's too big, return an error
				{
					_client[i]={255, 255, 255, 255};
					return 1;
				}	

				strncpy(tempOctet, p, octetLength); // Copy string to temp char array:
				currentIP[j] = atoi(tempOctet); // Move the temp char into IP Address octet
				
				p += (octetLength + 1); // Increment p to next octet
			}
			_client[i] = currentIP;
		}			

		else
		{
			_state[i] = AVAILABLE;
			_client[i] = {0, 0, 0, 0};
		}

	}

	return 1;	
}

/////////////////////
// TCP/IP Commands //
/////////////////////

bool ESP8266Class::tcpConnect(uint8_t linkID, const char * destination, uint16_t port, uint16_t keepAlive)
{
	int i = 10;
	while(i--)
	{
	//_serial->print((const __FlashStringHelper*)ESP8266_AT);
	_serial->write(0x41);_serial->write(0x54);
	_serial->print((const __FlashStringHelper*)ESP8266_TCP_CONNECT);
	//_serial->print(F("="));
	_serial->write(0x3d);
	_serial->print(linkID);
	//_serial->print(F(",\"TCP\",\""));
	_serial->write(0x2c);_serial->write(0x22);_serial->write(0x54);_serial->write(0x43);_serial->write(0x50);_serial->write(0x22);_serial->write(0x2c);_serial->write(0x22);
	_serial->print(destination);
	//_serial->print(F("\","));
	_serial->write(0x22);_serial->write(0x2c);
	_serial->print(port);
	if (keepAlive > 0)
	{
		//_serial->print(",");
		_serial->write(0x2c);
		// keepAlive is in units of 500 milliseconds.
		// Max is 7200 * 500 = 3600000 ms = 60 minutes.
		_serial->print(keepAlive / 500);
	}
	//_serial->print(F("\r\n"));
	_serial->write(0x0d);
	_serial->write(0x0a);
	
	// Example good: CONNECT\r\n\r\nOK\r\n
	// Example bad: DNS Fail\r\n\r\nERROR\r\n
	// Example meh: ALREADY CONNECTED\r\n\r\nERROR\r\n
	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, CLIENT_CONNECT_TIMEOUT);
	
	if (rsp <= 0)
	{
		// We may see "ERROR", but be "ALREADY CONNECTED".
		// Search for "ALREADY", and return success if we see it.
		char * p = searchBuffer("ALREADY");
		if (p != NULL)
			return true;
		// Otherwise the connection failed. Continue the trials or return false:
	}
	// Return true on successful (new) connection
	else 
		return true;
	delay(1);
	}
	return false;
}

bool ESP8266Class::setSslBufferSize(size_t size)
{
	if(size < 2048 || size > 4096) return false;	
	char modeChar[5] = {0};
	sprintf(modeChar, "%d", size);

	sendCommand(ESP8266_SSL_SIZE, ESP8266_CMD_SETUP, (const char*)modeChar);

	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT)> 0 ? true : false );
}

bool ESP8266Class::sslConnect(uint8_t linkID, const char * destination, uint16_t port, uint16_t keepAlive)
{
	
	if(!setSslBufferSize(4096)) return false;
	
	int i = 10;
	while(i--)
	{
	//_serial->print((const __FlashStringHelper*)ESP8266_AT);
	_serial->write(0x41);_serial->write(0x54);
	_serial->print((const __FlashStringHelper*)ESP8266_TCP_CONNECT);
	//_serial->print(F("="));
	_serial->write(0x3d);
	_serial->print(linkID);
	//_serial->print(F(",\"TCP\",\""));
	_serial->write(0x2c);_serial->write(0x22);_serial->write(0x53);_serial->write(0x53);_serial->write(0x4c);_serial->write(0x22);_serial->write(0x2c);_serial->write(0x22);
	_serial->print(destination);
	//_serial->print(F("\","));
	_serial->write(0x22);_serial->write(0x2c);
	_serial->print(port);
	if (keepAlive > 0)
	{
		//_serial->print(",");
		_serial->write(0x2c);
		// keepAlive is in units of 500 milliseconds.
		// Max is 7200 * 500 = 3600000 ms = 60 minutes.
		_serial->print(keepAlive / 500);
	}
	//_serial->print(F("\r\n"));
	_serial->write(0x0d);
	_serial->write(0x0a);
	
	// Example good: CONNECT\r\n\r\nOK\r\n
	// Example bad: DNS Fail\r\n\r\nERROR\r\n
	// Example meh: ALREADY CONNECTED\r\n\r\nERROR\r\n
	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, CLIENT_CONNECT_TIMEOUT);
	
	if (rsp <= 0)
	{
		// We may see "ERROR", but be "ALREADY CONNECTED".
		// Search for "ALREADY", and return success if we see it.
		char * p = searchBuffer("ALREADY");
		if (p != NULL)
			return true;
		// Otherwise the connection failed. Continue the trials or return false:
	}
	// Return true on successful (new) connection
	else 
		return true;
	delay(1);
	}
	return false;
}

int16_t ESP8266Class::tcpSend(uint8_t linkID, const uint8_t *buf, size_t buf_size)
{
	
	if (buf_size > 2048)
		return ESP8266_CMD_BAD;
	
	int16_t rsp;
	char params[8] = {0};
	sprintf(params, "%d,%d", linkID, buf_size);
	
	sendCommand(ESP8266_TCP_SEND, ESP8266_CMD_SETUP, params);

	rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT);
	if (rsp > 0)
	{
		_serial->print((const char*)buf);
		
		rsp = readForResponses(RESPONSE_OK, RESPONSE_FAIL, COMMAND_RESPONSE_TIMEOUT);
		
		if (rsp > 0)
			return buf_size;
		
	}
	return rsp;
}

bool ESP8266Class::close(uint8_t linkID)
{
	char params[2] = {0};
	sprintf(params, "%d", linkID);
	sendCommand(ESP8266_TCP_CLOSE, ESP8266_CMD_SETUP, params);
	
	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT);

	if (rsp <= 0)
	{
		// We may see "ERROR", but be "UNLINK".
		// Search for "UNLINK", and return success if we see it.
		if (searchBuffer("UNLINK")!= NULL)
			return true;
		// Otherwise return error
		else
			return false;
	}
	// Return true on closing connection
	return true;
}

//int16_t ESP8266Class::setTransferMode(uint8_t mode)
//{
//	char params[2] = {0, 0};
//	params[0] = (mode > 0) ? '1' : '0';
//	sendCommand(ESP8266_TRANSMISSION_MODE, ESP8266_CMD_SETUP, params);
//	
//	return readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
//}

bool ESP8266Class::setMux(bool enable)
{
	sendCommand(ESP8266_TCP_MULTIPLE, ESP8266_CMD_SETUP, (enable==true) ? "1" : "0");
	
	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT)>0? true:false );
}

bool ESP8266Class::configureTCPServer(uint8_t create, uint16_t port)
{
	if(!setMux(true)) return false;
	char params[10] = {0};
	if (create >= 1) 
		sprintf(params, "1,%d", port);
	else

		sprintf(params, "0", port);
	
	sendCommand(ESP8266_SERVER_CONFIG, ESP8266_CMD_SETUP, params);
	
	if(readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT)<=0) return false;
	
	return setServerTimeout(SERVER_TIMEOUT);
}

bool ESP8266Class::setServerTimeout(uint16_t time)
{
	char params[10] = {0};
	time = constrain(time, 0, 7200);
	sprintf(params, "%d", time);
	
	sendCommand(ESP8266_SET_SERVER_TIMEOUT, ESP8266_CMD_SETUP, params);
	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT)>0? true:false);
}
/*
int16_t ESP8266Class::ping(IPAddress ip)
{
	char ipStr[17];
	sprintf(ipStr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
	return ping(ipStr);
}

int16_t ESP8266Class::ping(char * server)
{
	char params[strlen(server) + 3];
	sprintf(params, "\"%s\"", server);
	// Send AT+Ping=<server>
	sendCommand(ESP8266_PING, ESP8266_CMD_SETUP, params); 
	// Example responses:
	//  * Good response: +12\r\n\r\nOK\r\n
	//  * Timeout response: +timeout\r\n\r\nERROR\r\n
	//  * Error response (unreachable): ERROR\r\n\r\n
	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_PING_TIMEOUT);
	if (rsp > 0)
	{
		char * p = searchBuffer("+");
		p += 1; // Move p forward 1 space
		char * q = strchr(p, '\r'); // Find the first \r
		if (q == NULL)
			return ESP8266_RSP_UNKNOWN;
		char tempRsp[10];
		strncpy(tempRsp, p, q - p);
		return atoi(tempRsp);
	}
	else
	{
		if (searchBuffer("timeout") != NULL)
			return 0;
	}
	
	return rsp;
}
*/
//////////////////////////
// Custom GPIO Commands //
//////////////////////////

bool ESP8266Class::digitalWrite(uint8_t pin, uint8_t state)
{
	if(pin > 16) return false;
	char params[5] = {0}; 
	sprintf(params, "%d,%d", pin, state>0? 1:0);
	sendCommand(ESP8266_PINWRITE, ESP8266_CMD_SETUP, params);
	return (readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT)>0? true: false);
}

int8_t ESP8266Class::digitalRead(uint8_t pin)
{
	if(pin > 16) return -1;
	char params[5] = {0}; 
	sprintf(params, "%d", pin);
	sendCommand(ESP8266_PINREAD, ESP8266_CMD_SETUP, params);
	// Example response: 0:LOW\r\n\r\nOK\r\n
	
	if (readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT) > 0)
	{
		if (strstr(esp8266RxBuffer, "LOW") != NULL)
			return LOW;
		else if (strstr(esp8266RxBuffer, "HIGH") != NULL)
			return HIGH;
	}
	
	return -1;
}

//////////////////////////////
// Stream Virtual Functions //
//////////////////////////////

size_t ESP8266Class::write(uint8_t c)
{
	_serial->write(c);
}

int ESP8266Class::available()
{
	return _serial->available();
}

int ESP8266Class::read()
{
	return _serial->read();
}

int ESP8266Class::peek()
{
	return _serial->peek();
}

void ESP8266Class::flush()
{
	_serial->readString();
}

/*bool ESP8266Class::find(char *target)
{
	return _serial->find(target);
}

bool ESP8266Class::find(uint8_t *target)
{
	return _serial->find(target);
}*/

//////////////////////////////////////////////////
// Private, Low-Level, Ugly, Hardware Functions //
//////////////////////////////////////////////////

void ESP8266Class::sendCommand(const char * cmd, uint8_t type, const char * params)
{
	//if got any data streaming in, flush the data
	//if(_serial->available()>0) _serial->readString();
	
	//_serial->print((const __FlashStringHelper*)ESP8266_AT);
	if(!isHardwareSerial) {
		swSerial->listen();
	}
	
	_serial->write(0x41);_serial->write(0x54);
	_serial->print((const __FlashStringHelper*)cmd);
	if (type == ESP8266_CMD_QUERY)
		//_serial->print(F("?"));
		_serial->write(0x3f);
	else if (type == ESP8266_CMD_SETUP)
	{
		//_serial->print(F("="));
		_serial->write(0x3d);
		_serial->print(params);		
	}
	//_serial->print(F("\r\n"));
	_serial->write(0x0d);
	_serial->write(0x0a);
}

int16_t ESP8266Class::readForResponse(const char * rsp, unsigned int timeout)
{
	unsigned int received = 0; // received keeps track of number of chars read
	
	clearBuffer();
	
	while (timeout--) // While we haven't timed out
	{
		if (_serial->available()) // If data is available on UART RX
		{
			received += readByteToBuffer();
			
			if (searchBuffer(rsp))	// Search the buffer for goodRsp
				return received;	// Return how number of chars read
				
		}
		delay(1);
	}
	
	if (received > 0) // If we received any characters
		return ESP8266_RSP_UNKNOWN; // Return unkown response error code
	else // If we haven't received any characters
		return ESP8266_RSP_TIMEOUT; // Return the timeout error code
}

int16_t ESP8266Class::readForResponses(const char * pass, const char * fail, unsigned int timeout)
{
	unsigned int received = 0; // received keeps track of number of chars read
	
	clearBuffer();
	
	while (timeout--) // While we haven't timed out
	{
		if (_serial->available()) // If data is available on UART RX
		{
			received += readByteToBuffer();
			
			if (searchBuffer(pass))	// Search the buffer for goodRsp
				return received;	// Return how number of chars read
				
			if (searchBuffer(fail))	// Search the buffer for badRsp
				return ESP8266_RSP_FAIL;// return error
				
		}
		delay(1);
	}
	
	if (received > 0) // If we received any characters
		return ESP8266_RSP_UNKNOWN; // Return unkown response error code
	else // If we haven't received any characters
		return ESP8266_RSP_TIMEOUT; // Return the timeout error code
}

//////////////////
// Buffer Stuff //
//////////////////

void ESP8266Class::clearBuffer()
{
	memset(esp8266RxBuffer, '\0', ESP8266_RX_BUFFER_LEN);
	bufferHead = 0;
}	

unsigned int ESP8266Class::readByteToBuffer()
{
	// Read the data in
	char c = _serial->read();
	
	// Do not store the data if data is null character	
	if(c <= 0) return 0;
	
	// Store the data in the buffer
	esp8266RxBuffer[bufferHead] = c;

	//! TODO: Don't care if we overflow. Should we? Set a flag or something?
	bufferHead = (bufferHead + 1) % ESP8266_RX_BUFFER_LEN;
	
	return 1;
}

char * ESP8266Class::searchBuffer(const char * test)
{
	return strstr((const char *)esp8266RxBuffer, test);
}
