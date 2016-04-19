/*
 *  This sketch sends data to Google Form via pushingbox API service.
 *  Pushingbox API provide services to allow us to send GET or POST HTTPS request to Google Form 
 *  without connecting to Google server directly.
 *
 *  You need to get devid from pushingbox. You can refer to tutorial
 *	http://www.instructables.com/id/Post-to-Google-Docs-with-Arduino/
 *  to create Google Form and pushingbox services.
 *
 */

#include <CytronWiFiShield.h>
#include <CytronWiFiClient.h>
#include <SoftwareSerial.h>
#define WiFi wifi

ESP8266Client client;
const char* ssid     = "YourSSID";
const char* password = "YourPass";
const char* devid = "YourPushingBoxDevID";
const char host[] = "api.pushingbox.com";
const int httpPort = 80;

void setup() {
  
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  if (!WiFi.begin(2, 3)) {
    Serial.println("WiFi shield not present");
    while (true);       // don't continue
  }

  while (!WiFi.connectAP(ssid, password)) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  Serial.println("Press any key to continue...");

}


void loop() {
  
  serialEvent();
}

void serialEvent() {
  String inputString1 = "";
  String inputString2 ="";
  char opt;
  if(Serial.available())
  {
    while(Serial.available()) Serial.read();
    Serial.print("Enter data1: ");
    while(Serial.available()<=0);
    while (Serial.available()) {
    // get the new byte:
      char inChar = (char)Serial.read();
    // add it to the inputString:
      inputString1 += inChar;
      delay(10);
    }
    Serial.println(inputString1);
    Serial.print("Enter data2: ");
    while(Serial.available()<=0);
    while (Serial.available()) {
    // get the new byte:
      char inChar = (char)Serial.read();
    // add it to the inputString:
      inputString2 += inChar;
      delay(10);
    }
    Serial.println(inputString2);
    Serial.print("Upload?(y/n): ");
    while(Serial.available()<=0);
    while (Serial.available()) {
    // get the new byte:
      opt = (char)Serial.read();
      delay(10);
    }  
    Serial.println(opt);
    if(opt=='y')
    {
      String data="GET /pushingbox?devid=";
	  data+=devid;
	  data+="&data1="; //GET request query to pushingbox API
      data+=inputString1;
      data+="&data2=";
      data+=inputString2;
      data+=" HTTP/1.1\r\n";
      data+="Host: api.pushingbox.com\r\n";
      data+="Connection: close\r\n\r\n";
	  
      upload(data);
    }
  }
}

void upload(String data)
{
  Serial.println("Connecting");
  while(true)
  {
    if(client.connect(host, httpPort))
    {
      if(!client.print(data)) continue;
	  //5 sec to wait for reply from the server
      int i=5000;
      while (client.available()<=0&&i--)
      {
        delay(1);
        if(i==1) {
          Serial.println(F("Timeout"));
          return;
          }
      }
      // Read all the lines of the reply from server and print them to Serial
      while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }   
      Serial.print(data);
      break;
    }
    Serial.print(".");
  }
  
  Serial.println("closing connection");
  client.stop();
  Serial.println("Press any key to continue...");
}
