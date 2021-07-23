/*
MIT License

Copyright (c) 2021 Satoru Sato

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <bluefruit.h>
#include "class/hid/hid_device.h"

BLEDis bledis;

#define HIDINPUT(size)          (0x80 | size)
#define HIDOUTPUT(size)         (0x90 | size)
#define COLLECTION(size)        (0xa0 | size)
#define END_COLLECTION(size)    (0xc0 | size)
#define USAGE_PAGE(size)        (0x04 | size)
#define LOGICAL_MINIMUM(size)   (0x14 | size)
#define LOGICAL_MAXIMUM(size)   (0x24 | size)
#define REPORT_SIZE(size)       (0x74 | size)
#define REPORT_ID(size)         (0x84 | size)
#define REPORT_COUNT(size)      (0x94 | size)
#define USAGE(size)             (0x08 | size)
#define USAGE_MINIMUM(size)     (0x18 | size)
#define USAGE_MAXIMUM(size)     (0x28 | size)

#define KEYBOARD_ID 1
#define MEDIA_KEYS_ID 3
#define REPORT_ID_CONSUMER_CONTROL 2
#define REPORT_ID_MOUSE 3

uint8_t const hid_report_descriptor[] = {
  USAGE_PAGE(1),      0x01,          // USAGE_PAGE (Generic Desktop)
  USAGE(1),           0x06,          // USAGE (Keyboard)
  COLLECTION(1),      0x01,          // COLLECTION (Application) Start Keyboard Collection
  // ------------------------------------------------- Keyboard
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  REPORT_ID(1),       KEYBOARD_ID,   //   REPORT_ID (1) Report ID = 1 (Keyboard)
  USAGE_MINIMUM(1),   0xE0,          //   USAGE_MINIMUM (0xE0)
  USAGE_MAXIMUM(1),   0xE7,          //   USAGE_MAXIMUM (0xE7)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM (0)
  LOGICAL_MAXIMUM(1), 0x01,          //   LOGICAL_MAXIMUM (1)
  // ------------------------------------------------- 
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1) ; 1 byte (Modifier?)
  REPORT_COUNT(1),    0x08,          //   REPORT_COUNT (8) ; 8 bits
  HIDINPUT(1),        0x02,          //   INPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // ------------------------------------------------- 
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 1 byte (Reserved)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE (8) ; 8 bits
  HIDINPUT(1),        0x01,          //   INPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // ------------------------------------------------- 
  REPORT_COUNT(1),    0x05,          //   REPORT_COUNT (5) ; 5 bits (Num lock, Caps lock, Scroll lock, Compose, Kana)
  REPORT_SIZE(1),     0x01,          //   REPORT_SIZE (1)
  USAGE_PAGE(1),      0x08,          //   USAGE_PAGE (LEDs)
  REPORT_ID(1),       KEYBOARD_ID,   //   REPORT_ID (1) Report ID = 1 (Keyboard)
  USAGE_MINIMUM(1),   0x01,          //   USAGE_MINIMUM (0x01) ; Num Lock
  USAGE_MAXIMUM(1),   0x05,          //   USAGE_MAXIMUM (0x05) ; Kana
  HIDOUTPUT(1),       0x02,          //   OUTPUT (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  // ------------------------------------------------- 
  REPORT_COUNT(1),    0x01,          //   REPORT_COUNT (1) ; 3 bits (Padding)
  REPORT_SIZE(1),     0x03,          //   REPORT_SIZE (3)
  HIDOUTPUT(1),       0x03,          //   OUTPUT (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
  // ------------------------------------------------- 
  REPORT_COUNT(1),    0x06,          //   REPORT_COUNT (6) ; 6 bytes (Keys)
  REPORT_SIZE(1),     0x08,          //   REPORT_SIZE(8)
  LOGICAL_MINIMUM(1), 0x00,          //   LOGICAL_MINIMUM(0)
  LOGICAL_MAXIMUM(1), 0x65,          //   LOGICAL_MAXIMUM(0x65) ; 101 keys
  USAGE_PAGE(1),      0x07,          //   USAGE_PAGE (Kbrd/Keypad)
  USAGE_MINIMUM(1),   0x00,          //   USAGE_MINIMUM (0)
  USAGE_MAXIMUM(1),   0x65,          //   USAGE_MAXIMUM (0x65)
  HIDINPUT(1),        0x00,          //   INPUT (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
  // ------------------------------------------------- 
  END_COLLECTION(0),                 // END_COLLECTION
  // ------------------------------------------------- Media Keys, Mouse Keys
  TUD_HID_REPORT_DESC_CONSUMER(HID_REPORT_ID(REPORT_ID_CONSUMER_CONTROL)),
  TUD_HID_REPORT_DESC_MOUSE   (HID_REPORT_ID(REPORT_ID_MOUSE))
};

class BLEHidAlvin: public BLEHidAdafruit {
  public: err_t begin(void) {
    // keyboard, consumer, mouse
    uint16_t input_len [] = { sizeof(hid_keyboard_report_t), 2, sizeof(hid_mouse_report_t) };
    uint16_t output_len[] = { 1 };
  
    setReportLen(input_len, output_len, NULL);
    enableKeyboard(true);
    enableMouse(true);
    setReportMap(hid_report_descriptor, sizeof(hid_report_descriptor));
  
    VERIFY_STATUS( BLEHidGeneric::begin() );
  
    // Attempt to change the connection interval to 11.25-15 ms when starting HID
    Bluefruit.Periph.setConnInterval(9, 12);
  
    return ERROR_NONE;
  }
};

BLEHidAlvin blehid;

static int target_connection = 0;
static char bluetooth_name[4][9] = {
  "Planck 1",
  "Planck 2",
  "Planck 3",
  "Planck 4"
};
static uint8_t org_mac[6] = {0};

void setup() 
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }
  Serial.println(); 
  Serial.println("Planck BLE");
 
  // Bluetooth
  Bluefruit.begin();
  Bluefruit.getAddr(org_mac);
  Bluefruit.setTxPower(4); // Check bluefruit.h for supported values
  Bluefruit.setName(bluetooth_name[target_connection]);

  // Configure and Start Device Information Service
  bledis.setManufacturer("OLKB");
  bledis.setModel("Planck BLE");
  bledis.begin();

  // HID Service
  blehid.begin();

  // Set up and start advertising
  startAdv();
}

void startAdv(void) {  
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  Bluefruit.Advertising.addAppearance(BLE_APPEARANCE_HID_KEYBOARD);
  
  // Include BLE HID service
  Bluefruit.Advertising.addService(blehid);

  // There is enough room for the dev name in the advertising packet
  Bluefruit.Advertising.addName();
  
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds
}

uint8_t getChar(void) {
  while (!Serial.available()) {
    ;
  }
  uint8_t c;
  c = Serial.read();
  return c;
}

void change_address(int connection) {
  ble_gap_addr_t addr;

  target_connection = connection;
  sd_ble_gap_addr_get(&addr);
  // disconnect
  Bluefruit.disconnect(Bluefruit.connHandle());
  // advertising stop
  Bluefruit.Advertising.stop();
  addr.addr[0] = org_mac[0] + connection;
  Bluefruit.setAddr(&addr);
  // change name
  Bluefruit.setName(bluetooth_name[connection]);
  // advertising start
  Bluefruit.Advertising.start(0);
}

boolean convertkey(uint8_t modifier, uint8_t keycode[]) {
  static boolean flowflag = false;
  static unsigned long flowstart = 0;

  // Logicool Flow suport
  if (modifier == 0x05 && keycode[0] == 0x00 && keycode[1] == 0x00 && keycode[2] == 0x00 && 
      keycode[3] == 0x00 && keycode[4] == 0x00 && keycode[5] == 0x00) {
    // Left Option + Left Ctrl
    flowflag = true;
    flowstart = millis();
  }
  else if (flowflag) {
    if (modifier == 0x05 && (keycode[0] != 0x00 || keycode[1] != 0x00 || keycode[2] != 0x00 || 
      keycode[3] != 0x00 || keycode[4] != 0x00 || keycode[5] != 0x00)) {
      // flow flag clear
      flowflag = false;
    }
    else if (modifier == 0x00 && keycode[0] == 0x00 && keycode[1] == 0x00 && keycode[2] == 0x00 && 
        keycode[3] == 0x00 && keycode[4] == 0x00 && keycode[5] == 0x00) {
      // release
      flowflag = false;
      if (millis() - flowstart > 500) { // 500ms
        // send release all key
        blehid.keyboardReport(modifier, keycode);
        if (target_connection == 0) {
          target_connection = 1;
        }
        else {
          target_connection = 0;
        }
        change_address(target_connection);
        return true;
      }
    }
    else {
      return false;
    }
  }

  if (modifier == 0x08 && (keycode[0] == 0x1e || keycode[1] == 0x1e || keycode[2] == 0x1e || 
      keycode[3] == 0x1e || keycode[4] == 0x1e || keycode[5] == 0x1e)) {
    // cmd-1
    change_address(0);
    return true;      
  }
  else if (modifier == 0x08 && (keycode[0] == 0x1f || keycode[1] == 0x1f || keycode[2] == 0x1f || 
      keycode[3] == 0x1f || keycode[4] == 0x1f || keycode[5] == 0x1f)) {
    // cmd-2
    change_address(1);      
    return true;      
  }
  else if (modifier == 0x08 && (keycode[0] == 0x20 || keycode[1] == 0x20 || keycode[2] == 0x20 || 
      keycode[3] == 0x20 || keycode[4] == 0x20 || keycode[5] == 0x20)) {
    // cmd-3
    change_address(2);      
    return true;      
  }
  else if (modifier == 0x08 && (keycode[0] == 0x21 || keycode[1] == 0x21 || keycode[2] == 0x21 || 
      keycode[3] == 0x21 || keycode[4] == 0x21 || keycode[5] == 0x21)) {
    // cmd-4
    change_address(3);      
    return true;      
  }
  return false;      
}

void loop() {
  uint8_t modifier = 0;
  uint8_t keycode[6] = {0};
  uint8_t mediakey = 0;
  uint16_t mediareport = 0;
  uint8_t buttons = 0;
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t wheel = 0;
  uint8_t pan = 0;
  uint8_t c;

  while (Serial.available()) {
    c = getChar();
    if (c == 0xfd) {
      // start
      c = getChar();
      // key, media ,mouse
      switch (c) {
      case 9: // key
        // skip
        c = getChar();
        // modifier
        modifier = getChar();
        // reserved
        c = getChar();
        // keycode only 6kro support
        for (int i = 0; i < 6; i++) {
          keycode[i] = getChar();
        }
        if (Bluefruit.connected()) {
          if (!convertkey(modifier, keycode)) {
            blehid.keyboardReport(modifier, keycode);
          }
        }
        break;
      case 3: // media key
        // skip
        c = getChar();
        // media key
        mediakey = getChar();
        // skip (only 1 byte media key support)
        c = getChar();
        if (Bluefruit.connected()) {
          mediareport = mediakey;
          blehid.consumerReport(mediareport);
        }
        break;
      case 0: // mouse key
        // skip
        c = getChar();
        buttons = getChar();
        x = getChar();
        y = getChar();
        wheel = getChar();
        pan = getChar();
        c = getChar();
        if (Bluefruit.connected()) {
          blehid.mouseReport(BLE_CONN_HANDLE_INVALID, buttons, x, y, wheel, pan);
        }
        break;
      }
    }
    else {
      Serial.print("Error:");
      Serial.println(c, HEX);
    }
  }

  // Poll interval
  delay(10);
}
