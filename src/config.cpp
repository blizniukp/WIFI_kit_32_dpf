#include <vector>
#include <sstream>
#include "config.hpp"

Preferences pref;

void config_init() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    Serial.println("nvs_flash_init error. Erasing flash");
    ESP_ERROR_CHECK(nvs_flash_erase());

    Serial.println("Execute nvs_flash_init");
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

void config_load(configuration_t* cfg, std::vector<measurement_t>& m) {
  bool result = pref.begin(CFG_NAMESPACE_WIFI, true);
  if (!result) {
    Serial.printf("Namespace %s not found. Use default values.\n", CFG_NAMESPACE_WIFI);
  }
  cfg->wifi_ssid = pref.getString(CFG_WIFI_SSID, CFG_WIFI_SSID_DEFAULT);
  cfg->wifi_passwd = pref.getString(CFG_WIFI_PASSWORD, CFG_WIFI_PASSWORD_DEFAULT);
  pref.end();

  result = pref.begin(CFG_NAMESPACE_BT, true);
  if (!result) {
    Serial.printf("Namespace %s not found. Use default values.\n", CFG_NAMESPACE_BT);
  }
  cfg->bt_if_name = pref.getString(CFG_BT_IF_NAME, CFG_BT_IF_NAME_DEFAULT);
  cfg->bt_if_pin = pref.getString(CFG_BT_IF_PIN, CFG_BT_IF_PIN_DEFAULT);
  pref.end();

  result = pref.begin(CFG_NAMESPACE_DISPLAY, true);
  if (!result) {
    Serial.printf("Namespace %s not found. Use default values.\n", CFG_NAMESPACE_DISPLAY);
  }
  cfg->display_flip_screen = pref.getBool(CFG_DISPLAY_FLIP_SCREEN, CFG_DISPLAY_FLIP_SCREEN_DEFAULT);
  pref.end();

  result = pref.begin(CFG_NAMESPACE_MEASUREMENTS, true);
  if (!result) {
    Serial.printf("Namespace %s not found. Use default values.\n", CFG_NAMESPACE_MEASUREMENTS);
  }
  for (uint8_t msmIdx = 0; msmIdx < m.size(); msmIdx++) {
    String key = "m_" + String(m[msmIdx].id);
    m[msmIdx].enabled = pref.getBool(key.c_str(), true);
  }
  pref.end();

  result = pref.begin(CFG_NAMESPACE_PARAMS, true);
  if (!result) {
    Serial.printf("Namespace %s not found. Use default values.\n", CFG_NAMESPACE_PARAMS);
  }
  cfg->temperature_threshold = pref.getFloat(CFG_PARAMS_THRESHOLD, CFG_PARAMS_THRESHOLD_DEFAULT);
  cfg->max_soot_mass = pref.getFloat(CFG_PARAMS_MAX_SHOOT, CFG_PARAMS_MAX_SHOOT_DEFAULT);
  pref.end();

#if DEBUG
  Serial.println("Configuration:\nWiFi:");
  Serial.printf("%s: %s\n", CFG_WIFI_SSID, cfg->wifi_ssid.c_str());
  Serial.printf("%s: %s\n", CFG_WIFI_PASSWORD, cfg->wifi_passwd.c_str());
  Serial.println("Bluetooth:");
  Serial.printf("%s: %s\n", CFG_BT_IF_NAME, cfg->bt_if_name.c_str());
  Serial.printf("%s: %s\n", CFG_BT_IF_PIN, cfg->bt_if_pin.c_str());
  Serial.println("Display:");
  Serial.printf("%s: %s\n", CFG_DISPLAY_FLIP_SCREEN, (cfg->display_flip_screen == true ? "Yes" : "No"));
  Serial.println("Measurements:");
  for (uint8_t msmIdx = 0; msmIdx < m.size(); msmIdx++) {
    Serial.print("m_" + String(m[msmIdx].id));
    Serial.printf(": %s\n", m[msmIdx].enabled == true ? "true" : "false");
  }
  Serial.println("Params:");
  Serial.printf("%s: %.2f\n", CFG_PARAMS_THRESHOLD, cfg->temperature_threshold);
  Serial.printf("%s: %.2f\n", CFG_PARAMS_MAX_SHOOT, cfg->max_soot_mass);
#endif
}

bool config_save(configuration_t* cfg, std::vector<measurement_t>& m) {
  bool result = pref.begin(CFG_NAMESPACE_WIFI, false);
  if (!result) {
    Serial.printf("Can't open namespace: %s\n", CFG_NAMESPACE_WIFI);
    return false;
  }
  pref.putString(CFG_WIFI_SSID, cfg->wifi_ssid);
  pref.putString(CFG_WIFI_PASSWORD, cfg->wifi_passwd);
  pref.end();

  result = pref.begin(CFG_NAMESPACE_BT, false);
  if (!result) {
    Serial.printf("Can't open namespace: %s\n", CFG_NAMESPACE_BT);
    return false;
  }
  pref.putString(CFG_BT_IF_NAME, cfg->bt_if_name);
  pref.putString(CFG_BT_IF_PIN, cfg->bt_if_pin);
  pref.end();

  result = pref.begin(CFG_NAMESPACE_DISPLAY, false);
  if (!result) {
    Serial.printf("Can't open namespace: %s\n", CFG_NAMESPACE_DISPLAY);
    return false;
  }
  pref.putBool(CFG_DISPLAY_FLIP_SCREEN, cfg->display_flip_screen);
  pref.end();

  result = pref.begin(CFG_NAMESPACE_MEASUREMENTS, false);
  if (!result) {
    Serial.printf("Can't open namespace: %s\n", CFG_NAMESPACE_MEASUREMENTS);
    return false;
  }
  for (uint8_t msmIdx = 0; msmIdx < m.size(); msmIdx++) {
    String key = "m_" + String(m[msmIdx].id);
    pref.putBool(key.c_str(), m[msmIdx].enabled);
  }
  pref.end();

  result = pref.begin(CFG_PARAMS_THRESHOLD, false);
  if (!result) {
    Serial.printf("Can't open namespace: %s\n", CFG_PARAMS_THRESHOLD);
    return false;
  }
  pref.putFloat(CFG_PARAMS_THRESHOLD, cfg->temperature_threshold);
  pref.putFloat(CFG_PARAMS_MAX_SHOOT, cfg->max_soot_mass);
  pref.end();

  return true;
}

static void config_get_page_header(std::ostringstream* const configPage) {
  *configPage << R"rawliteral(<html>
  <script type="text/javascript">
        function EnableDisableTextBox(chkbx) {
          switch (chkbx.id) {
            case 'm_5': txtBox = document.getElementById("temp_thr"); break;
            case 'm_8': txtBox = document.getElementById("max_soot"); break;
            default: return
          }
            txtBox.disabled = chkbx.checked ? false : true;
            if (!txtBox.disabled) { txtBox.focus(); }
        }
        function SetTextBoxEnabled() {
          EnableDisableTextBox(document.getElementById("m_5"))
          EnableDisableTextBox(document.getElementById("m_8"))
        }
  </script>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<body style='background-color:#161317; color: white' onload="SetTextBoxEnabled()">)rawliteral";
}

static void config_get_page_footer(std::ostringstream* const configPage) {
  *configPage << R"rawliteral(<p>Bluetooth serial dump:</p>
  <p><textarea name="serial" id="serial" rows="30" cols="40"></textarea></p>
</body>
<script>
  var gateway = `ws://${window.location.hostname}/ws`;
  var websocket;
  function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage; 
  }
  function onOpen(event) {
    console.log('Connection opened');
  }
  function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
  }
  
  function onMessage(event) {
    var txtArea = document.getElementById('serial');
    txtArea.innerHTML = txtArea.innerHTML + event.data;
    txtArea.scrollTop = txtArea.scrollHeight;
  }
  
  function onLoad(event) {
    initWebSocket();
  }
  function toggle(){
    websocket.send('toggle');
  }
  
  window.addEventListener('load', onLoad);
</script>
</html>)rawliteral";
}

static void config_get_page_body(std::ostringstream* const configPage, configuration_t* const cfg, std::vector<measurement_t>* const m) {
  String flip_screen = cfg->display_flip_screen == true ? "checked" : "";
  *configPage << "<form action='/save'>";
  *configPage << "<label for='bt_name'>Bluetooth interface name</label><br>";
  *configPage << "<input type='text' id='bt_name' name='bt_name' value='" << cfg->bt_if_name.c_str() << "' maxlength='32'><br>";
  *configPage << "<label for='bt_pin'>Pin</label><br>";
  *configPage << "<input type='text' id='bt_pin' name='bt_pin' value='" << cfg->bt_if_pin.c_str() << "'><br>";
  *configPage << "<label for='d_flip'>Flip screen vertically</label><br>";
  *configPage << "<input type='checkbox' id='d_flip' name='d_flip' value='d_flip' " << flip_screen.c_str() << " ><br><br>";

  for (uint8_t msmIdx = 0; msmIdx < m->size(); msmIdx++) {
    measurement_t* msm = &(*m)[msmIdx];
    String key = "m_" + String(msm->id);
    *configPage << "<label>";
    *configPage << "<input type='checkbox' " << (msm->enabled == true ? "checked" : "") << " name='" << key.c_str() << "'";
    *configPage << " id='" << key.c_str() << "' value='" << key.c_str() << "' " << (msm->dataReadFunPtr != NULL ? "onclick='EnableDisableTextBox(this)'" : "") << " >";
    *configPage << msm->caption;
    *configPage << "</label><br>";
  }

  *configPage << "<br><br>";
  *configPage << "<label for='temp_thr'>Temperature threshold to activates the buzzer alarm.</label><br>";
  *configPage << "<input type='text' id='temp_thr' name='temp_thr' value='" << cfg->temperature_threshold << "' disabled='disabled' ><br>";
  *configPage << "<label for='max_soot'>Maximum soot load (used to calculate the percentage)</label><br>";
  *configPage << "<input type='text' id='max_soot' name='max_soot' value='" << cfg->max_soot_mass << "' disabled='disabled' ><br>";

  *configPage << "<input type='submit' value='Save'></form>";
  *configPage << "<br><br><br>";
  *configPage << "<form action='/remove'><input type='submit' value='Remove bonded devices'></form>";
}

void config_get_page(std::ostringstream* const configPage, configuration_t* const cfg, std::vector<measurement_t>* const m) {
  config_get_page_header(configPage);
  config_get_page_body(configPage, cfg, m);
  config_get_page_footer(configPage);
}