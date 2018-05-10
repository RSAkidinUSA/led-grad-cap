// colorwheel demo for Adafruit RGBmatrixPanel library.
// Renders a nice circle of hues on our 32x32 RGB LED matrix:
// http://www.adafruit.com/products/607

// Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon
// for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.

#include "Adafruit_GFX.h"   // Core graphics library
#include "RGBmatrixPanel.h" // Hardware-specific library
#include "images.h"

// Similar to F(), but for PROGMEM string pointers rather than literals
#define F2(progmem_ptr) (const __FlashStringHelper *)progmem_ptr

bool cmdAvailable(void);
void readCmd(void);
void dispMessage(bool reset);
bool dispMessage(const char *buf, int rval, int gval, int bval, bool reset);
void dispImage(uint16_t *buf);
void dispImage(const uint16_t *buf);
void dispPreset(bool);

// Array of all images
#define NUM_IMAGES 5
const uint16_t* const image_array[NUM_IMAGES] = {oldlogo, america, ecelogo, oldlogowhite, vertlogo};
int imageNum;

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

enum {WAIT, TEXT_DISPLAY, IMAGE_DISPLAY, PRESET} LED_STATE;

RGBmatrixPanel matrix(A, B, C, D, CLK, LAT, OE, true);

char str[256] = "LET'S GO HOKIES!";

int    textX   = matrix.width(),
       textMin = strlen(str) * -15,
       hue     = 0;

uint8_t presetNum;

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

uint16_t imagebuf[1024];

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

  // copy something into imagebuf so it works
  memset(imagebuf, 0, 1024);

  matrix.begin();
  matrix.setTextWrap(false); // Allow text to run off right edge
  matrix.setTextSize(2.5);

  LED_STATE = PRESET;
  presetNum = 0;
  imageNum = 0;
}

void loop() {
    bool reset;
    // set reset equal to cmdAvailable
    reset = cmdAvailable();
    if (reset) {
      readCmd();
    }
    switch(LED_STATE) {
    case WAIT:
      break;
    case TEXT_DISPLAY:
      dispMessage(reset);
      break;
    case IMAGE_DISPLAY:
      dispImage(image_array[imageNum]);
      LED_STATE = WAIT;
      break;
    case PRESET:
      dispPreset(reset);
      break;
    }
}

///////////////////////////////// PRESET FUNCTIONS ///////////////////////////////////////////

#define NUMCYCLES 25

/* Debug messages to save memory */
const char preset_0_str[] PROGMEM = "Lets go Hokies!";
const char preset_1_str[] PROGMEM = "Start Jumping!";
const char ece_0_str[] PROGMEM = "Shout out to the ECE Dept!";
const char ece_1_str[] PROGMEM = "Special thanks to the AMP Lab!";
const char count_0_str[] PROGMEM = "3    2    1    WE DID IT!!!!";
const char sa_0_string[] PROGMEM = "@RSAkidinUSA";


#define NUMSTRINGS 2
const char* const preset_str[2] = {preset_0_str, preset_1_str};
const char* const ece_str[2] = {ece_0_str, ece_1_str};

void presetImgStrArr(const uint16_t *imagebuf, const char **arr, bool reset) {
  static int count = 0, strCount = 0;
  if (reset) {
    count = 0;
    strCount = 0;
    dispMessage(arr[strCount], 255, 255, 255, reset);
  }
  if (count < NUMCYCLES) {
    dispImage(imagebuf);
    count++;
  } else if (dispMessage(arr[strCount], 255, 255, 255, reset)) {
    strCount++;
    strCount %= NUMSTRINGS;
    count = 0;
  }
}

void presetImgStr(const uint16_t *imagebuf, const char *strbuf, bool reset) {
  static int count = 0;
  if (reset) {
    count = 0;
    dispMessage(strbuf, 255, 255, 255, reset);
  }
  if (count < NUMCYCLES) {
    dispImage(imagebuf);
    count++;
  } else if (dispMessage(strbuf, 255, 255, 255, reset)) {
    count = 0;
  }
}

void preset2ImgStr(const uint16_t *imagebuf0, const uint16_t *imagebuf1, const char *strbuf, bool reset) {
  static int count = 0;
  static int imgcount = 0;
  if (reset) {
    count = 0;
    dispMessage(strbuf, 255, 255, 255, reset);
  }
  if (count < NUMCYCLES) {
    if (imgcount == 0) {
      dispImage(imagebuf0);
    } else if (imgcount == 1) {
      dispImage(imagebuf1);
    }
    count++;
  } else if (dispMessage(strbuf, 255, 255, 255, reset)) {
    count = 0;
    imgcount++;
    imgcount %= 2;
  }
}




/////////////////////////////////DISPLAY FUNCTIONS////////////////////////////////////////////

// display a preset
void dispPreset(bool reset) {
  switch(presetNum) {
    case 6:
      preset2ImgStr(southafrica, america, sa_0_string, reset);
      break;
    case 5:
      if (dispMessage(count_0_str, 255, 255, 255, reset)) {
        LED_STATE = WAIT;
      }
      break;
    case 4:
      presetImgStrArr(ecelogo, ece_str, reset);
      break;
    case 3:
      presetImgStrArr(oldlogowhite, preset_str, reset);
      break;
    case 2:
      presetImgStrArr(america, preset_str, reset);
      break;
    case 1:
      presetImgStrArr(vertlogo, preset_str, reset);
      break;
    case 0:
    default:
//      presetImgStr(vertlogo, preset_str[1], reset);
      presetImgStrArr(oldlogo, preset_str, reset);
      break;
  }
}

/* display the image saved in the buffer */
void dispImage(uint16_t *buf) {
  matrix.fillScreen(0);
  matrix.drawRGBBitmap(0, 0, buf, matrix.width(), matrix.height());
  matrix.swapBuffers(false);
}

/* display an image from flash */
void dispImage(const uint16_t *buf) {
  matrix.fillScreen(0);
  matrix.drawRGBBitmap(0, 0, buf, matrix.width(), matrix.height());
  matrix.swapBuffers(false);
}

/* display a preset message */
bool dispMessage(const char *buf, int rval, int gval, int bval, bool reset) {
  byte i;
  bool done = false;
  static int textX = matrix.width();
  int   textMin = strlen_P(buf) * -15;

  if (reset) {
    textX = matrix.width();
  }

  // Clear background
  matrix.fillScreen(0);

  // Draw big scrolly text on top
  matrix.setTextColor(matrix.Color888(rval, gval, bval, true));
  matrix.setCursor(textX, 8);
  matrix.print(F2(buf));

  // Move text left (w/wrap), increase hue
  if((--textX) < textMin) {
    textX = matrix.width();
    done = true;
  }

  // Update display
  matrix.swapBuffers(false);
  return done;
}

/* display the message saved in the buffer */
void dispMessage(bool reset) {
  byte i;
  bool done = false;

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
  if((--textX) < textMin) {
    textX = matrix.width();
    done = true;
  }
  hue += 7;
  if(hue >= 1536) hue -= 1536;

  // Update display
  matrix.swapBuffers(false);
  return done;
}

////////////////////////// Communication code //////////////////////////////////////

// check if a command on the chosen interface (SPI/UART) is available
bool cmdAvailable(void) {
  char tmp;
  while (Serial1.available() > 0) {
    tmp = Serial1.read();
    if (tmp == 's') {
      return true;
    }
  }
  return false;
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
    switch (incomingByte) {
      case 'P':
        presetNum = (uint8_t) Serial1.read();
        LED_STATE = PRESET;
        matrix.fillRect(0, 0, 32, 32, matrix.Color333(0, 3, 0));
        break;
      case 'T':
        numBytes = Serial1.available();
        if (numBytes > 256) {
          numBytes = 255;
        }
        Serial1.readBytes(str,numBytes);
        textMin = strlen(str) * -15;
        Serial1.println(numBytes);
        LED_STATE = TEXT_DISPLAY;
        break;
      case 'I':
        imageNum = (uint8_t) Serial1.read();
        if (imageNum >= NUM_IMAGES) {
          imageNum = 0;
        }
        LED_STATE = IMAGE_DISPLAY;
        break;
      default:
        Serial.print(F("Not an option: "));  
        Serial.println(incomingByte);
        break;
    }  
  }  
}
 

