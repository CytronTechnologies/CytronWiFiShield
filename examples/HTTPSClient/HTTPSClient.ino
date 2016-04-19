/*
 *  HTTP over TLS (HTTPS) example sketch
 *
 *  This example demonstrates how to use
 *  secure_connect function to access HTTPS API.
 *  We fetch and display the status of
 *  esp8266/Arduino project continuous integration
 *  build.
 *
 *  Created by Ivan Grokhotkov, 2015.
 *  This example is in public domain.
 *  
 *  Modified by Ng Beng Chet, 2016
 */

#include <CytronWiFiShield.h>
#include <CytronWiFiClient.h>
#include <SoftwareSerial.h>

const char* ssid = "...";
const char* pass = "...";

const char* host = "api.github.com";
const int httpsPort = 443;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  if(!wifi.begin(2, 3))
    Serial.println(F("Error talking to shield"));
  Serial.println(F("Start wifi connection"));
  if(!wifi.connectAP(ssid, pass))
    Serial.println(F("Error connecting to WiFi")); 
  Serial.print(F("Connected to: "));Serial.println(wifi.SSID());
  Serial.print(F("IP address: "));Serial.println(wifi.localIP());

  ESP8266Client client;
  Serial.print("Connecting to ");
  Serial.println(host);
  if (!client.secure_connect(host, httpsPort))
  {
    Serial.println(F("Failed to connect to server."));
    client.stop();
    return;
  }
  
  String url = "/repos/esp8266/Arduino/commits/master/status";
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //set 5 seconds timeout for response from server
  int i=5000;
  while (client.available()<=0&&i--)
  {
    delay(1);
    if(i==1) {
      Serial.println(F("Timeout"));
      client.stop();
      return;
      }
  }
  
  if(client.available())
  {
    if (client.find("\r\n\r\n"))
      Serial.println(F("headers received"));

    String line = client.readStringUntil('\n');
    if (line.startsWith("{\"state\":\"success\""))
      Serial.println(F("esp8266/Arduino CI successfull!"));  
    else 
      Serial.println(F("esp8266/Arduino CI has failed"));
    
    Serial.println(F("reply was:"));
    Serial.println(F("=========="));
    Serial.println(line);
    Serial.println(F("=========="));
    Serial.println(F("closing connection"));
    
    //flush the rest of received data
    while(client.available()>0)
    	client.flush();  
  }
  
  client.stop();
}

void loop() {
}
