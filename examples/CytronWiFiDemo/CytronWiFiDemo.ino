#include <CytronWiFiShield.h>
#include <CytronWiFiClient.h>
#include <CytronWiFiServer.h>
#include <SoftwareSerial.h>

const char *ssid = "...";
const char *pass = "...";
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
  {
    Serial.println(F("Error talking to shield"));
    while(1);
  }
  Serial.println(wifi.firmwareVersion());
  Serial.print(F("Mode: "));Serial.println(wifi.getMode());// 1- station mode, 2- softap mode, 3- both
  // Uncomment these 2 lines if you are using static IP Address
  // Serial.println(F("Setup wifi config"));
  // wifi.config(ip);
  Serial.println(F("Start wifi connection"));
  if(!wifi.connectAP(ssid, pass))
  {
    Serial.println(F("Error connecting to WiFi"));
    while(1);
  } 
  Serial.print(F("Connected to "));Serial.println(wifi.SSID());
  Serial.println(F("IP address: "));
  Serial.println(wifi.localIP());
  wifi.updateStatus();
  Serial.println(wifi.status()); //2- wifi connected with ip, 3- got connection with servers or clients, 4- disconnect with clients or servers, 5- no wifi
  clientTest();
  espblink(100);
  server.begin();
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
  ESP8266Client client = server.available();
  
  if(client.available()>0)
  {
    String req = client.readStringUntil('\r'); 
    // First line of HTTP request looks like "GET /path HTTP/1.1"
    // Retrieve the "/path" part by finding the spaces
    int addr_start = req.indexOf(' ');
    int addr_end = req.indexOf(' ', addr_start + 1);
    if (addr_start == -1 || addr_end == -1) {
      Serial.print(F("Invalid request: "));
      Serial.println(req);
      return;
    }
    req = req.substring(addr_start + 1, addr_end);
    Serial.print(F("Request: "));
    Serial.println(req);
    client.flush();
    
    if(req.equals("/"))
    {
      IPAddress ip = wifi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      client.print(htmlHeader);
      String htmlBody = "Hello from ESP8266 at ";
      htmlBody += ipStr;
      htmlBody += "</html>\r\n\r\n";
      client.print(htmlBody);
    }
    
    else if(req.equals("/analog"))
    {
      client.print(htmlHeader);
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
      client.print(htmlBody);
    }
    
    else if(req.equals("/gpio2"))
    {
      wifi.digitalWrite(2, wifi.digitalRead(2)^1);
      client.print(htmlHeader);
      String htmlBody="GPIO2 is now ";
      htmlBody += wifi.digitalRead(2)==HIGH?"HIGH":"LOW";
      htmlBody += "</html>\r\n";
      client.print(htmlBody);
    }
    
    else if(req.equals("/info"))
    {
      String toSend = wifi.firmwareVersion();
      toSend.replace("\r\n","<br>");
      client.print(htmlHeader);
      client.print(toSend);
      client.print("</html>\r\n");
    }

    else
      client.print("HTTP/1.1 404 Not Found\r\n\r\n");
    
    client.stop();
 
  }
}

void clientTest()
{
  const char destServer[] = "www.adafruit.com";
  ESP8266Client client;
  if (!client.connect(destServer, 80))
  {
    Serial.println(F("Failed to connect to server."));
    client.stop();
    return;
  }
  
  const char *httpRequest = "GET /testwifi/index.html HTTP/1.1\r\n"
                           "Host: www.adafruit.com\r\n"
                           "Connection: close\r\n\r\n";
  if(!client.print(httpRequest))
  {
    Serial.println(F("Sending failed"));
    client.stop();
    return;;
  }

  // set timeout approximately 5s for server reply
  int i=5000;
  while (client.available()<=0&&i--)
  {
    delay(1);
    if(i==1) {
      Serial.println(F("Timeout"));
      return;
      }
  }

  while (client.available()>0)
  {
    //char c = (char)client.read();
    //Serial.print(c);
    Serial.write(client.read());
  }
  
  client.stop();
}

