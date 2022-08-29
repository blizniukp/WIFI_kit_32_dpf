#include "config.hpp"

Preferences pref;

void config_init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        Serial.println("nvs_flash_init error. Erasing flash");
        ESP_ERROR_CHECK(nvs_flash_erase());

        Serial.println("Execute nvs_flash_init");
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);
}

void config_load(configuration_t *cfg)
{
    bool result = pref.begin(CFG_NAMESPACE_WIFI, true);
    if (!result)
        Serial.printf("Namespace %s not found. Use default values.\n", CFG_NAMESPACE_WIFI);
    cfg->wifi_ssid = pref.getString(CFG_WIFI_SSID, CFG_WIFI_SSID_DEFAULT);
    cfg->wifi_passwd = pref.getString(CFG_WIFI_PASSWORD, CFG_WIFI_PASSWORD_DEFAULT);
    pref.end();

    result = pref.begin(CFG_NAMESPACE_BT, true);
    if (!result)
        Serial.printf("Namespace %s not found. Use default values.\n", CFG_NAMESPACE_BT);
    cfg->bt_if_name = pref.getString(CFG_BT_IF_NAME, CFG_BT_IF_NAME_DEFAULT);
    cfg->bt_if_pin = pref.getString(CFG_BT_IF_PIN, CFG_BT_IF_PIN_DEFAULT);
    pref.end();

#if DEBUG
    Serial.println("Configuration:\nWiFi:");
    Serial.printf("%s: %s\n", CFG_WIFI_SSID, cfg->wifi_ssid.c_str());
    Serial.printf("%s: %s\n", CFG_WIFI_PASSWORD, cfg->wifi_passwd.c_str());
    Serial.println("Bluetooth:");
    Serial.printf("%s: %s\n", CFG_BT_IF_NAME, cfg->bt_if_name.c_str());
    Serial.printf("%s: %s\n", CFG_BT_IF_PIN, cfg->bt_if_pin.c_str());
#endif
}

bool config_save(configuration_t *cfg)
{
    bool result = pref.begin(CFG_NAMESPACE_WIFI, false);
    if (!result)
    {
        Serial.printf("Can't open namespace: %s\n", CFG_NAMESPACE_WIFI);
        return false;
    }
    pref.putString(CFG_WIFI_SSID, cfg->wifi_ssid);
    pref.putString(CFG_WIFI_PASSWORD, cfg->wifi_passwd);
    pref.end();

    result = pref.begin(CFG_NAMESPACE_BT, false);
    if (!result)
    {
        Serial.printf("Can't open namespace: %s\n", CFG_NAMESPACE_BT);
        return false;
    }
    pref.putString(CFG_BT_IF_NAME, cfg->bt_if_name);
    pref.putString(CFG_BT_IF_PIN, cfg->bt_if_pin);
    pref.end();
    return true;
}