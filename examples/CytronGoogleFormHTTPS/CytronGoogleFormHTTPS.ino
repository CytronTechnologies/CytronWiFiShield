/*
 *  HTTP over TLS (HTTPS) example sketch
 *
 *  This example demonstrates how to use
 *  client secure connect to access HTTPS API.
 *  This example sketch uploads the data to Google Form.
 *  The link for Google Form Spreadsheet
 *  https://docs.google.com/spreadsheets/d/14kkP-M6_bFmnTlBSK_6ILxSf12dfvxFEBAFyZxkQ__U/edit?usp=sharing
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

const char* host = "docs.google.com";
const int httpsPort = 443;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  if(!wifi.begin(2, 3))
  {
    Serial.println(F("Error talking to shield"));
    while(true);
  }
    
  Serial.println(F("Start wifi connection"));
  if(!wifi.connectAP(ssid, pass))
  {
    Serial.println(F("Error connecting to WiFi"));
    while(true);
  }
    
  Serial.print(F("Connected to: "));Serial.println(wifi.SSID());
  Serial.print(F("IP address: "));Serial.println(wifi.localIP());

  ESP8266Client client;
  Serial.print("Connecting to ");Serial.println(host);
  
  if (!client.secure_connect(host, httpsPort))
  {
    Serial.println(F("Failed to connect to server."));
    client.stop();
    while(true);
  }
  
  String data1 = "Hello";
  String data2 = "Cytron";
  
  String url = "/forms/d/1472Zeb9kDfira_8WAQRbAAdCx0gy8dsgh5HI0vxeqlQ/formResponse";
  String arg = "?entry.1019905728="+data1+"&entry.385998086="+data2+"&submit=Submit";
  Serial.print(F("requesting URL: "));
  Serial.println(url+arg);

  client.print(String("GET ") + url + arg + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "User-Agent: BuildFailureDetectorESP8266\r\n" +
               "Connection: close\r\n\r\n");

  Serial.println(F("request sent"));
  
  //set 5 sec timeout for response from server
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

  if(client.available()>0)
  {
    client.read();client.read(); //flush the first "\r\n"
    String line = client.readStringUntil('\r');
    if (line.endsWith("HTTP/1.1 200 OK")) {
      Serial.println(F("Data successfully sent!"));
    } 
  }
  
  //flush the rest of received data
  while(client.available()>0)
	client.read();

  client.stop();

  //disconnect from current joined network
  Serial.print(F("Disconnecting from "));Serial.println(wifi.SSID());
  wifi.disconnectAP();
}

void loop() {
}
