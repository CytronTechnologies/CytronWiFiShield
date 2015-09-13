/******************************************************************************
CytronWiFiShield.h
Cytron WiFi Shield Library Main Header File
Created by Ng Beng Chet @ Cytron Technologies Sdn Bhd 
Original Creation Date: Sept 11, 2015
https://github.com/CytronTechnologies/CytronWiFiShield

Modified from ESP8266 WiFi Shield Library Main Header File
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

#ifndef _CYTRONWIFISHIELD_H_
#define _CYTRONWIFISHIELD_H_

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <IPAddress.h>
#include "CytronWiFiClient.h"
#include "CytronWiFiServer.h"

///////////////////////////////
// Command Response Timeouts //
///////////////////////////////
#define COMMAND_RESPONSE_TIMEOUT 5000
#define COMMAND_PING_TIMEOUT 3000
#define WIFI_CONNECT_TIMEOUT 30000
#define COMMAND_RESET_TIMEOUT 5000
#define CLIENT_CONNECT_TIMEOUT 5000

#define ESP8266_MAX_SOCK_NUM 5
#define ESP8266_SOCK_NOT_AVAIL 255
#define ESP8266_RX_BUFFER_LEN 128

typedef enum esp8266_cmd_rsp {
	ESP8266_CMD_BAD = -5,
	ESP8266_RSP_MEMORY_ERR = -4,
	ESP8266_RSP_FAIL = -3,
	ESP8266_RSP_UNKNOWN = -2,
	ESP8266_RSP_TIMEOUT = -1,
	ESP8266_RSP_SUCCESS = 0
};

typedef enum esp8266_wifi_mode {
	WIFI_STA = 1,
	WIFI_AP = 2,
	WIFI_BOTH = 3
};

typedef enum esp8266_command_type {
	ESP8266_CMD_QUERY,
	ESP8266_CMD_SETUP,
	ESP8266_CMD_EXECUTE
};
/*
typedef enum esp8266_encryption {
	ESP8266_ECN_OPEN,
	ESP8266_ECN_WPA_PSK,
	ESP8266_ECN_WPA2_PSK,
	ESP8266_ECN_WPA_WPA2_PSK
};
*/
typedef enum esp8266_connect_status {
	ESP8266_STATUS_GOTIP = 2,
	ESP8266_STATUS_CONNECTED = 3,
	ESP8266_STATUS_DISCONNECTED = 4,
	ESP8266_STATUS_NOWIFI = 5	
};

typedef enum esp8266_socket_state {
	AVAILABLE = 0,
	TAKEN = 1,
};

typedef enum esp8266_connection_type {
	ESP8266_TCP,
	ESP8266_UDP,
	ESP8266_TYPE_UNDEFINED
};

typedef enum esp8266_tetype {
	ESP8266_CLIENT,
	ESP8266_SERVER
};

struct esp8266_ipstatus
{
	uint8_t linkID;
	esp8266_connection_type type;
	IPAddress remoteIP;
	uint16_t port;
	esp8266_tetype tetype;
};

struct esp8266_status
{
	esp8266_connect_status stat;
	esp8266_ipstatus ipstatus[ESP8266_MAX_SOCK_NUM];
};

class ESP8266Class : public Stream
{
public:
	ESP8266Class();
	
	bool begin(uint8_t rx_pin=2, uint8_t tx_pin=3);
	
	///////////////////////
	// Basic AT Commands //
	///////////////////////
	bool test();
	bool reset();
	String getVersion();
	bool echo(bool enable);

	////////////////////
	// WiFi Functions //
	////////////////////
	int8_t getMode();
	bool setMode(esp8266_wifi_mode mode);
	bool connectAP(const char * ssid);
	bool connectAP(const char * ssid, const char * pwd);
	bool disconnectAP();
	String getAP();
	IPAddress localIP();
	bool config(IPAddress ip, IPAddress gateway = {192, 168, 1, 1}, IPAddress subnet = {255, 255,255, 0});
	IPAddress softAPIP();
	int8_t status();
	int8_t updateStatus();
	
	/////////////////////
	// TCP/IP Commands //
	/////////////////////
	bool tcpConnect(uint8_t linkID, const char * destination, uint16_t port, uint16_t keepAlive=0);
	int16_t tcpSend(uint8_t linkID, const uint8_t *buf, size_t buf_size);
	bool close(uint8_t linkID);
	//int16_t setTransferMode(uint8_t mode);
	bool setMux(bool enable);
	bool configureTCPServer(uint8_t create = 1, uint16_t port = 80);
	bool setServerTimeout(uint16_t time);
	//int16_t ping(IPAddress ip);
	//int16_t ping(char * server);
		
	//////////////////////////
	// Custom GPIO Commands //
	//////////////////////////
	//int16_t pinMode(uint8_t pin, uint8_t mode);
	bool digitalWrite(uint8_t pin, uint8_t state);
	int8_t digitalRead(uint8_t pin);
	
	///////////////////////////////////
	// Virtual Functions from Stream //
	///////////////////////////////////
	size_t write(uint8_t);
	int available();
	int read();
	int peek();
	void flush();
	
	friend class ESP8266Client;
	friend class ESP8266Server;

    bool _state[ESP8266_MAX_SOCK_NUM];
	char esp8266RxBuffer[ESP8266_RX_BUFFER_LEN];
	
protected:
    Stream* _serial;
	
private:
	//////////////////////////
	// Command Send/Receive //
	//////////////////////////
	void sendCommand(const char * cmd, enum esp8266_command_type type = ESP8266_CMD_EXECUTE, const char * params = NULL);
	int16_t readForResponse(const char * rsp, unsigned int timeout);
	int16_t readForResponses(const char * pass, const char * fail, unsigned int timeout);
	
	//////////////////
	// Buffer Stuff // 
	//////////////////
	/// clearBuffer() - Reset buffer pointer, set all values to 0
	void clearBuffer();
	
	/// readByteToBuffer() - Read first byte from UART receive buffer
	/// and store it in rxBuffer.
	unsigned int readByteToBuffer();
	
	/// searchBuffer([test]) - Search buffer for string [test]
	/// Success: Returns pointer to beginning of string
	/// Fail: returns NULL
	//! TODO: Fix this function so it searches circularly
	char * searchBuffer(const char * test);
	
	esp8266_status _status;
	bool isHardwareSerial;
};

extern ESP8266Class wifi;

#endif