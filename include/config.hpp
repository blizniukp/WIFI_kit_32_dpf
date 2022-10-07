#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <stdbool.h>
#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>
#include <vector>
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

#define CFG_NAMESPACE_PARAMS "params"
#define BUZZER_PIN 12
#define CFG_PARAMS_THRESHOLD "temp_thr"
#define CFG_PARAMS_THRESHOLD_DEFAULT 400.0f
#define CFG_PARAMS_MAX_SHOOT "max_soot"
#define CFG_PARAMS_MAX_SHOOT_DEFAULT 27.0f

typedef struct configuration_def {
    String wifi_ssid;   /* AP SSID */
    String wifi_passwd; /* AP password */

    String bt_if_name; /* Bluetooth name of OBD interface */
    String bt_if_pin;  /* Pin to OBD interface */

    bool display_flip_screen; /* Flip the screen */

    float temperature_threshold; /*The temperature threshold that activates the buzzer alarm.*/
    float max_soot_mass; /*Maximum value of "soot mass". Used to calculate the percentage value.*/
} configuration_t;

void config_init();
void config_load(configuration_t* cfg, std::vector<measurement_t>& m);
bool config_save(configuration_t* cfg, std::vector<measurement_t>& m);
void config_get_page(std::ostringstream* const configPage, configuration_t* cfg, std::vector<measurement_t>* m);
#endif