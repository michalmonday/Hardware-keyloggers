
#include <SoftwareSerial.h>
SoftwareSerial mySerial(8, 9);

#define BAUD 9600

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ;
  } 
  
  mySerial.begin(BAUD);

  mySerial.write("AT+IPR=57600\r\n");
  mySerial.end();
  mySerial.begin(57700);
  mySerial.write("AT&W_SAVE\r\n");
  mySerial.write("AT&W\r\n");
 
}

void loop() { // run over and over
  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }

  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}

