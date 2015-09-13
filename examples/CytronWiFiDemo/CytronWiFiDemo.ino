#include <CytronWiFiShield.h>
#include <SoftwareSerial.h>

//const char *ssid = "Cytron-Asus";
//const char *pass = "f5f4f3f2f1";
const char *ssid = "NBCWIFI";
const char *pass = "yeansaw660916";
IPAddress ip(192, 168, 1 ,242);
ESP8266Server server(80);

const char *htmlHeader = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/html\r\n"
                        "Connection: close\r\n\r\n"
                        "<!DOCTYPE HTML>\r\n"
                        "<html>\r\n";
                          
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  if(!wifi.begin(8,9))
    Serial.println(F("Error talking to shield"));
//  wifi.reset();
  Serial.println(wifi.getVersion());
//  Serial.print(F("Mode: "));Serial.println(wifi.getMode());
//  wifi.setMode(WIFI_BOTH);
//  Serial.print(F("Mode: "));Serial.println(wifi.getMode());
  Serial.println(F("Setup wifi config"));
  wifi.config(ip);
  Serial.println(F("Start wifi connection"));
  if(!wifi.connectAP(ssid, pass))
    Serial.println(F("Error connecting to WiFi")); 
  Serial.print(F("Connected to "));Serial.println(wifi.getAP());
  Serial.println(F("IP address: "));
  Serial.println(wifi.localIP());
//  Serial.println(wifi.softAPIP());
//  wifi.updateStatus();
//  Serial.println(wifi.status());
  clientTest();
  espblink();
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  //clientTest();
  //delay(5000);
  serverTest();
}

void espblink()
{
  for(int i = 0;i<12;i++)
  {
    wifi.digitalWrite(2,wifi.digitalRead(2)^1);
    delay(100);
  }
}

void serverTest()
{
  ESP8266Client client = server.available();
  
  if(client.available()>0)
  {
    String s ="";
//    while(client.available()>0)
//      s += (char)client.read();
    s = client.readStringUntil('\n'); 
    client.readString(); //act as client.flush
    Serial.println(s);
    
    if(strstr(s.c_str(),"GET / HTTP/1.1"))
    {
      IPAddress ip = wifi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      client.print(htmlHeader);
      String htmlBody = "Hello from ESP8266 at ";
      htmlBody += ipStr;
      htmlBody += "</html>\r\n\r\n";
      client.print(htmlBody);
    }
    
    else if(strstr(s.c_str(),"GET /analog HTTP/1.1"))
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
    
    else if(strstr(s.c_str(),"GET /gpio2 HTTP/1.1"))
    {
      wifi.digitalWrite(2, wifi.digitalRead(2)^1);
      client.print(htmlHeader);
      String htmlBody="GPIO2 is now ";
      htmlBody += wifi.digitalRead(2)==HIGH?"HIGH":"LOW";
      htmlBody += "</html>\r\n";
      client.print(htmlBody);
    }
    
    else if(strstr(s.c_str(),"GET /info HTTP/1.1"))
    {
      client.print(htmlHeader);
      client.print(wifi.getVersion());
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
  int i=5;
  while (client.available()<=0&&i--)
  {
    delay(1000);
    if(i==1) {Serial.println(F("Timeout"));return;}
  }

  while (client.available()>0)
  {
    char c = (char)client.read();
    Serial.print(c);
  }
   
  client.stop();
}

