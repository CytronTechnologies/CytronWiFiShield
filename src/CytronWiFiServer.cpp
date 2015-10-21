/******************************************************************************
CytronWiFiServer.cpp
Cytron WiFi Shield Library Server Source File
Created by Ng Beng Chet @ Cytron Technologies Sdn Bhd 
Original Creation Date: Sept 11, 2015
https://github.com/CytronTechnologies/CytronWiFiShield

Modified from ESP8266 WiFi Shield Library Server Source File
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

#include "CytronWiFiServer.h"
#include "CytronWiFiClient.h"

//#define DEBUG
#define DEBUG_OUTPUT Serial

//Public:
ESP8266Server::ESP8266Server(uint16_t port)
: _currentArgCount(0)
, _currentArgs(0)
{
    _port = port;
}

ESP8266Server::~ESP8266Server(){}

void ESP8266Server::begin()
{
	wifi.configureTCPServer(1, _port);
}

void ESP8266Server::end()
{
	wifi.configureTCPServer(0);
}

bool ESP8266Server::setTimeout(uint16_t time)
{
	return wifi.setServerTimeout(time);
}

ESP8266Client ESP8266Server::available()
{
	ESP8266Client client(0);
	char *p;
	String s;
	if(client.available()>0)
	{
      //wifi.find("\r\n\r\n");  
	  //skip all ,CONNECTs or ,CLOSEDs at the beginning of data received
      s = client.readStringUntil(':');
      p = strstr(s.c_str(),"+IPD,");
      if(p) 
      {
        uint8_t socket = *(p+5)-48;
#ifdef DEBUG
		DEBUG_OUTPUT.println(socket);
#endif
		IPAddress _currentIP;
		int index1 = s.lastIndexOf(',');
		int index2 = s.lastIndexOf(',',index1-1);
		if(index1!=-1&&index2!=-1)
		{
			String temp = s.substring(index2+1,index1);
			
			byte pos1 = temp.indexOf('.');
			_currentIP[0] = atoi(temp.substring(0,pos1).c_str());
			byte pos2 = temp.indexOf('.',pos1+1);
			_currentIP[1] = atoi(temp.substring(pos1+1,pos2).c_str());
			byte pos3 = temp.indexOf('.',pos2+1);
			_currentIP[2] = atoi(temp.substring(pos2+1,pos3).c_str());
			_currentIP[3] = atoi(temp.substring(pos3+1).c_str());
			wifi._client[socket]=_currentIP;
#ifdef DEBUG
		DEBUG_OUTPUT.println(wifi._client[socket]);
#endif
		}
		return ESP8266Client(socket);
      }
	}

	return ESP8266Client(ESP8266_SOCK_NOT_AVAIL);
}

uint8_t ESP8266Server::status() 
{
	return wifi.status();
}

//TODO: support file upload and download 

bool ESP8266Server::hasClient()
{
	_currentUri = String(); _currentMethod = HTTP_ANY;
	_contentLength = CONTENT_LENGTH_NOT_SET;
  
  ESP8266Client client = available();
  //_currentClient = client;
  if (!client) {
    return false;
  }

  if (!_parseRequest(client)) {
    return false;
  }
  _currentClient = client;
  return true;
}

void ESP8266Server::closeClient()
{
	_currentClient.stop();
}

String ESP8266Server::arg(const char* name) {
  for (int i = 0; i < _currentArgCount; ++i) {
    if (_currentArgs[i].key == name)
      return _currentArgs[i].value;
  }
  return String();
}

String ESP8266Server::arg(int i) {
  if (i < _currentArgCount)
    return _currentArgs[i].value;
  return String();
}

String ESP8266Server::argName(int i) {
  if (i < _currentArgCount)
    return _currentArgs[i].key;
  return String();
}

int ESP8266Server::args() {
  return _currentArgCount;
}

bool ESP8266Server::hasArg(const char* name) {
  for (int i = 0; i < _currentArgCount; ++i) {
    if (_currentArgs[i].key == name)
      return true;
  }
  return false;
}

size_t ESP8266Server::write(uint8_t c)
{
	return write(&c, 1);
}
size_t ESP8266Server::write(const uint8_t *buf, size_t size)
{
	return _currentClient.write(buf, size);
}

//Protected:
bool ESP8266Server::_parseRequest(ESP8266Client& client) {
  // Read the first line of HTTP request
  String req = client.readStringUntil('\r');

  // First line of HTTP request looks like "GET /path HTTP/1.1"
  // Retrieve the "/path" part by finding the spaces
  int addr_start = req.indexOf(' ');
  int addr_end = req.indexOf(' ', addr_start + 1);
  if (addr_start == -1 || addr_end == -1) {
#ifdef DEBUG
    DEBUG_OUTPUT.print("Invalid request: ");
    DEBUG_OUTPUT.println(req);
#endif
    return false;
  }
  
  String methodStr = req.substring(0, addr_start);
  String url = req.substring(addr_start + 1, addr_end);
  String searchStr = "";
  int hasSearch = url.indexOf('?');
  if (hasSearch != -1){
    searchStr = url.substring(hasSearch + 1);
    url = url.substring(0, hasSearch);
  }
  _currentUri = url;
  
  HTTPMethod method = HTTP_GET;
  if (methodStr == "POST") {
    method = HTTP_POST;
  } else if (methodStr == "DELETE") {
    method = HTTP_DELETE;
  } else if (methodStr == "PUT") {
    method = HTTP_PUT;
  } else if (methodStr == "PATCH") {
    method = HTTP_PATCH;
  }
  _currentMethod = method;
  
#ifdef DEBUG
  DEBUG_OUTPUT.print("method: ");
  DEBUG_OUTPUT.print(methodStr);
  DEBUG_OUTPUT.print(" url: ");
  DEBUG_OUTPUT.print(url);
  DEBUG_OUTPUT.print(" search: ");
  DEBUG_OUTPUT.println(searchStr);
#endif

  // below is needed only when POST type request
  if (method == HTTP_POST || method == HTTP_PUT || method == HTTP_PATCH || method == HTTP_DELETE){
    bool isForm = false;
    uint16_t contentLength = 0;
    //parse headers
	wifi.find("Content-Length: ");
	contentLength = client.readStringUntil('\r').toInt();
	wifi.find("\r\n\r\n");
	while (client.available()>0 && contentLength--)
		searchStr += (char)client.read();
  }
  if(client.available()>0)
	  client.flush();
  _parseArguments(searchStr);
  
#ifdef DEBUG
  DEBUG_OUTPUT.print("Request: ");
  DEBUG_OUTPUT.println(url);
  DEBUG_OUTPUT.print("Arguments: ");
  DEBUG_OUTPUT.println(searchStr);
#endif

  return true;
}

void ESP8266Server::_parseArguments(String data) {
#ifdef DEBUG
  DEBUG_OUTPUT.print("args length: ");
  DEBUG_OUTPUT.println(data.length());
#endif
  if (_currentArgs)
    delete[] _currentArgs;
  _currentArgs = 0;
  if (data.length() == 0) {
    _currentArgCount = 0;
    return;
  }
  _currentArgCount = 1;

  for (int i = 0; i < data.length(); ) {
    i = data.indexOf('&', i);
    if (i == -1)
      break;
    ++i;
    ++_currentArgCount;
  }
#ifdef DEBUG
  DEBUG_OUTPUT.print("args count: ");
  DEBUG_OUTPUT.println(_currentArgCount);
#endif

  _currentArgs = new RequestArgument[_currentArgCount];
  int pos = 0;
  int iarg;
  for (iarg = 0; iarg < _currentArgCount;) {
    int equal_sign_index = data.indexOf('=', pos);
    int next_arg_index = data.indexOf('&', pos);
#ifdef DEBUG
    DEBUG_OUTPUT.print("pos ");
    DEBUG_OUTPUT.print(pos);
    DEBUG_OUTPUT.print("=@ ");
    DEBUG_OUTPUT.print(equal_sign_index);
    DEBUG_OUTPUT.print(" &@ ");
    DEBUG_OUTPUT.println(next_arg_index);
#endif
    if ((equal_sign_index == -1) || ((equal_sign_index > next_arg_index) && (next_arg_index != -1))) {
#ifdef DEBUG
      DEBUG_OUTPUT.print("arg missing value: ");
      DEBUG_OUTPUT.println(iarg);
#endif
      if (next_arg_index == -1)
        break;
      pos = next_arg_index + 1;
      continue;
    }
    RequestArgument& arg = _currentArgs[iarg];
    arg.key = data.substring(pos, equal_sign_index);
    arg.value = data.substring(equal_sign_index + 1, next_arg_index);
#ifdef DEBUG
    DEBUG_OUTPUT.print("arg ");
    DEBUG_OUTPUT.print(iarg);
    DEBUG_OUTPUT.print(" key: ");
    DEBUG_OUTPUT.print(arg.key);
    DEBUG_OUTPUT.print(" value: ");
    DEBUG_OUTPUT.println(arg.value);
#endif
    ++iarg;
    if (next_arg_index == -1)
      break;
    pos = next_arg_index + 1;
  }
  _currentArgCount = iarg;
#ifdef DEBUG
  DEBUG_OUTPUT.print("args count: ");
  DEBUG_OUTPUT.println(_currentArgCount);
#endif

}
