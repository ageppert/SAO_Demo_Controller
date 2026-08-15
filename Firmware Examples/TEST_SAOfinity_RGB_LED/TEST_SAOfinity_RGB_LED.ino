/* 
  Test for SAOfinity V1 chain of built-in LEDs
    https://hackaday.io/project/203745-saofinity
  Test code running on SAO Demo Controller
    https://hackaday.io/project/198034-sao-demo-controller
  Requires configuration of:
    Demo Controller Solder Jumper to send LED control over GPIO1.
    SAOfinity jumpers closed and open, add resistor to first tile to start signal routing
*/



// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// Released under the GPLv3 license to match the rest of the
// Adafruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN         1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS   36

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

// Appears to be a TM1804 RGB order LED ^^^^^^^^^ used for the SAO Demo Controller boards

#define DELAYVAL 250 // Time (in milliseconds) to pause between pixels

void setup() {
  // These lines are specifically to support the Adafruit Trinket 5V 16 MHz.
  // Any other board, you can remove this part (but no harm leaving it):
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  // END of Trinket-specific code.

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  // pixels.Color() takes RGB values, from 0,0,0 up to 255,255,255
  // The first NeoPixel in a strand is #0, second is 1, all the way up
  // to the count of pixels minus one.
}

void loop() {
  // AllOff();
  // delay(DELAYVAL);

  AllOn(25,0,0);
  delay(DELAYVAL);

  AllOn(0,25,0);
  delay(DELAYVAL);

  AllOn(0,0,25);
  delay(DELAYVAL);

  AllOn(25,25,25);
  delay(DELAYVAL);
}

void AllOff() {
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    pixels.show();
  }
}

void AllOn(uint8_t PixelRed, uint8_t PixelGreen, uint8_t PixelBlue) {
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(PixelRed, PixelGreen, PixelBlue));
    pixels.show();
  }
}