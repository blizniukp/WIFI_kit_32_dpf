#include <Arduino.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include "oled/SSD1306Wire.h"
#ifdef ENABLE_WIFI
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiServer.h>
#include <ESPAsyncWebServer.h>
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"
#endif
#include "config.hpp"
#include "measurement.hpp"
#include "buzzer.hpp"

#ifdef WEMOS_BOARD
#define SDA_OLED 5
#define SCL_OLED 4
#define RST_OLED 16
#endif
#ifdef WIFI_KIT_32_BOARD
#define SDA_OLED 4
#define SCL_OLED 15
#define RST_OLED 16
#endif

#define MAX_LOG_LINES (60)
#define LAST_LINE (56)
#define MAX_BT_RESPONSE_TIME (10) /*10 seconds*/
#define PAIR_MAX_DEVICES 20
#define REMOVE_BONDED_DEVICES 1
