#include <Wire.h>

#define ADDRESS 21

const char *answer = "HELLO_I2C";

void setup() {
  
  Wire.begin(ADDRESS);
  Wire.onRequest(request_data);
  Wire.onReceive(receive_data);
  
  Serial.begin(9600);
  
  Serial.println("I2C Slave Initialized.");
}

void receive_data() {
  while (Wire.available() > 0) {
    char buffer[32];
    byte b = Wire.read();

    sprintf(buffer, "Received: %2.2X - %c", b, b);
    Serial.println(buffer);
  }
}

void request_data() {

  // Send response back to Master
  Wire.write(answer,strlen(answer));
  
  // Print to Serial Monitor
  Serial.print("Sent Data: ");
  Serial.println(answer);
}

void loop() {
  delay(50);
}
