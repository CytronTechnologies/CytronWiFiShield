#include <CytronWiFiShield.h>
#include <CytronWiFiServer.h>
#include <SoftwareSerial.h>

#define ssid "..."
#define pass "..."
IPAddress ip(192, 168, 1 ,242);
ESP8266Server server(80);

const char htmlHeader[] = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Connection: close\r\n\r\n"
                        "<!DOCTYPE HTML>\r\n"
                        "<html>\r\n";
                          
void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  if(!wifi.begin(2, 3))
    Serial.println(F("Error talking to shield"));
  Serial.println(wifi.firmwareVersion());
  Serial.print(F("Mode: "));Serial.println(wifi.getMode());// 1- station mode, 2- softap mode, 3- both
  Serial.println(F("Setup wifi config"));
  wifi.config(ip);
  Serial.println(F("Start wifi connection"));
  if(!wifi.connectAP(ssid, pass))
    Serial.println(F("Error connecting to WiFi")); 
  Serial.print(F("Connected to: "));Serial.print(wifi.SSID());
  Serial.print(F(", "));Serial.println(wifi.RSSI());
  Serial.print(F("IP address: "));Serial.println(wifi.localIP());
  Serial.print(F("Status: "));Serial.println(wifi.status()); //2- wifi connected with ip, 3- got connection with servers or clients, 4- disconnect with clients or servers, 5- no wifi
  server.begin();
  espblink(100);

}

void loop() {
  // put your main code here, to run repeatedly:
  serverTest();
}

void espblink(int time)
{
  for(int i = 0;i<12;i++)
  {
    wifi.digitalWrite(2,wifi.digitalRead(2)^1);
    delay(time);
  }
}

void serverTest()
{
  if(server.hasClient())
  {
    Serial.println(server.uri());
    
    if(server.uri().equals("/"))
    {
      IPAddress ip = wifi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      server.print(htmlHeader);
      String htmlBody = "Hello from ESP8266 at ";
      htmlBody += ipStr;
      htmlBody += "</html>\r\n\r\n";
      server.print(htmlBody);
    }
    
    else if(server.uri().equals("/analog"))
    {
      server.print(htmlHeader);
      String htmlBody="";
      for (int a = 0; a < 6; a++)
      {
        htmlBody += "A";
        htmlBody += String(a);
        htmlBody += ": ";
        htmlBody += String(analogRead(a));
        htmlBody += "<br>\r\n";
      }
      htmlBody += "\r\n</html>\r\n";
      server.print(htmlBody);
    }
    
    else if(server.uri().equals("/gpio2"))
    {
      wifi.digitalWrite(2, wifi.digitalRead(2)^1);
      server.print(htmlHeader);
      String htmlBody="GPIO2 is now ";
      htmlBody += wifi.digitalRead(2)==HIGH?"HIGH":"LOW";
      htmlBody += "</html>\r\n";
      server.print(htmlBody);
    }
    
    else if(server.uri().equals("/info"))
    {
      String toSend = wifi.firmwareVersion();
      toSend.replace("\r\n","<br>");
      server.print(htmlHeader);
      server.print(toSend);
      server.print("</html>\r\n");
    }
    
    else if(server.uri().equals("/chat"))
    {
      server.setTimeout(180);// set 3 min for reply before disconnecting client
      unsigned long timeIn = millis();
      while(Serial.available()<=0&&timeIn+175000>millis());
      //server will return what you have sent in serial monitor
      //max characters 64
      server.print(htmlHeader);
      if(Serial.available()>0) server.write(Serial);
      server.print("</html>\r\n");
      server.setTimeout(10);
    }

    else if(server.uri().equals("/connect"))
    {
      server.setTimeout(180);
      server.print(htmlHeader);
      delay(100);
      if(!clientTest())
        server.print("Connection failed\r\n");
      server.print("</html>\r\n");
      server.setTimeout(10);
    }

    else
      server.print("HTTP/1.1 404 Not Found\r\n\r\n");
    
    server.closeClient();
  }
}

bool clientTest()
{
  const char destServer[] = "www.adafruit.com";
  ESP8266Client client;
  if (!client.connect(destServer, 80))
  {
    Serial.println(F("Failed to connect to server."));
    client.stop();
    return false;
  }
  wifi.updateStatus();
  const char *httpRequest = "GET /testwifi/index.html HTTP/1.1\r\n"
                           "Host: www.adafruit.com\r\n"
                           "Connection: close\r\n\r\n";
  if(!client.print(httpRequest))
  {
    Serial.println(F("Sending failed"));
    client.stop();
    return false;
  }

  // set timeout approximately 3s for server reply
  int i=3000;
  while (client.available()<=0&&i--)
  {
    delay(1);
    if(i==1) {
      Serial.println(F("Timeout"));
      client.stop();
      return false;
      }
  }
  String s, line;
  while (client.available()>0)
  {
    s=client.readStringUntil('\n');
    if(s.indexOf('\r')==-1) 
      line+=s;
  }
  
  server.print(line);
  
  client.stop();
  return true;
}

