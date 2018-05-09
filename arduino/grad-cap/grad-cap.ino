// colorwheel demo for Adafruit RGBmatrixPanel library.
// Renders a nice circle of hues on our 32x32 RGB LED matrix:
// http://www.adafruit.com/products/607

// Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon
// for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.

#include "Adafruit_GFX.h"   // Core graphics library
#include "RGBmatrixPanel.h" // Hardware-specific library
#include "vtcross.h"

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

bool cmdAvailable(void);
void readCmd(void);
void dispMessage(bool reset);
void dispImage(void);

/* Debug messages to save memory */
const char debug_0[] PROGMEM = "Serial port open and ready for communication";
const char debug_1[] PROGMEM = "";

const char* const debug_msg[] PROGMEM = {debug_0, debug_1};

// If your 32x32 matrix has the SINGLE HEADER input,
// use this pinout:
#define CLK 11  // MUST be on PORTB! (Use pin 11 on Mega)
#define OE  9
#define LAT 10
#define A   A0
#define B   A1
#define C   A2
#define D   A3

enum {WAIT, TEXT_DISPLAY, IMAGE_DISPLAY, TEST} LED_STATE;

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

  
  Serial1.begin(115200);
  while (!Serial1) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial1.println("Hello Pi!");
  Serial.begin(57600);
  
  while (!Serial); // wait for serial port to connect. Needed for native USB port only

  Serial.println( (const __FlashStringHelper *) debug_msg[0]);

  
  int      x, y, hue;
  float    dx, dy, d;
  uint8_t  sat, val;
  uint16_t c;

  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(2.5);

  LED_STATE = IMAGE_DISPLAY;
}

void loop() {
    bool reset;
    if ((reset = cmdAvailable())) {
      readCmd();
    }
    switch(LED_STATE) {
    case WAIT:
      break;
    case TEXT_DISPLAY:
      dispMessage(reset);
      break;
    case IMAGE_DISPLAY:
      dispImage();
      LED_STATE = WAIT;
      break;
    case TEST:
      test();
      LED_STATE = WAIT;
      break;
    }
}

void test(void) {
  for (int i = 0; i < 32; i++) {
    for (int j = 0; j < 32; j++) {
      uint16_t val = (i * 32) + j;
      matrix.drawPixel(i,j, 0xFFFF);    
    }
  }
  
  matrix.swapBuffers(false);
}

/* display the image saved in the buffer */
void dispImage() {
  matrix.fillScreen(0);
  matrix.drawRGBBitmap(0, 0, gimp_image, matrix.width(), matrix.height());
//  matrix.drawPixel(matrix.width(),matrix.height(),128);
  matrix.swapBuffers(false);
}

/* display the message saved in the buffer */
void dispMessage(bool reset) {
  byte i;

  if (reset) {
    textX = matrix.width();
  }

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

////////////////////////// Communication code //////////////////////////////////////

// check if a command on the chosen interface (SPI/UART) is available
bool cmdAvailable(void) {
  char tmp;
  while (Serial1.available() > 0) {
    tmp = Serial1.read();
    Serial.print(tmp);
    if (tmp == 's') {
      return true;
    }
  }
}

// read the command and write appropriate data to globals
void readCmd(void) {
  char incomingByte;
  char command[64];
  unsigned int numBytes;
  memset(command, 0, 64);
  memset(str, 0, 256);
  if (Serial1.available())
  {
    incomingByte = Serial1.read();
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
        Serial1.print("Text\n");
        numBytes = Serial1.available();
        if (numBytes > 256) {
          numBytes = 255;
        }
        Serial1.readBytes(str,numBytes);
        textMin = strlen(str) * -15;
        Serial.println(str);
        LED_STATE = TEXT_DISPLAY;
        break;
      case 'I':
        Serial.print("Image: ");
        LED_STATE = IMAGE_DISPLAY;
        Serial.println("");
        break;
      case 'Q':
        Serial.print("Test: ");
        LED_STATE = TEST;
        break;
      default:
        Serial.print("Not an option: ");  
        Serial.println(incomingByte);
    }  
  }  
}
 

