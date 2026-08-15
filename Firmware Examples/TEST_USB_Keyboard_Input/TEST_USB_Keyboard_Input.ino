#include <Arduino.h>
#include "Adafruit_TinyUSB.h"

// Explicitly instantiate the global USB Host manager object
Adafruit_USBH_Host USBHost;

// Define the ASCII lookup table for a standard US keyboard layout
const char hid_to_ascii[] = {
    0,   0,   0,   0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
  'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',
  '3', '4', '5', '6', '7', '8', '9', '0', '\n',  27,  8, '\t', ' ', '-', '=', '[',
  ']', '\\',  0, ';', '\'', '`', ',', '.', '/'
};

void setup() {
  // Initialize UART0 on pins GP0 (TX) and GP1 (RX)
  Serial1.begin(115200);
  
  // Initialize the Arduino wrapper for the TinyUSB Host Core
  // '0' refers to the native physical USB port on the RP2040 chip
  USBHost.begin(0);
}

void loop() {
  // Process internal USB background tasks and drivers
  USBHost.task();
}

// TinyUSB Callback: Triggered when any HID device is plugged in
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  // If the connected device is a keyboard, request its first report packet
  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
    tuh_hid_receive_report(dev_addr, instance);
  }
}

// TinyUSB Callback: Triggered when a device is unplugged
void tuh_hid_unmount_cb(uint8_t dev_addr, uint8_t instance) {
  // Handled automatically by TinyUSB stack core
}

// TinyUSB Callback: Triggered when a key is pressed or released
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
    hid_keyboard_report_t const* kbd_report = (hid_keyboard_report_t const*) report;
    static uint8_t prev_keys[6] = {0};

    // Scan the 6 available key slots inside the standard HID report
    for (int i = 0; i < 6; i++) {
      uint8_t keycode = kbd_report->keycode[i]; 
      if (keycode == 0) continue;

      // Filter out keys that were already pressed in the prior packet
      bool is_new_press = true;
      for (int j = 0; j < 6; j++) {
        if (keycode == prev_keys[j]) {
          is_new_press = false;
          break;
        }
      }

      // If it's a new stroke, process it
      if (is_new_press && keycode < sizeof(hid_to_ascii)) {
        char typed_char = hid_to_ascii[keycode];
        
        if (typed_char != 0) {
          // Check for active Left/Right Shift keys
          bool is_shift = (kbd_report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT));
          if (is_shift && typed_char >= 'a' && typed_char <= 'z') {
            typed_char -= 32; // Convert character to Uppercase ASCII
          }
          
          // Instantly write data down the physical serial line (GP0)
          Serial1.print(typed_char);
        }
      }
    }

    // Cache the active report data for comparison next packet
    memcpy(prev_keys, kbd_report->keycode, 6);
    
    // Ready the interface to receive the next upcoming HID payload
    tuh_hid_receive_report(dev_addr, instance);
  }
}
