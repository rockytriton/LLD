#include <SPI.h>
char buff [255];
volatile byte indx;
volatile boolean process;
int lastSize = 0;

void setup (void) {
   Serial.begin (115200);
   
   pinMode(MISO, OUTPUT); // have to send on master in so it set as output
   SPCR |= _BV(SPE); // turn on SPI in slave mode
   indx = 0; // buffer empty
   process = false;
   SPI.setClockDivider(2);
   SPI.attachInterrupt(); // turn on interrupt

   Serial.println("SPI LISTENING");
}

ISR (SPI_STC_vect) { 
   byte c = SPDR; // read byte from SPI Data Register
   
   if (indx < sizeof buff) {
      buff [indx++] = c; // save data in the next index in the array buff
      if (c == '\n') //check for the end of the word
      process = true;
   }
}

void loop (void) {
   if (process) {
      process = false; //reset the process
      buff[indx] = 0;
      
      Serial.print("LINE: ");
      Serial.println (buff); //print the array on serial monitor

      indx= 0; //reset button to zero
   }

   
}
