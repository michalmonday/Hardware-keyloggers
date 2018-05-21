#ifndef C_USBhost_H
#define C_USBhost_H

#include <Keyboard.h>
#include <Arduino.h>

class C_USBhost
{
public:
    C_USBhost(HardwareSerial& serial);
    C_USBhost(HardwareSerial& serial, bool debug_state);
    
    byte GetKey();
    void Begin(unsigned long baud_rate);
    void SetMode(char mode);
    void SetBaudRate(char* baud_rate);
    static void ReleaseAllButtons(char* reason);   

private:
  HardwareSerial& serial;
   
  char hidText[27];
  byte hbc = 0;                                                     // hidText string buffer count 
  byte rawHID[8];                                                   // modifier_bit_map, manufacturer(ignore) , key1, key2, key3, key4, key5, key6
  byte prevRawHID[8];
  byte fullBufferFlag_preventHold;
  byte collectedAsciiValue;

  void IgnoreBytesIfUseless();                             
  void ConvertInputToBytes();                              
  void Send_Report();         
  void SaveTheKey();       
  byte HID_to_ASCII(byte key, bool shiftDown);                     
  bool WasKeyPressed();                                    
  bool WasModifierPressed();                               
  byte GetKeyPressed();                                    
  bool WasShiftDown();                                     
  bool IsBitHigh(byte byteToConvert, byte bitToReturn);              
  void CleanUpVars();                                      
  void FullBuffer_BugPrevention();                         
};




#endif



