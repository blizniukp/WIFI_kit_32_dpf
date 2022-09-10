#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <stdbool.h>
#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include "measurement.hpp"

#define CFG_NAMESPACE_WIFI "wifi"
#define CFG_WIFI_SSID "wifi_ssid"
#define CFG_WIFI_PASSWORD "wifi_passwd"
#define CFG_WIFI_SSID_DEFAULT "WIFI_kit_32_dpf"
#define CFG_WIFI_PASSWORD_DEFAULT "wifikit32"

#define CFG_DEVICE_NAME_DEFAULT "WIFI_kit_32_dpf"

#define CFG_NAMESPACE_BT "bt"
#define CFG_BT_IF_NAME "bt_name"
#define CFG_BT_IF_PIN "bt_pin"
#define CFG_BT_IF_NAME_DEFAULT "V-LINK"
#define CFG_BT_IF_PIN_DEFAULT "1234"

#define CFG_NAMESPACE_DISPLAY "display"
#define CFG_DISPLAY_FLIP_SCREEN "d_flip"
#define CFG_DISPLAY_FLIP_SCREEN_DEFAULT false

#define CFG_NAMESPACE_MEASUREMENTS "measurements"

typedef struct configuration_def
{
    String wifi_ssid;   /* AP SSID */
    String wifi_passwd; /* AP password */

    String bt_if_name; /* Bluetooth name of OBD interface */
    String bt_if_pin;  /* Pin to OBD interface */

    bool display_flip_screen; /* Flip the screen */
} configuration_t;

void config_init();
void config_load(configuration_t *cfg, measurement_t m[]);
bool config_save(configuration_t *cfg, measurement_t m[]);
const char *config_get_page(configuration_t *cfg, measurement_t *m);
#endif