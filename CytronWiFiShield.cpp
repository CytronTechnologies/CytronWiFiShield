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

#include <Arduino.h>
#include "CytronWiFiShield.h"
#include "util/ESP8266_AT.h"

#define ESP8266_DISABLE_ECHO 1
#define baudRate 9600

////////////////////////
// Buffer Definitions //
////////////////////////
//#define ESP8266_RX_BUFFER_LEN 128 // Number of bytes in the serial receive buffer
//char esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
unsigned int bufferHead; // Holds position of latest byte placed in buffer.
////////////////////
// Initialization //
////////////////////

ESP8266Class wifi;

ESP8266Class::ESP8266Class()
{
	for (int i=0; i<ESP8266_MAX_SOCK_NUM; i++)
		_state[i] = AVAILABLE;
}

bool ESP8266Class::begin(uint8_t rx_pin, uint8_t tx_pin)
{
	//_baud = baudRate;
	if (rx_pin==2&&tx_pin==3 || rx_pin==8&&tx_pin==9)
	{
		SoftwareSerial *swSerial = new SoftwareSerial(rx_pin, tx_pin);
		swSerial->begin(baudRate);
		_serial = swSerial;
		isHardwareSerial = false;
	}
	else if (rx_pin==0&&tx_pin==1)
	{
		Serial.begin(baudRate);
		_serial = &Serial;
		isHardwareSerial = true;
	}
	
	if (test())
	{
		//if (!setTransferMode(0))
		//	return false;
#ifdef ESP8266_DISABLE_ECHO
		if (!echo(false))
			return false;
#endif
		if (!setMux(true))
			return false;
		
		//TODO: list to do if successfully talking to esp8266
		if(!setMode(WIFI_STA))
			return false;
		
		return true;
	}
	
	return false;
}

///////////////////////
// Basic AT Commands //
///////////////////////

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
	
	if (!_serial->find("ready\r\n"))
	//if (readForResponse(RESPONSE_READY, COMMAND_RESET_TIMEOUT) <= 0)
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

String ESP8266Class::getVersion()
{
	sendCommand(ESP8266_VERSION); // Send AT+GMR
	// Example Response: AT version:0.30.0.0(Jul  3 2015 19:35:49)\r\n (43 chars)
	//                   SDK version:1.2.0\r\n (19 chars)
	//                   compile time:Jul  7 2015 18:34:26\r\n (36 chars)
	//                   OK\r\n
	// (~101 characters)
	// Look for "OK":

	if (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0)
	{
		
		char * p = strstr((const char*)esp8266RxBuffer, "AT version:");
		char * q = strstr((const char*)esp8266RxBuffer, "\r\nOK\r\n");
		if(p==NULL||q==NULL) return "response unknown";
		char version[150]={0};
		strncpy(version, p, q-p);
		return (String)version;
	}
	
	return "Version unknown";
}

////////////////////
// WiFi Functions //
////////////////////

// getMode()
// Input: None
// Output:
//    - Success: 1, 2, 3 (ESP8266_MODE_STA, ESP8266_MODE_AP, ESP8266_MODE_STAAP)
//    - Fail: <0 (esp8266_cmd_rsp)
int8_t ESP8266Class::getMode()
{
	sendCommand(ESP8266_WIFI_MODE, ESP8266_CMD_QUERY);
	
	// Example response: \r\nAT+CWMODE_CUR?\r+CWMODE_CUR:2\r\n\r\nOK\r\n
	// Look for the OK:
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rsp > 0)
	{
		// Then get the number after ':':
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
// Input: 1, 2, 3 (ESP8266_MODE_STA, ESP8266_MODE_AP, ESP8266_MODE_STAAP)
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
	_serial->print("AT");
	_serial->print(ESP8266_CONNECT_AP);
	_serial->print("=\"");
	_serial->print(ssid);
	_serial->print("\"");
	if (pwd != NULL)
	{
		_serial->print(',');
		_serial->print("\"");
		_serial->print(pwd);
		_serial->print("\"");
	}
	_serial->print("\r\n");
	
	return (readForResponses(RESPONSE_OK, RESPONSE_FAIL, WIFI_CONNECT_TIMEOUT) > 0 ? true : false );
}

String ESP8266Class::getAP()
{
	sendCommand(ESP8266_CONNECT_AP, ESP8266_CMD_QUERY); // Send "AT+CWJAP?"
	
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	// Example Responses: No AP\r\n\r\nOK\r\n
	// - or -
	// +CWJAP:"WiFiSSID","00:aa:bb:cc:dd:ee",6,-45\r\n\r\nOK\r\n
	if (rsp > 0)
	{
		// Look for "No AP"
		if (strstr(esp8266RxBuffer, "No AP") != NULL)
			return "No AP";
		
		// Look for "+CWJAP"
		char * p = strstr(esp8266RxBuffer, "+CWJAP:");
		if (p != NULL)
		{
			p += strlen(ESP8266_CONNECT_AP) + 2;
			char * q = strchr(p, '"');
			if (q == NULL) return "AP unknown";
			char ssid[50]={0};
			strncpy(ssid, p, q-p); // Copy string to temp char array:
			return (String)ssid;
		}
	}
	
	return "Response error";
}

bool ESP8266Class::disconnectAP()
{
	sendCommand(ESP8266_DISCONNECT); // Send AT+CWQAP
	// Example response: \r\n\r\nOK\r\nWIFI DISCONNECT\r\n
	// "WIFI DISCONNECT" comes up to 500ms _after_ OK. 
	//int16_t rsp = 
	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0 ? true : false);
	/*if (rsp > 0)
	{
		rsp = readForResponse("WIFI DISCONNECT", COMMAND_RESPONSE_TIMEOUT);
		if (rsp > 0)
			return rsp;
		return 1;
	}
	
	return rsp;*/
}

// localIP()
// Input: none
// Output:
//    - Success: Device's local IPAddress
//    - Fail: 0
IPAddress ESP8266Class::localIP()
{
	//sendCommand(ESP8266_GET_LOCAL_IP); // Send AT+CIFSR\r\n
	sendCommand(ESP8266_SET_STA_IP, ESP8266_CMD_QUERY);
	// Example Response: +CIFSR:STAIP,"192.168.0.114"\r\n
	//                   +CIFSR:STAMAC,"18:fe:34:9d:b7:d9"\r\n
	//                   \r\n
	//                   OK\r\n
	// Look for the OK:
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rsp > 0)
	{
		// Look for "STAIP" in the rxBuffer
		//char * p = strstr(esp8266RxBuffer, "STAIP");
		char * p = strstr(esp8266RxBuffer, "ip");
		
		if (p != NULL)
		{
			IPAddress returnIP;
			
			p += 4; // Move p seven places. (skip STAIP,")
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
	_serial->print("AT");
	_serial->print(ESP8266_SET_STA_IP);
	_serial->print("=\"");
	_serial->print(ip);
	_serial->print("\",\"");
	_serial->print(gateway);
	_serial->print("\",\"");
	_serial->print(subnet);
	_serial->print("\"\r\n");
	
	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT) > 0 ? true : false);
	
}

IPAddress ESP8266Class::softAPIP()
{
	//sendCommand(ESP8266_GET_LOCAL_IP); // Send AT+CIFSR\r\n
	sendCommand(ESP8266_SET_AP_IP, ESP8266_CMD_QUERY);
	// Example Response: +CIFSR:STAIP,"192.168.0.114"\r\n
	//                   +CIFSR:STAMAC,"18:fe:34:9d:b7:d9"\r\n
	//                   \r\n
	//                   OK\r\n
	// Look for the OK:
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rsp > 0)
	{
		// Look for "STAIP" in the rxBuffer
		//char * p = strstr(esp8266RxBuffer, "STAIP");
		char * p = strstr(esp8266RxBuffer, "ip");
		
		if (p != NULL)
		{
			IPAddress returnIP;
			
			p += 4; // Move p seven places. (skip STAIP,")
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
	int16_t statusRet = updateStatus();
	if (statusRet > 0)
	{
		switch (_status.stat)
		{
		case ESP8266_STATUS_GOTIP: // 2
		case ESP8266_STATUS_DISCONNECTED: // 4 - "Client" disconnected, not wifi
			return 1;
			break;
		case ESP8266_STATUS_CONNECTED: // 3 - Connected, but haven't gotten an IP
		case ESP8266_STATUS_NOWIFI: // 5 - No WiFi configured
			return 0;
			break;
		}
	}
	return statusRet;
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
	
	int16_t rsp = readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT);
	if (rsp > 0)
	{
		char * p = searchBuffer("STATUS:");
		if (p == NULL)
			return ESP8266_RSP_UNKNOWN;
		
		p += strlen("STATUS:");
		_status.stat = (esp8266_connect_status)(*p - 48);
		
		for (int i=0; i<ESP8266_MAX_SOCK_NUM; i++)
		{
			p = strstr(p, "+CIPSTATUS:");
			if (p == NULL)
			{
				// Didn't find any IPSTATUS'. Set linkID to 255.
				for (int j=i; j<ESP8266_MAX_SOCK_NUM; j++)
					_status.ipstatus[j].linkID = 255;
				return rsp;
			}
			else
			{
				p += strlen("+CIPSTATUS:");
				// Find linkID:
				uint8_t linkId = *p - 48;
				if (linkId >= ESP8266_MAX_SOCK_NUM)
					return rsp;
				_status.ipstatus[linkId].linkID = linkId;
				
				// Find type (p pointing at linkID):
				p += 3; // Move p to either "T" or "U"
				if (*p == 'T')
					_status.ipstatus[linkId].type = ESP8266_TCP;
				else if (*p == 'U')
					_status.ipstatus[linkId].type = ESP8266_UDP;
				else
					_status.ipstatus[linkId].type = ESP8266_TYPE_UNDEFINED;
				
				// Find remoteIP (p pointing at first letter or type):
				p += 6; // Move p to first digit of first octet.
				for (uint8_t j = 0; j < 4; j++)
				{
					char tempOctet[4];
					memset(tempOctet, 0, 4); // Clear tempOctet
					
					size_t octetLength = strspn(p, "0123456789"); // Find length of numerical string:
					
					strncpy(tempOctet, p, octetLength); // Copy string to temp char array:
					_status.ipstatus[linkId].remoteIP[j] = atoi(tempOctet); // Move the temp char into IP Address octet
					
					p += (octetLength + 1); // Increment p to next octet
				}
				
				// Find port (p pointing at ',' between IP and port:
				p += 1; // Move p to first digit of port
				char tempPort[6];
				memset(tempPort, 0, 6);
				size_t portLen = strspn(p, "0123456789"); // Find length of numerical string:
				strncpy(tempPort, p, portLen);
				_status.ipstatus[linkId].port = atoi(tempPort);
				p += portLen + 1;
				
				// Find tetype (p pointing at tetype)
				if (*p == '0')
					_status.ipstatus[linkId].tetype = ESP8266_CLIENT;
				else if (*p == '1')
					_status.ipstatus[linkId].tetype = ESP8266_SERVER;
			}
		}
	}
	
	return rsp;
}

/////////////////////
// TCP/IP Commands //
/////////////////////

bool ESP8266Class::tcpConnect(uint8_t linkID, const char * destination, uint16_t port, uint16_t keepAlive)
{
	if(!setMux(true)) return false;
	int i = 10;
	while(i--)
	{
	_serial->print("AT");
	_serial->print(ESP8266_TCP_CONNECT);
	_serial->print("=");
	_serial->print(linkID);
	_serial->print(",\"TCP\",\"");
	_serial->print(destination);
	_serial->print("\",");
	_serial->print(port);
	if (keepAlive > 0)
	{
		_serial->print(",");
		// keepAlive is in units of 500 milliseconds.
		// Max is 7200 * 500 = 3600000 ms = 60 minutes.
		_serial->print(keepAlive / 500);
	}
	_serial->print("\r\n");
	
	// Example good: CONNECT\r\n\r\nOK\r\n
	// Example bad: DNS Fail\r\n\r\nERROR\r\n
	// Example meh: ALREADY CONNECTED\r\n\r\nERROR\r\n
	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, CLIENT_CONNECT_TIMEOUT);
	//int16_t rsp = readForResponse(RESPONSE_OK, CLIENT_CONNECT_TIMEOUT);
	if (rsp <= 0)
	{
		// We may see "ERROR", but be "ALREADY CONNECTED".
		// Search for "ALREADY", and return success if we see it.
		char * p = searchBuffer("ALREADY");
		if (p != NULL)
			return true;
		// Otherwise the connection failed. Return the error code:
	}
	// Return 1 on successful (new) connection
	else 
		return true;
	}
	return false;
}

int16_t ESP8266Class::tcpSend(uint8_t linkID, const uint8_t *buf, size_t buf_size)
{
	
	if (buf_size > 2048)
		return ESP8266_CMD_BAD;
	
	char params[8];
	sprintf(params, "%d,%d", linkID, buf_size);
	sendCommand(ESP8266_TCP_SEND, ESP8266_CMD_SETUP, params);

	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT);
	if (rsp > 0)
	//if (rsp != ESP8266_RSP_FAIL)
	{
		_serial->print((const char*)buf);
		
		rsp = readForResponses("SEND OK\r\n", "SEND FAIL\r\n", COMMAND_RESPONSE_TIMEOUT);
		
		if (rsp > 0)
			return buf_size;
		
	}
	
	return rsp;
}

bool ESP8266Class::close(uint8_t linkID)
{
	char params[2];
	sprintf(params, "%d", linkID);
	sendCommand(ESP8266_TCP_CLOSE, ESP8266_CMD_SETUP, params);
	
	// Eh, client virtual function doesn't have a return value.
	// We'll wait for the OK or timeout anyway.
	int16_t rsp = readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT);
	
	if (rsp <= 0)
	{
		// We may see "ERROR", but be "UNLINK".
		// Search for "UNLINK", and return success if we see it.
		if (searchBuffer("UNLINK")!= NULL)
			return true;
		// Otherwise other error
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
	//char params[2] = {0, 0};
	sendCommand(ESP8266_TCP_MULTIPLE, ESP8266_CMD_SETUP, (enable==true) ? "1" : "0");
	
	return (readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT)>0? true:false );
}

bool ESP8266Class::configureTCPServer(uint8_t create, uint16_t port)
{
	if(!setMux(true)) return false;
	char params[10];
	if (create >= 1) 
		sprintf(params, "1,%d", port);
	else
		sprintf(params, "0");
	
	sendCommand(ESP8266_SERVER_CONFIG, ESP8266_CMD_SETUP, params);
	
	if(readForResponse(RESPONSE_OK, COMMAND_RESPONSE_TIMEOUT)<=0) return false;
	
	return setServerTimeout(10);
}

bool ESP8266Class::setServerTimeout(uint16_t time)
{
	char params[10];
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
/*
int16_t ESP8266Class::pinMode(uint8_t pin, uint8_t mode)
{
	char params[5];
	
	char modeC = 'i'; // Default mode to input
	if (mode == OUTPUT) 
		modeC = 'o'; // o = OUTPUT
	else if (mode == INPUT_PULLUP) 
		modeC = 'p'; // p = INPUT_PULLUP
	
	sprintf(params, "%d,%c", pin, modeC);
	sendCommand(ESP8266_PINMODE, ESP8266_CMD_SETUP, params);
	
	return readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT);
}
*/

bool ESP8266Class::digitalWrite(uint8_t pin, uint8_t state)
{
	_serial->print("AT");
	_serial->print(ESP8266_PINWRITE);
	_serial->print("=");
	_serial->print(pin);
	_serial->print(",");
	_serial->print(state>0?"1":"0");
	_serial->print("\r\n");
	
	return (readForResponses(RESPONSE_OK, RESPONSE_ERROR, COMMAND_RESPONSE_TIMEOUT)>0? true: false);
}

int8_t ESP8266Class::digitalRead(uint8_t pin)
{
	_serial->print("AT");
	_serial->print(ESP8266_PINREAD);
	_serial->print("=");
	_serial->print(pin);
	_serial->print("\r\n");
	// Example response: 1\r\n\r\nOK\r\n
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
	_serial->flush();
}

//////////////////////////////////////////////////
// Private, Low-Level, Ugly, Hardware Functions //
//////////////////////////////////////////////////

void ESP8266Class::sendCommand(const char * cmd, enum esp8266_command_type type, const char * params)
{
	_serial->print("AT");
	_serial->print(cmd);
	if (type == ESP8266_CMD_QUERY)
		_serial->print('?');
	else if (type == ESP8266_CMD_SETUP)
	{
		_serial->print("=");
		_serial->print(params);		
	}
	_serial->print("\r\n");
	//delay(50);
}

int16_t ESP8266Class::readForResponse(const char * rsp, unsigned int timeout)
{
	unsigned long timeIn = millis();	// Timestamp coming into function
	unsigned int received = 0; // received keeps track of number of chars read
	
	clearBuffer();	// Clear the class receive buffer (esp8266RxBuffer)
	while (timeIn + timeout > millis()) // While we haven't timed out
	{
		if (_serial->available()) // If data is available on UART RX
		{
			received += readByteToBuffer();
			if (searchBuffer(rsp))	// Search the buffer for goodRsp
				return received;	// Return how number of chars read
		}
	}
	
	if (received > 0) // If we received any characters
		return ESP8266_RSP_UNKNOWN; // Return unkown response error code
	else // If we haven't received any characters
		return ESP8266_RSP_TIMEOUT; // Return the timeout error code
}

int16_t ESP8266Class::readForResponses(const char * pass, const char * fail, unsigned int timeout)
{
	unsigned long timeIn = millis();	// Timestamp coming into function
	unsigned int received = 0; // received keeps track of number of chars read
	
	clearBuffer();	// Clear the class receive buffer (esp8266RxBuffer)
	while (timeIn + timeout > millis()) // While we haven't timed out
	{
		if (_serial->available()) // If data is available on UART RX
		{
			received += readByteToBuffer();
			if (searchBuffer(pass))	// Search the buffer for goodRsp
				return received;	// Return how number of chars read
			if (searchBuffer(fail))
				return ESP8266_RSP_FAIL;
		}
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
	
	// Store the data in the buffer
	esp8266RxBuffer[bufferHead] = c;
	//! TODO: Don't care if we overflow. Should we? Set a flag or something?
	bufferHead = (bufferHead + 1) % ESP8266_RX_BUFFER_LEN;
	
	return 1;
}

char * ESP8266Class::searchBuffer(const char * test)
{
	//int bufferLen = strlen((const char *)esp8266RxBuffer);
	// If our buffer isn't full, just do an strstr
	//if (bufferLen < ESP8266_RX_BUFFER_LEN)
		return strstr((const char *)esp8266RxBuffer, test);
	//else
	//{	//! TODO
		// If the buffer is full, we need to search from the end of the 
		// buffer back to the beginning.
	//	int testLen = strlen(test);
	//	for (int i=0; i<ESP8266_RX_BUFFER_LEN; i++)
	//	{
	//		
	//	}
	//}
}