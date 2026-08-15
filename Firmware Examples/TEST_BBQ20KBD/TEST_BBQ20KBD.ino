/* 
https://github.com/solderparty/arduino_bbq10kbd
https://www.solder.party/docs/bbq20kbd/examples/

Update your Arduino Boards Manager to use this Additional Boards Manager URL:
https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
Select the Board option: Raspberry Pi Pico/RP2040/RP2350 
This allows customization of the I2C port pins and a lot of other awesomeness which it's better than the old MBED core.
*/

#include <BBQ10Keyboard.h>
#define BBQ10_KB_ADDR 0x1F
#include <Wire.h>
BBQ10Keyboard keyboard;

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN        16 // On Trinket or Gemma, suggest changing this to 1

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 1 // Popular NeoPixel ring size

// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

// Appears to be a TM1804 RGB order LED ^^^^^^^^^ used for the SAO Demo Controller boards
#define DELAYVAL 250 // Time (in milliseconds) to pause between pixels

bool isDevicePresentOnWire(uint8_t address) {
  Wire.beginTransmission(address); // Target the Wire1 bus
  byte error = Wire.endTransmission();
  
  return (error == 0); // Returns true if an ACK was received
}

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

  Serial.begin(115200);
  while (!Serial && millis() < 3000); // Wait for serial monitor

// 1. Initialize custom I2C pins for Wire1 (Example using Philhower Core)
Wire.setSDA(4); // GPIO 4
Wire.setSCL(5); // GPIO 5
Wire.begin();

  Serial.println("I2C address detection test");

  if (!isDevicePresentOnWire(BBQ10_KB_ADDR)){
    Serial.println("❌ ERROR: BBQ10 Keyboard not detected on Wire!");
    while(1); // Freeze execution
  }

  Serial.println("✅ Success: BBQ10 Keyboard detected on Wire!");
  keyboard.setBacklight(0.5f);

// 2. Instruct the keyboard library to use Wire1 instead of default Wire
  keyboard.begin(BBQ10_KB_ADDR, &Wire); 
  keyboard.setBacklight(0.5f);
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

  const int keyCount = keyboard.keyCount();
  if (keyCount == 0)
    return;

  const BBQ10Keyboard::KeyEvent key = keyboard.keyEvent();
  String state = "pressed";
  if (key.state == BBQ10Keyboard::StateLongPress)
    state = "held down";
  else if (key.state == BBQ10Keyboard::StateRelease)
    state = "released";

char buf[128];
snprintf(buf, sizeof(buf), "key: '%c' (dec %d, hex %02x) %s", key.key, key.key, key.key, state.c_str());
Serial.println(buf);

  // pressing 'b' turns off the backlight and pressing Shift+b turns it on
  if (key.state == BBQ10Keyboard::StatePress) {
    if (key.key == 'b') {
      keyboard.setBacklight(0);
    } else if (key.key == 'B') {
      keyboard.setBacklight(1.0);
    }
  }

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