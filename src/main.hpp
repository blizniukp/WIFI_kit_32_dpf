#include <Arduino.h>
#include <Wire.h>
#include <BluetoothSerial.h>

#include "oled/SSD1306Wire.h"


#define SDA_OLED 4
#define SCL_OLED 15
#define RST_OLED 16

#define MAX_LOG_LINES (60)
#define LAST_LINE (55)
