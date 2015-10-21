/************************************************************
This example sketch is a handy "serial passthrough". Anything
sent to the Arduino serial port will be routed to the ESP8266
software serial port, and vice-versa.

If you use the Serial Monitor, make sure you SET THE LINE
ENDING PULLDOWN to "Both NL & CR".

Then try typing commands from the AT command set.
For example, to set the mode to STA, connect to a network,
and check your IP address, type:
AT+CWMODE=1
AT+CWJAP="networkName","networkPassword"
AT+CIFSR
************************************************************/
#include <CytronWiFiShield.h>
#include <SoftwareSerial.h>

void setup() 
{
  Serial.begin(9600);
  wifi.begin(2, 3);
}

void loop() 
{
  serialPassthrough();
}

void serialPassthrough()
{
  while (Serial.available())
    wifi.write(Serial.read());
  while (wifi.available())
    Serial.write(wifi.read());
}
