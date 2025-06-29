#ifndef MAIN_HPP
#define MAIN_HPP

#include <Arduino.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include "oled/SSD1306Wire.h"
#include <vector>
#include <sstream>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiServer.h>
#include <ESPAsyncWebServer.h>
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_err.h"

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
#ifdef DIYMORE_BOARD
#define SDA_OLED 21
#define SCL_OLED 22
#define RST_OLED -1
#endif

#endif