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

#define MAX_UNIT_LENGTH (4)
#define MAX_COMMAND_LENGTH (9)
#define MAX_CAPTION_LENGTH (30)

typedef struct measurement_def
{
  byte id; /*Index*/
  char caption[MAX_CAPTION_LENGTH];
  char command[MAX_COMMAND_LENGTH];                             /*Command to send*/
  char unit[MAX_UNIT_LENGTH];                                   /*Unit [g,km,min]*/
  float value;                                                  /*Calculated value*/
  bool (*calcFunPtr)(char *command, float *val, float divider); /*A pointer to a function to retrieve and calculate values*/
  float divider;                                                /*Parameter to calc_fun*/
} measurement_t;
