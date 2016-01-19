/*
  WiFi Web Server LED Blink

 A simple web server that lets you blink an LED via the web.
 This sketch will print the IP address of your WiFi Shield (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to turn on and off the LED on pin 13.

 If the IP address of your shield is yourAddress:
 http://yourAddress/H turns the LED on
 http://yourAddress/L turns it off

 Circuit:
 * WiFi shield attached
 * LED attached to pin 13
 created 25 Nov 2012
 by Tom Igoe
 modified 16 Sept 2015
 by Ng Beng Chet
 */

/* This example utilises softAP ability of the shield.
 *  
 * You can configure softAP settings with function below
 * WiFi.softAP(ssid, password, channel id, encryption)
 * 
 * by default channel id = 1, encryption = WPA_WPA2_PSK
 * 
 * Encryption option:
 * OPEN - 0
 * WPA_PSK - 2
 * WPA2_PSK - 3
 * WPA_WPA2_PSK - 4
 * 
 */
 
#include <CytronWiFiShield.h>
#include <CytronWiFiServer.h>
#include <SoftwareSerial.h>
#define WiFi wifi

const char ssid[] = "CytronESPShield";      //  your network SSID (name)
const char pass[] = "RobotHeadToToe";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

ESP8266Server server(80);
bool status = false;

void setup() {
  Serial.begin(115200);      // initialize serial communication
  pinMode(13, OUTPUT);      // set the LED pin mode

  // check for the presence of the shield:
  if (!WiFi.begin(2, 3)) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  String fv = WiFi.firmwareVersion();
  Serial.println(fv);

  // attempt to connect to Wifi network:
  WiFi.setMode(WIFI_AP);
  if(!WiFi.softAP(ssid, pass))
  //if(!WiFi.softAP(ssid, pass, 5, WPA_PSK)) // use WPA encryption
    Serial.println("Setting softAP failed");
  Serial.println(WiFi.softAPIP());
  server.begin();                           // start the web server on port 80
}


void loop() {
  
  ESP8266Client client = server.available();   // listen for incoming clients

  if(!client) return;
  Serial.println("new client");
  
  if (client.connected()) //if client is present and connected
  {             
      String s = client.readStringUntil('\r');   //get the first line of request       
      // Check to see if the client request was "GET /H" or "GET /L":
      if (strstr(s.c_str(),"GET /H")) 
        digitalWrite(13, HIGH);               // GET /H turns the LED on
      
      else if (strstr(s.c_str(),"GET /L"))
        digitalWrite(13, LOW);                // GET /L turns the LED off
         
      Serial.print(s);
      while(client.available())      
        Serial.write(client.read());                    // print the client request out the serial monitor

      // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
      // and a content-type so the client knows what's coming, then a blank line:
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println();

      // the content of the HTTP response follows the header:
      client.print("Click <a href=\"/H\">here</a> turn the LED on pin 13 on<br>");
      client.print("Click <a href=\"/L\">here</a> turn the LED on pin 13 off<br>");

      // The HTTP response ends with another blank line:
      client.println();
      // close the connection:
      client.stop();
      Serial.println("client disonnected");
  }
}
