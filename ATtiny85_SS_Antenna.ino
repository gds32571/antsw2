/*
  Serial to pin 2
  receives serial data on pin 2 and turns on/off digital pin
  First character is pin, second character is state: 0 or 1 
  Must delay 1/2 second after sending string before sending another.
  16 Sept 2018 - gswann

   Clk - internal 8 MHz

   23 April 2019 - gswann v2
   Updated to respond to software reset
   Three 'r' characters sent in sequence

   29 Apr 2020 - gswann in The Villages
   This is for the Five Port Switch controller
   with ATTiny85 IC
   Changed protocol to be a single character

   This version for Antenna Switch

   7 September 2020
   Updated to add decode for a sixth port.

*/

// Runs on Attiny44, also ATTiny 85 if you change the pins
// This LED is currently not used
#define led 6    // package pin 10

char c ;

#define delayval 500

#include <SoftwareSerial.h>
SoftwareSerial mySerial(3,4); // RX, TX  package pins 2 and 3  -  ATTiny85
// SoftwareSerial mySerial(10, 9); // RX, TX  package pins 2 and 3  -  ATTiny44

void setup() {

// do this for antenna tuner version on ATTiny 85
pinMode(0, OUTPUT);
pinMode(1, OUTPUT);
pinMode(2, OUTPUT);

/* for testing pins
  pinMode(0, OUTPUT);
  for (int ii=0;ii<300;ii++){
     digitalWrite(0,HIGH);
     delay(1000);
     digitalWrite(0,LOW);
     delay(1000);
  }
*/
    
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  mySerial.begin(300);
  mySerial.listen();
  delay(delayval);
  char x = 0;
  while (x != '\n'){
    x = getChar();
  }

  mySerial.print("ATtiny85 SS Antenna starting\n");
}

// chars are -128 to 127
char getChar() {
  char c = -1;
  while (c == -1 ) {
    c = mySerial.read();
  }
  return c;
}

void loop() {

  char pin = getChar();

// software reset 
  if (pin == 'r'){
    return;
  }
  
  //char state = getChar();
  //char newline = getChar();

  mySerial.print(pin);
  mySerial.print(" ");
//  mySerial.println(state);
  pin = pin - 0x30;
//  state = state - 0x30;
  
  if (pin >= 0 && pin <= 6) {
     switch (pin){
       case 0:
          digitalWrite(2,0);  digitalWrite(1,0);    digitalWrite(0,0);
          break;
       case 1:
          digitalWrite(2,0);  digitalWrite(1,0);    digitalWrite(0,1);
          break;
       case 2:
          digitalWrite(2,0);  digitalWrite(1,1);    digitalWrite(0,0);
          break;
       case 3:
          digitalWrite(2,0);  digitalWrite(1,1);    digitalWrite(0,1);
          break;
       case 4:
          digitalWrite(2,1);  digitalWrite(1,0);    digitalWrite(0,0);
          break;
       case 5:
          digitalWrite(2,1);  digitalWrite(1,0);    digitalWrite(0,1);
          break;
       case 6:
          digitalWrite(2,1);  digitalWrite(1,1);    digitalWrite(0,0);
          break;
     }
  }
  delay(10);
}
