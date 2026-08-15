#include <hardware/xosc.h>
#include <hardware/clocks.h>
#include <pico/runtime_init.h>

// Define the wake-up pin (Must be a valid GPIO, e.g., GPIO 10)
const int WAKE_PIN = 15;
const int PIN_BUTTON_1 = 14;
const int PIN_BUTTON_2 = 15;
const int PIN_BUTTON_3 =  7;
const int PIN_BUTTON_4 =  6;

void setup() {
  // // Initialize the onboard LED
  // pinMode(LED_BUILTIN, OUTPUT);
  
  // // Configure the wake pin with an internal pull-down resistor
  // pinMode(WAKE_PIN, INPUT_DOWN);
  pinMode(WAKE_PIN, INPUT_PULLUP);
  pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  pinMode(PIN_BUTTON_2, INPUT_PULLUP);
  pinMode(PIN_BUTTON_3, INPUT_PULLUP);
  pinMode(PIN_BUTTON_4, INPUT_PULLUP);
  Serial.begin(115200);
  while (!Serial && millis() < 3000) { delay(10); }// Safe yield
}

void loop() {
  if(!gpio_get(PIN_BUTTON_1)) { Serial.println("Button 1"); }
  if(!gpio_get(PIN_BUTTON_2)) { Serial.println("Button 2"); }
  if(!gpio_get(PIN_BUTTON_3)) { Serial.println("Button 3"); }
  if(!gpio_get(PIN_BUTTON_4)) { Serial.println("Button 4"); }

  // 1. ACTIVE STATE: Blink the LED
  // digitalWrite(LED_BUILTIN, HIGH);
  // delay(1000);
  // digitalWrite(LED_BUILTIN, LOW);
  // delay(500);

  // 2. PREPARE FOR SLEEP: Enable dormant wake interrupts on WAKE_PIN
  // This triggers when the pin transitions from LOW to HIGH (Edge High)
  gpio_set_dormant_irq_enabled(WAKE_PIN, IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS, true);

  // 3. ENTER DORMANT MODE: Shut down the crystal oscillator.
  // Execution freezes completely right here to save power.
  Serial.println("\n--- Going to Sleep! ---");
  delay(5000);
  xosc_dormant();

  // 4. WAKE UP: Execution resumes right here once WAKE_PIN goes HIGH.
  // Acknowledge/clear the interrupt flag so it can sleep again later
  gpio_acknowledge_irq(WAKE_PIN, IO_BANK0_DORMANT_WAKE_INTE0_GPIO0_EDGE_LOW_BITS);
  Serial.println("\n--- I'm awake! ---");
  delay(5000);

  // 5. RESTORE CLOCKS: Re-initialize system clocks back to normal speed
  runtime_init_clocks();
}
