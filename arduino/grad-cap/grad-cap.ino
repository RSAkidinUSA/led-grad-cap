// colorwheel demo for Adafruit RGBmatrixPanel library.
// Renders a nice circle of hues on our 32x32 RGB LED matrix:
// http://www.adafruit.com/products/607

// Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon
// for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.

#include "Adafruit_GFX.h"   // Core graphics library
#include "RGBmatrixPanel.h" // Hardware-specific library


bool cmdAvailable(void);
void readCmd(void);

// If your 32x32 matrix has the SINGLE HEADER input,
// use this pinout:
#define CLK 11  // MUST be on PORTB! (Use pin 11 on Mega)
#define OE  9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3
// If your matrix has the DOUBLE HEADER input, use:
//#define CLK 8  // MUST be on PORTB! (Use pin 11 on Mega)
//#define LAT 9
//#define OE  10
//#define A   A3
//#define B   A2
//#define C   A1
//#define D   A0
RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, false);

void setup() {

// Open serial communications and wait for port to open:

  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }


  Serial.println("Goodnight moon!");

  
  int      x, y, hue;
  float    dx, dy, d;
  uint8_t  sat, val;
  uint16_t c;

  matrix.begin();

  for(y=0; y < matrix.width(); y++) {
    dy = 15.5 - (float)y;
    for(x=0; x < matrix.height(); x++) {
      dx = 15.5 - (float)x;
      d  = dx * dx + dy * dy;
      if(d <= (16.5 * 16.5)) { // Inside the circle(ish)?
        hue = (int)((atan2(-dy, dx) + PI) * 1536.0 / (PI * 2.0));
        d = sqrt(d);
        if(d > 15.5) {
          // Do a little pseudo anti-aliasing along perimeter
          sat = 255;
          val = (int)((1.0 - (d - 15.5)) * 255.0 + 0.5);
        } else
        {
          // White at center
          sat = (int)(d / 15.5 * 255.0 + 0.5);
          val = 255;
        }
        c = matrix.ColorHSV(hue, sat, val, true);
      } else {
        c = 0;
      }
      matrix.drawPixel(x, y, c);
    }
  }
}

void loop() {
    if (cmdAvailable()) {
      readCmd();
    }
}

// check if a command on the chosen interface (SPI/UART) is available
bool cmdAvailable(void) {
  while(Serial.available() > 0) {
    if (Serial.read() == 's') {
      return true;
    }
  }
  return false;
}

// read the command and write appropriate data to globals
void readCmd(void) {
  char incomingByte;
  char command[64];
  memset(command, 0, 64);
  while (Serial.available() == 0);
  incomingByte = Serial.read();
  switch (incomingByte) {
    case '1':
      Serial.print("Option1\n");
      Serial.readBytes(command, 2);
      Serial.println(command);
      matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 3, 0));
      break;
    case '2':
      Serial.print("Option2\n");
      Serial.readBytes(command, 3);
      Serial.println(command);
      matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 3, 3));
      break;
    default:
      Serial.print("Not an option\n");   
  }    
}
 

