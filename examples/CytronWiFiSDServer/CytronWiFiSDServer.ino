/*
 * This example creates a server that loads webpage 
 * from the files stored in microSDCard
 * 
 * ioControl.htm file can be referred 
 * https://github.com/CytronTechnologies/ESP8266-WiFi-Module
 */

#include <CytronWiFiShield.h>
#include <CytronWiFiServer.h>
#include <SPI.h>
#include <SdFat.h>
#include <SoftwareSerial.h>

const char* ssid = "...";
const char* pass = "...";

IPAddress ip(192, 168, 1 ,242);
ESP8266Server server(80);
const char htmlHeader[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nConnection: close\r\n\r\n";

SdFat sd;
                
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  if (!sd.begin(4)) {
    sd.initErrorHalt();
  }
  if(!wifi.begin(2, 3))
  {
    Serial.println("Error talking to shield");
    while(1);
  }
  Serial.println(wifi.firmwareVersion());
  wifi.config(ip);
  Serial.print("Status: ");Serial.println(wifi.status());
  Serial.println("Start wifi connection");
  if(!wifi.connectAP(ssid, pass))
  {
    Serial.println("Error connecting to WiFi");
    while(true);
  }
  Serial.print("Connected to ");Serial.print(wifi.SSID());
  Serial.print(", ");Serial.println(wifi.RSSI());
  Serial.print("IP address: ");Serial.println(wifi.localIP());
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  serverTest();
}

void serverTest()
{
  if(server.hasClient())
  {
    Serial.println(server.uri());
    Serial.println(server.method());
    for(int i = 0; i< server.args();i++)
    {
      Serial.print(server.argName(i));
      Serial.print("=");
      Serial.println(server.arg(i));
    }
    
    if(server.uri().equals("/gpio")&&server.method()==HTTP_GET)
    {
      if(server.arg(0).equals("ON")) 
        wifi.digitalWrite(server.argName(0).toInt(),LOW);
      else if(server.arg(0).equals("OFF"))
        wifi.digitalWrite(server.argName(0).toInt(),HIGH);  
    }

    SdFile file;
    if (!file.open("ioControl.htm", O_READ))
      return; 
    server.print(htmlHeader); 
    server.write(file);
    file.close();

    server.closeClient();    
  }
}
