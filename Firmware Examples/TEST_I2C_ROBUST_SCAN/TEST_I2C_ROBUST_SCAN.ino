#include <Arduino.h>
#include <Wire.h>

// Define RP2040-Zero Hardware Pins
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5

// Scanning Frequency Configuration
const uint32_t START_FREQ = 100000;  // 100 kHz (Standard Mode)
const uint32_t END_FREQ   = 1000000; // 1.0 MHz (Fast Mode Plus benchmark)
const uint32_t STEP_FREQ  = 100000;  // 100 kHz increments
const uint32_t BUS_TIMEOUT_US = 50000; // 50ms fault recovery timeout

// Tracks the maximum stable frequency for each discovered address
uint32_t max_stable_freq[128] = {0};
bool device_present_at_start[128] = {false};

// Force reset the physical I2C bus lines if a bus lockup occurs
void recoverBus() {
    pinMode(I2C_SDA_PIN, INPUT_PULLUP);
    pinMode(I2C_SCL_PIN, OUTPUT);
    
    // Clock out up to 9 cycles to free any hung slave devices
    for (int i = 0; i < 9; i++) {
        digitalWrite(I2C_SCL_PIN, LOW);
        delayMicroseconds(5);
        digitalWrite(I2C_SCL_PIN, HIGH);
        delayMicroseconds(5);
    }
    
    // Re-initialize Wire peripheral
    Wire.end();
}

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000); // Wait for serial monitor
    // while (!Serial) { delay(10); } // Wait for serial monitor
    
    Serial.println("\n=================================");
    Serial.println("RP2040 I2C Stress & Boundary Scanner");
    Serial.println("=================================");
}

void loop() {
    Serial.println("\n--- Starting New Sweep Dynamic Analysis ---");
    memset(max_stable_freq, 0, sizeof(max_stable_freq));
    memset(device_present_at_start, 0, sizeof(device_present_at_start));

    // Phase 1: Baseline mapping at lowest reference speed
    Wire.setSDA(I2C_SDA_PIN);
    Wire.setSCL(I2C_SCL_PIN);
    Wire.begin();
    Wire.setClock(START_FREQ);
    Wire.setTimeout(BUS_TIMEOUT_US);

    Serial.print("Mapping baseline devices at ");
    Serial.print(START_FREQ / 1000);
    Serial.println(" kHz...");

    for (uint8_t address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        if (Wire.endTransmission() == 0) {
            device_present_at_start[address] = true;
            max_stable_freq[address] = START_FREQ;
            Serial.print("  -> Found responsive device at Address: 0x");
            if (address < 16) Serial.print("0");
            Serial.println(address, HEX);
        }
    }

    // Phase 2: Step up frequency to find individual device crash points
    for (uint32_t current_freq = START_FREQ + STEP_FREQ; current_freq <= END_FREQ; current_freq += STEP_FREQ) {
        Serial.print("\nTesting Bus Frequency: ");
        Serial.print(current_freq / 1000);
        Serial.println(" kHz");

        Wire.setClock(current_freq);

        for (uint8_t address = 1; address < 127; address++) {
            // Only test devices that were successfully found during baseline initialization
            if (!device_present_at_start[address]) continue;

            Wire.beginTransmission(address);
            uint8_t error = Wire.endTransmission();

            if (error == 0) {
                max_stable_freq[address] = current_freq; // Device stable at this threshold
            } else {
                Serial.print("  [!] Address 0x");
                if (address < 16) Serial.print("0");
                Serial.print(address, HEX);
                Serial.print(" FAILED with Hardware Code: ");
                Serial.println(error);
                
                // If a fatal timeout or bus error happens, clear the hardware line state
                if (error == 4 || error == 5) { 
                    Serial.println("  [!] Bus Lockup Detected. Initiating Bit-Bang Hardware Recovery Routine...");
                    recoverBus();
                    Wire.begin();
                    Wire.setClock(current_freq);
                    Wire.setTimeout(BUS_TIMEOUT_US);
                }
            }
        }
        delay(100); // Settling delay between frequency jumps
    }

    // Phase 3: Print Consolidated Maximum Performance Results
    Serial.println("\n=========================================");
    Serial.println("   FINAL MAXIMUM FREQUENCY REPORT       ");
    Serial.println("=========================================");
    bool found_any = false;
    
    for (uint8_t address = 1; address < 127; address++) {
        if (device_present_at_start[address]) {
            found_any = true;
            Serial.print("Device 0x");
            if (address < 16) Serial.print("0");
            Serial.print(address, HEX);
            Serial.print(" Max Operational Frequency: ");
            
            if (max_stable_freq[address] == END_FREQ) {
                Serial.print(">= ");
            }
            Serial.print(max_stable_freq[address] / 1000);
            Serial.println(" kHz");
        }
    }
    
    if (!found_any) {
        Serial.println("No operational I2C devices detected on the bus.");
    }
    Serial.println("=========================================");

    delay(5000); // Wait before starting the next full loop sweep
}
