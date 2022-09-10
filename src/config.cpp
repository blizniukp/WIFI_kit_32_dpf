#include "config.hpp"

Preferences pref;
String config_page;

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

void config_load(configuration_t *cfg, measurement_t m[])
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

  result = pref.begin(CFG_NAMESPACE_DISPLAY, true);
  if (!result)
    Serial.printf("Namespace %s not found. Use default values.\n", CFG_NAMESPACE_DISPLAY);
  cfg->display_flip_screen = pref.getBool(CFG_DISPLAY_FLIP_SCREEN, CFG_DISPLAY_FLIP_SCREEN_DEFAULT);
  pref.end();

  result = pref.begin(CFG_NAMESPACE_MEASUREMENTS, true);
  if (!result)
  {
    Serial.printf("Namespace %s not found. Use default values.\n", CFG_NAMESPACE_MEASUREMENTS);
  }
  for (int i = 0; true; i++)
  {
    if (m[i].id == 0 || m[i].calcFunPtr == NULL)
      break;
    String key = "m_" + String(m[i].id);
    m[i].enabled = pref.getBool(key.c_str(), true);
  }
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
  for (int i = 0; true; i++)
  {
    if (m[i].id == 0 || m[i].calcFunPtr == NULL)
      break;
    Serial.print("m_" + String(m[i].id));
    Serial.printf(": %s\n", m[i].enabled == true ? "true" : "false");
  }
#endif
}

bool config_save(configuration_t *cfg, measurement_t m[])
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

  result = pref.begin(CFG_NAMESPACE_DISPLAY, false);
  if (!result)
  {
    Serial.printf("Can't open namespace: %s\n", CFG_NAMESPACE_DISPLAY);
    return false;
  }
  pref.putBool(CFG_DISPLAY_FLIP_SCREEN, cfg->display_flip_screen);
  pref.end();

  result = pref.begin(CFG_NAMESPACE_MEASUREMENTS, false);
  if (!result)
  {
    Serial.printf("Can't open namespace: %s\n", CFG_NAMESPACE_MEASUREMENTS);
    return false;
  }
  for (int i = 0; true; i++)
  {
    if (m[i].id == 0 || m[i].calcFunPtr == NULL)
      break;
    String key = "m_" + String(m[i].id);
    pref.putBool(key.c_str(), m[i].enabled);
  }
  pref.end();

  return true;
}

static String config_get_page_header()
{
  return R"rawliteral(<html>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<body style='background-color:#161317; color: white'>)rawliteral";
}

static String config_get_page_footer()
{
  return R"rawliteral(<p>Bluetooth serial dump:</p>
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

static String config_get_page_body(configuration_t *cfg, measurement_t *m)
{
  String flip_screen = cfg->display_flip_screen == true ? "checked" : "";
  String body_page = "<form action='/save'>";
  body_page += "<label for='bt_name'>Bluetooth interface name</label><br>";
  body_page += "<input type='text' id='bt_name' name='bt_name' value='" + cfg->bt_if_name + "' maxlength='32'><br>";
  body_page += "<label for='bt_pin'>Pin</label><br>";
  body_page += "<input type='text' id='bt_pin' name='bt_pin' value='" + cfg->bt_if_pin + "'><br>";
  body_page += "<label for='d_flip'>Flip screen vertically</label><br>";
  body_page += "<input type='checkbox' id='d_flip' name='d_flip' value='d_flip' " + flip_screen + " ><br><br>";

  for (int i = 0; true; i++)
  {
    if (m[i].id == 0 || m[i].calcFunPtr == NULL)
      break;
    String key = "m_" + String(m[i].id);
    body_page += "<label><input type='checkbox' " + String(m[i].enabled == true ? "checked" : "") + " name='" + key + "' id='" + key + "' value='" + key + "'>" + String(m[i].caption) + "</label><br>";
  }

  body_page += "<input type='submit' value='Save'>";
  body_page += "<br><br><br>";
  body_page += "<form action='/remove'><input type='submit' value='Remove bonded devices'></form>";
  return body_page;
}

const char *config_get_page(configuration_t *cfg, measurement_t *m)
{
  if (!config_page.isEmpty())
    config_page.clear();

  config_page += config_get_page_header();
  config_page += config_get_page_body(cfg, m);
  config_page += config_get_page_footer();
  return config_page.c_str();
}