// colorwheel demo for Adafruit RGBmatrixPanel library.
// Renders a nice circle of hues on our 32x32 RGB LED matrix:
// http://www.adafruit.com/products/607

// Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon
// for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.

#include "Adafruit_GFX.h"   // Core graphics library
#include "RGBmatrixPanel.h" // Hardware-specific library

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

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

enum {WAIT, TEXT_DISPLAY} LED_STATE;

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, true);

char str[256] = "LET'S GO HOKIES!";

int    textX   = matrix.width(),
       textMin = strlen(str) * -15,
       hue     = 0;

int8_t ball[3][4] = {
  {  3,  0,  1,  1 }, // Initial X,Y pos & velocity for 3 bouncy balls
  { 17, 15,  1, -1 },
  { 27,  4, -1,  1 }
};

static const uint16_t PROGMEM ballcolor[3] = {
  0x0080, // Green=1
  0x0002, // Blue=1
  0x1000  // Red=1
};
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
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(2.5);

  LED_STATE = WAIT;

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
    switch(LED_STATE) {
    case WAIT:
      break;
    case TEXT_DISPLAY:
      dispMessage();
      break;
    }
}

// check if a command on the chosen interface (SPI/UART) is available
bool cmdAvailable(void) {
  char tmp;
  while (Serial.available() > 0) {
    tmp = Serial.read();
    if (tmp == 's') {
      return true;
    }
  }
  return false;
}

void dispMessage(void) {
  byte i;

  // Clear background
  matrix.fillScreen(0);

  // Bounce three balls around
  for(i=0; i<3; i++) {
    // Draw 'ball'
      matrix.fillCircle(ball[i][0], ball[i][1], 5, pgm_read_word(&ballcolor[i]));
    // Update X, Y position
    ball[i][0] += ball[i][2];
    ball[i][1] += ball[i][3];
    // Bounce off edges
    if((ball[i][0] == 0) || (ball[i][0] == (matrix.width() - 1)))
      ball[i][2] *= -1;
    if((ball[i][1] == 0) || (ball[i][1] == (matrix.height() - 1)))
      ball[i][3] *= -1;
  }

  // Draw big scrolly text on top
  matrix.setTextColor(matrix.ColorHSV(hue, 255, 255, true));
  matrix.setCursor(textX, 8);
  matrix.print(str);

  // Move text left (w/wrap), increase hue
  if((--textX) < textMin) textX = matrix.width();
  hue += 7;
  if(hue >= 1536) hue -= 1536;

  // Update display
  matrix.swapBuffers(false);
}

// read the command and write appropriate data to globals
void readCmd(void) {
  char incomingByte;
  char command[64];
  unsigned int numBytes;
  memset(command, 0, 64);
  memset(str, 0, 256);
  if (Serial.available())
  {
    Serial.print("Bytes available: ");
    Serial.println(Serial.available());
    incomingByte = Serial.read();
    Serial.println(incomingByte);
    switch (incomingByte) {
      case '1':
        Serial.print("Option1\n");
        Serial.readBytes(command, 2);
        Serial.println(command);
        LED_STATE = WAIT;
        matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 3, 0));
        break;
      case '2':
        Serial.print("Option2\n");
        Serial.readBytes(command, 3);
        Serial.println(command);
        LED_STATE = WAIT;
        matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 3, 3));
        break;
      case 'T':
        Serial.print("Text\n");
        numBytes = Serial.available();
        if (numBytes > 256) {
          numBytes = 255;
        }
        Serial.readBytes(str,numBytes);
        textMin = strlen(str) * -15;
        LED_STATE = TEXT_DISPLAY;
        break;
      default:
        Serial.print("Not an option: ");  
        Serial.println(incomingByte);
    }  
  }  
}
 

