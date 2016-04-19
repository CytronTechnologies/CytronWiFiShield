/******************************************************************************
ESP8266_AT.h
ESP8266 AT Command Definitions
Created by Ng Beng Chet @ Cytron Technologies Sdn Bhd 
Original Creation Date: Sept 11, 2015
https://github.com/CytronTechnologies/CytronWiFiShield

Modified from ESP8266 AT Command Definitions File
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
#ifndef _ESP8266AT_H_
#define _ESP8266AT_H_

//////////////////////
// Common Responses //
//////////////////////
const char RESPONSE_OK[] = "OK\r\n";
const char RESPONSE_ERROR[] = "ERROR\r\n";
const char RESPONSE_FAIL[] = "FAIL\r\n";

///////////////////////
// Basic AT Commands //
///////////////////////
//const char ESP8266_AT[] PROGMEM = "AT";
const char ESP8266_TEST[] PROGMEM= "";	// Test AT startup
const char ESP8266_RESET[] PROGMEM= "+RST"; // Restart module
const char ESP8266_VERSION[] PROGMEM= "+GMR"; // View version info
//!const char ESP8266_SLEEP[] = "+GSLP"; // Enter deep-sleep mode
const char ESP8266_ECHO_ENABLE[] PROGMEM= "E1"; // AT commands echo
const char ESP8266_ECHO_DISABLE[] PROGMEM= "E0"; // AT commands echo
const char ESP8266_SHOW_INFO[] PROGMEM= "+CIPDINFO";
//!const char ESP8266_RESTORE[] = "+RESTORE"; // Factory reset
//!const char ESP8266_UART[] = "+UART"; // UART configuration

////////////////////
// WiFi Functions //
////////////////////
const char ESP8266_WIFI_MODE[] PROGMEM= "+CWMODE"; // WiFi mode (sta/AP/sta+AP)
const char ESP8266_CONNECT_AP[] PROGMEM= "+CWJAP"; // Connect to AP
//!const char ESP8266_LIST_AP[] = "+CWLAP"; // List available AP's
const char ESP8266_DISCONNECT[] PROGMEM= "+CWQAP"; // Disconnect from AP
const char ESP8266_AP_CONFIG[] PROGMEM= "+CWSAP"; // Set softAP configuration
//!const char ESP8266_STATION_IP[] = "+CWLIF"; // List station IP's connected to softAP
//!const char ESP8266_DHCP_EN[] = "+CWDHCP"; // Enable/disable DHCP
const char ESP8266_AUTO_CONNECT[] PROGMEM= "+CWAUTOCONN"; // Connect to AP automatically
//!const char ESP8266_SET_STA_MAC[] = "+CIPSTAMAC"; // Set MAC address of station
//!const char ESP8266_SET_AP_MAC[] = "+CIPAPMAC"; // Set MAC address of softAP
const char ESP8266_SET_STA_IP[] PROGMEM= "+CIPSTA_CUR"; // Set IP address of ESP8266 station
const char ESP8266_SET_AP_IP[] PROGMEM= "+CIPAP_CUR"; // Set IP address of ESP8266 softAP

/////////////////////
// TCP/IP Commands //
/////////////////////
const char ESP8266_SSL_SIZE[] PROGMEM= "+CIPSSLSIZE"; // Set SSL buffer size
const char ESP8266_TCP_STATUS[] PROGMEM= "+CIPSTATUS"; // Get connection status
const char ESP8266_TCP_CONNECT[] PROGMEM= "+CIPSTART"; // Establish TCP connection or SSL connection or register UDP port
const char ESP8266_TCP_SEND[] PROGMEM= "+CIPSEND"; // Send Data
const char ESP8266_TCP_CLOSE[] PROGMEM= "+CIPCLOSE"; // Close TCP/UDP connection
//!const char ESP8266_GET_LOCAL_IP[] = "+CIFSR"; // Get local IP address
const char ESP8266_TCP_MULTIPLE[] PROGMEM= "+CIPMUX"; // Set multiple connections mode
const char ESP8266_SERVER_CONFIG[] PROGMEM= "+CIPSERVER"; // Configure as server
//!const char ESP8266_TRANSMISSION_MODE[] = "+CIPMODE"; // Set transmission mode
const char ESP8266_SET_SERVER_TIMEOUT[] PROGMEM= "+CIPSTO"; // Set timeout when ESP8266 runs as TCP server
//!const char ESP8266_PING[] = "+PING"; // Function PING

//////////////////////////
// Custom GPIO Commands //
//////////////////////////
const char ESP8266_PINWRITE[] PROGMEM= "+CIOWRITE"; // Write GPIO (high/low)
const char ESP8266_PINREAD[] PROGMEM= "+CIOREAD"; // Read GPIO digital value

#endif
