#include <Arduino.h>
#include <Preferences.h>
#include <nvs_flash.h>

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

typedef struct configuration_def
{
    String wifi_ssid;   /* AP SSID */
    String wifi_passwd; /* AP password */

    String bt_if_name; /* Bluetooth name of OBD interface */
    String bt_if_pin;  /* Pin to OBD interface */
} configuration_t;

void config_init();
void config_load(configuration_t *cfg);
bool config_save(configuration_t *cfg);