
#define BAUD_RATE 9600

#define mySerial Serial1

void setup() {
  Serial.begin(BAUD_RATE);
  while (!Serial){;}

  mySerial.begin(BAUD_RATE);
}

void loop() {

  if (mySerial.available()) {
    Serial.write(mySerial.read());
  }
  
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}
