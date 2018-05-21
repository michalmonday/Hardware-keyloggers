#ifndef C_Sim800L_H
#define C_Sim800L_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "C_USBhost.h"

class C_Sim800L
{
public:
  C_Sim800L(byte rx_pin, byte tx_pin, byte reset_pin, bool debug_state);
  void Begin(unsigned long baud_rate);
  bool SendCmd(char* cmd, char* desiredResponsePart);
  bool SendSms(char* number, char* text);
  void SetSmsResponseCollectionDelay(unsigned int response_delay=6000);
  

  void ReadSerialBuffer();
  bool CheckDelayedSmsResponse(unsigned long sendingTime);
  
  bool EmergencySmsRoutine(char* number, char* backupTextSms);
  
private:
  bool debug = false;
  byte resetPin;
  SoftwareSerial* serial;
  char delayedResponse[120];
  byte drc = 0;                 // delayed response count
  bool readyToSend = false;     // needed to confirm the sms right before reading USBhost buffer, otherwise it will get full and some of the response would be left (buffer has only 64 bytes)
  unsigned long sendSmsEntryTime;
  unsigned long failedSmsTime;
  unsigned int responseDelay;
  byte emergencySmsRoutineStep=0;
    
  void ResetSim800L();
  bool GetResponse(char* response, byte respLen, char* lastCmd);
  void ClearSerialBuffer();
  
};




#endif

