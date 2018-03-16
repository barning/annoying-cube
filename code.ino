#include <Wire.h>
#include "Adafruit_MPR121.h"

// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap = Adafruit_MPR121();

// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint16_t lasttouched = 0;
uint16_t currtouched = 0;

#include <Adafruit_NeoPixel.h>

#define PIXELSPIN   13
#define NUMPIXELS   60
#define CALIBRATIONTIME 20000

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIXELSPIN, NEO_GRBW + NEO_KHZ800);

unsigned long pixelsInterval=50;  // the time we need to wait
unsigned long colorWipePreviousMillis=0;
unsigned long theaterChasePreviousMillis=0;
unsigned long theaterChaseRainbowPreviousMillis=0;
unsigned long rainbowPreviousMillis=0;
unsigned long rainbowCyclesPreviousMillis=0;

int theaterChaseQ = 0;
int theaterChaseRainbowQ = 0;
int theaterChaseRainbowCycles = 0;
int rainbowCycles = 0;
int rainbowCycleCycles = 0;

uint16_t currentPixel = 0;// what pixel are we operating on

byte neopix_gamma[] = {
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };

// This is for counting the dice
long previousMillis = 0;
long interval = 0;

void setup() {
  currentPixel = 0;

  interval = random(60000, 300000);

  pixels.begin(); // This initializes the NeoPixel library.
  pixels.show(); // This sends the updated pixel color to the hardware.

  while (!Serial);        // needed to keep leonardo/micro from starting too fast!

  Serial.begin(9600);
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test");

  // Default address is 0x5A, if tied to 3.3V its 0x5B
  // If tied to SDA its 0x5C and if SCL then 0x5D
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");

  pinMode(4, OUTPUT); // Output for vibration

}

void loop () {
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis > interval) {
    digitalWrite(4, HIGH);

    // Get the currently touched pads
    currtouched = cap.touched();

    for (uint8_t i=0; i<12; i++) {
      // if it *was* touched and now *isnt*, alert!
      if (!(currtouched & _BV(i)) && (lasttouched & _BV(i)) ) {
        Serial.print(i); Serial.println(" released");
        previousMillis = currentMillis;
      }
    }

    // reset our state
    lasttouched = currtouched;

    colors();

  } else {
      digitalWrite(4, LOW);
      pulseWhite(5);
  }
}

void colors() {
    if ((unsigned long)(millis() - colorWipePreviousMillis) >= pixelsInterval) {
    colorWipePreviousMillis = millis();
    colorWipe(pixels.Color(0,255,125));
  }

  if ((unsigned long)(millis() - theaterChasePreviousMillis) >= pixelsInterval) {
    theaterChasePreviousMillis = millis();
    theaterChase(pixels.Color(127, 127, 127)); // White
  }

  if ((unsigned long)(millis() - theaterChaseRainbowPreviousMillis) >= pixelsInterval) {
    theaterChaseRainbowPreviousMillis = millis();
    theaterChaseRainbow();
  }

  if ((unsigned long)(millis() - rainbowPreviousMillis) >= pixelsInterval) {
    rainbowPreviousMillis = millis();
    rainbow();
  }

  if ((unsigned long)(millis() - rainbowCyclesPreviousMillis) >= pixelsInterval) {
    rainbowCyclesPreviousMillis = millis();
    rainbowCycle();
  }
}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c){
  pixels.setPixelColor(currentPixel,c);
  pixels.show();
  currentPixel++;
  if(currentPixel == NUMPIXELS){
    currentPixel = 0;
  }
}

void rainbow() {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, Wheel((i+rainbowCycles) & 255));
  }
  pixels.show();
  rainbowCycles++;
  if(rainbowCycles >= 256) rainbowCycles = 0;
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle() {
  uint16_t i;

  //for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
  for(i=0; i< pixels.numPixels(); i++) {
    pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + rainbowCycleCycles) & 255));
  }
  pixels.show();

  rainbowCycleCycles++;
  if(rainbowCycleCycles >= 256*5) rainbowCycleCycles = 0;
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c) {
  for (int i=0; i < pixels.numPixels(); i=i+3) {
    pixels.setPixelColor(i+theaterChaseQ, c);    //turn every third pixel on
  }
  pixels.show();
  for (int i=0; i < pixels.numPixels(); i=i+3) {
    pixels.setPixelColor(i+theaterChaseQ, 0);        //turn every third pixel off
  }
  theaterChaseQ++;
  if(theaterChaseQ >= 3) theaterChaseQ = 0;
}


//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow() {
  for (int i=0; i < pixels.numPixels(); i=i+3) {
    pixels.setPixelColor(i+theaterChaseRainbowQ, Wheel( (i+theaterChaseRainbowCycles) % 255));    //turn every third pixel on
  }

  pixels.show();
  for (int i=0; i < pixels.numPixels(); i=i+3) {
    pixels.setPixelColor(i+theaterChaseRainbowQ, 0);        //turn every third pixel off
  }
  theaterChaseRainbowQ++;
  theaterChaseRainbowCycles++;
  if(theaterChaseRainbowQ >= 3) theaterChaseRainbowQ = 0;
  if(theaterChaseRainbowCycles >= 256) theaterChaseRainbowCycles = 0;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void pulseWhite(uint8_t wait) {

  for(int j = 0; j < 256 ; j++){
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(0,0,0, neopix_gamma[j] ) );
    }
    delay(wait);
    pixels.show();
  }

  for(int j = 255; j >= 0 ; j--){
    for(uint16_t i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, pixels.Color(0,0,0, neopix_gamma[j] ) );
    }
    delay(wait);
    pixels.show();
  }
}
