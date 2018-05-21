#include "C_Sim800L.h"


C_Sim800L::C_Sim800L(byte rx_pin, byte tx_pin,  byte reset_pin, bool debug_state){
   serial = new SoftwareSerial(rx_pin, tx_pin);
   resetPin = reset_pin;
   debug = debug_state;
}

void C_Sim800L::Begin(unsigned long baud_rate){
  serial->begin(baud_rate);
}

void C_Sim800L::ResetSim800L(){
  digitalWrite(resetPin, HIGH);
  delay(100);
  digitalWrite(resetPin, LOW);
  if(debug){Serial.println(F("Resetted Sim800L module."));}
}


bool C_Sim800L::SendSms(char* number, char* text){
  
  bool success;
  sendSmsEntryTime = millis();
  char sms_cmd[40];

  if(debug){
    Serial.println(F("\nSms with the following content is about to be sent to sim800L: ")); Serial.println(text);
    Serial.print(F("C_Sim800L::SendSms strlen(text) - ")); Serial.println(strlen(text));
  }
  sprintf(sms_cmd, "AT+CMGS=\"%s\"\r\n", number);

  if (SendCmd(sms_cmd, ">")){
    for(byte i=0; i<strlen(text); i++){
      serial->write(text[i]);  
      
      if(serial->available()){
        if(debug){Serial.print(F("C_Sim800L::SendSms[first] - cleaning 1 byte from serial buffer, current buffer size - ")); Serial.print((int)serial->available()); Serial.print(F(", char - ")); Serial.println((char)serial->read());}
        else{serial->read();} 
      }        
    }
    serial->write(26);

    for(byte i=0; i<strlen(text); i++){ // ignore the same amount of bytes that was sent to it
      if(serial->available()){
        if(debug){Serial.print(F("C_Sim800L::SendSms[second] - cleaning 1 byte from serial buffer, current buffer size - ")); Serial.print((int)serial->available()); Serial.print(F(", char - ")); Serial.println((char)serial->read());}
        else{serial->read();} 
      }
    }
      
    if(debug){Serial.println(F("Sms sending command has been sent, response should be read in few seconds."));}
    success = true;
  }else{
    if(debug){Serial.println(F("WARNING: Sending sms failed. \">\" character was not received."));}
    success = false;
  }
  
  if(success == true){if(debug){Serial.print(F("Sending SMS took: ")); Serial.print(millis() - sendSmsEntryTime); Serial.println("ms.");}}
  memset(sms_cmd, 0, sizeof(sms_cmd));
  return success;
}


bool C_Sim800L::SendCmd(char* cmd, char* desiredResponsePart){ 
  ClearSerialBuffer();                             
  serial->write(cmd);
  if(debug){
    Serial.print(F("\nThe following command has been sent to sim800L: ")); Serial.print(cmd);
    Serial.println(F("Waiting for response..."));
  }
  
  char response[60];
  if(!GetResponse(response, sizeof(response), cmd)){
    if(debug){Serial.println(F("WARNING: No response received within 2000ms."));}
    return false;
  }
  else{
    if(!strstr(response, desiredResponsePart)){ 
      if(debug){
        Serial.println(F("WARNING: Response did not include desired characters."));
        Serial.println(F("Response: ")); Serial.println(response);
        Serial.println(F("\nDesired response phrase was: ")); Serial.println(desiredResponsePart);
      }
      memset(response, 0, sizeof(response));
      return false; 
    }
    else{
      if(debug){Serial.println(F("Response: ")); Serial.println(response);}
      memset(response, 0, sizeof(response));
      return true;
    }
  }
}


bool C_Sim800L::GetResponse(char* response, byte respLen, char* lastCmd) {                    // timeout = waitForTheFirstByteForThatLongUntilReturningFalse, keepCheckingFor = checkingTimeLimitAfterLastCharWasReceived
  for(byte i=0; i < respLen; i++){response[i]=0;}  //clear buffer
  C_USBhost::ReleaseAllButtons("Getting Sim800L response.");
  unsigned long functionEntryTime = millis();
  while(serial->available() <= 0){
    //delayMicroseconds(10);
    if(millis() - functionEntryTime > 2000){
      return false;
    }
  }

  byte i=0;
  byte ignorePreviouslyTypedCommand = 0;                                                  // value used to ignore the command that was sent in case if it's returned back (which is done by the sim800L...)
  unsigned long lastByteRec = millis();                                                   // timing functions (needed for reliable reading of the BTserial)
  while (10 > millis() - lastByteRec)                                                     // if no further character was received during 10ms then proceed with the amount of chars that were already received (notice that "previousByteRec" gets updated only if a new char is received)
  {         
    while (serial->available() > 0 && i < respLen - 1) {
      if(i == strlen(lastCmd)){
        if(!strncmp(response, lastCmd, strlen(response)-1)){
          ignorePreviouslyTypedCommand = i+1;
          //PL("IGNORING_PREVIOUSLY_TYPED_CMD_THAT_IS_RETURNED_FROM_SIM_800L");
          for(byte j=0; j<i; j++){response[j]=0;}                                         // clear that command from response 
        }
      }
      byte ind = i - ignorePreviouslyTypedCommand;
      if(ind < 0){ind=0;} 
      response[ind] = serial->read(); 
      i++;
      lastByteRec = millis();
    }    
    if(i >= respLen - 1){if(debug){Serial.println(F("WARNING: Sim800L response size is over limit."));} return true;}
  }
  return true;
}


void C_Sim800L::ReadSerialBuffer(){
  if(serial->available() && drc < 120){
    if(drc == 0 && debug){Serial.print(F("C_Sim800L::ReadSerialBuffer - FIRST drc TIME: ")); Serial.println(millis());}
    
    if(serial->available() == 63){
      if(debug){Serial.println(F("C_Sim800L::ReadSerialBuffer - WARNING: Sim800L software serial buffer is full. Some of the response was probably lost."));}
    }
    delayedResponse[drc++] = serial->read();
    if(debug){Serial.print(F("C_Sim800L::ReadSerialBuffer - serial->available value - ")); Serial.print((int)serial->available()); Serial.print(F(", drc value - ")); Serial.println((int)drc);}
  }
}

bool C_Sim800L::CheckDelayedSmsResponse(unsigned long sendingTime){                                                                  
  if(millis() - sendingTime > responseDelay){ 
    if(debug){Serial.println("");}

    if (drc > 0){
      if(strstr(delayedResponse, "OK")){
        if(debug){
          Serial.println(F("Sms was probably sent successfully. 'OK' response was received. Response:"));
          Serial.println(delayedResponse); 
        }
      }
      else{
        if(debug){
          Serial.println(F("WARNING: 'OK' not received in sms response. Response:"));
          Serial.println(delayedResponse);                                                            
        }
      }
      drc = 0;
      memset(delayedResponse, 0 , 120);
    }
    else{
      if(debug){Serial.println(F("WARNING: No response after sending sms."));}                                                                
    }
    
    /*
    char response[80];
    if(debug){Serial.println("");}
    if (GetResponse(response, sizeof(response), NULL)){
      if(strstr(response, "OK")){
        if(debug){Serial.println(F("Sms was probably sent successfully. 'OK' response was received."));}
        ret = true;                                                                       // not sure about this
      }
      else{
        if(debug){
          Serial.println(F("WARNING: 'OK' not received in sms response. Response:"));
          Serial.println(response);
          ret = false;                                                                      // not sure about this
        }
      }
    }
    else{
      if(debug){Serial.println(F("WARNING: No response after sending sms."));}
      ret = false;                                                                    // not sure about this
    }
    memset(response, 0, sizeof(response));
    */
    return true;
  }
  return false;                                                                           // not sure about this
}


void C_Sim800L::ClearSerialBuffer(){
  while(serial->available()){
    serial->read();
  }
}

void C_Sim800L::SetSmsResponseCollectionDelay(unsigned int response_delay){
	responseDelay = response_delay;
}


bool C_Sim800L::EmergencySmsRoutine(char* number, char* backupTextSms){
	switch(emergencySmsRoutineStep){
    case 0:
	    failedSmsTime = millis();
      ResetSim800L();
      emergencySmsRoutineStep++;  
      break;

    /*
    case 1:
      if(millis() - failedSmsTime > 5000){
        //Sim800L.write("AT");
        emergencySmsRoutineStep++;
      }
      break; 
      
    case 2:
      if(millis() - failedSmsTime > 9000){
        ReleaseAllButtons();
        char response[80];
        if (Get_Sim800L_Response(response, sizeof(response), "AT")){
          if(strstr(response, "SMS Ready")){
            PL(F("Sim800L re-booted successfully. 'SMS Ready' response received ('AT')."));
          }
          else{
            PL(F("WARNING: 'SMS Ready' not received ('AT'). Proceeding anyway..."));
            PL(F("Response was:")); PL(response);
          }
        }
        else{
          PL(F("WARNING: No response to 'AT' command. Proceeding anyway..."));
          PL(F("Response was:")); PL(response);
        }
        memset(response, 0, sizeof(response));
        emergencySmsRoutineStep++;
      }
      break;
	  
     
    case 1:
      if(millis() - failedSmsTime > 5000){
        //Sim800L.write("AT+CMGF=1\r\n");
        emergencySmsRoutineStep++;
      }
      break;     
	*/
    case 1:
      if(millis() - failedSmsTime > 8000){
        ClearSerialBuffer();
        serial->write("AT+CMGF=1\r\n");
        char response[80];
    		if(debug){
    			if (GetResponse(response, sizeof(response), "AT+CMGF=1\r\n")){
    			  if(strstr(response, "OK")){
    				Serial.println(F("Sim800L::EmergencySmsRoutine - Sms mode set successfully. 'OK' response received ('AT+CMGF=1')."));
    			  }
    			  else{
    				Serial.println(F("Sim800L::EmergencySmsRoutine - WARNING: 'OK' not received after setting sms mode ('AT+CMGF=1'). Proceeding anyway..."));
    				Serial.println(F("Response was:")); Serial.println(response);
    			  }
    			}
    			else{
    			  Serial.println(F("Sim800L::EmergencySmsRoutine - WARNING: No response to 'AT+CMGF=1' command. Proceeding anyway..."));
    			}
    		}
    		else{
    			GetResponse(response, sizeof(response), "AT+CMGF=1\r\n");
    		}
        memset(response, 0, sizeof(response));
        emergencySmsRoutineStep++;
      }
      break;
            
    case 2:
      if(debug){Serial.println(F("Sim800L::EmergencySmsRoutine - Trying to re-send sms..."));}
      if(SendSms(number, backupTextSms)){
        emergencySmsRoutineStep=0;

        //memset(backupTextSms, 0, sizeof(backupTextSms));
        /* false debug info (it should say that delayed response will arrive soon)
        if(debug){
          Serial.println(F("Sim800L::EmergencySmsRoutine - Sms was probably re-sent successfully. 'OK' response was received. Response:"));
          Serial.println(delayedResponse); 
        }
        */
		    return true;
      }
      else{
        emergencySmsRoutineStep=0;
        if(debug){Serial.println(F("Sim800L::EmergencySmsRoutine - Sms sending failed again. Re-trying procedure..."));}
      }  
      break;    
  }  
  return false;
}








