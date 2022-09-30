#include "main.hpp"

static const uint8_t max_log_lines = 60;
static const uint8_t last_line = 56;
static const uint8_t max_bt_response_time = 10; /*10 seconds*/
static const uint8_t max_bt_paired_devices = 20;
static const uint8_t remove_bonded_devices = 1;
static const uint8_t rx_buffer_size = 128;

BluetoothSerial btSerial;
SSD1306Wire* display;

#ifdef ENABLE_WIFI
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
#endif

bool connected = false;
bool removeBondedDevices = false;
uint8_t logLineNumber = 0;
bool connectByName = true;

bool calcFun_AB(char* data, size_t data_len, float* val, float divider);
bool calcFun_ABCD(char* data, size_t data_len, float* val, float divider);
bool calcFun_Temperature(char* data, size_t data_len, float* val, float divider);
void dataReadFun_Temperature(float value);

measurement_t measurements[] = {
    {1, "Soot mass measured", "22114E1\r", "g", -100.0f, &calcFun_AB, 100.0f, true, NULL},
    {2, "Soot mass calculated", "22114F1\r", "g", -100.0f, &calcFun_AB, 100.0f, true, NULL},
    {3, "Distance since last regen.", "221156\r", "km", -100.0f, &calcFun_ABCD, 1000.0f, true, NULL},
    {4, "Time since last regen", "22115E\r", "min", -100.0f, &calcFun_ABCD, 60.0f, false, NULL},
    {5, "Input temperature", "2211B2\r", "*C", -100.0f, &calcFun_Temperature, 10.0f, true, &dataReadFun_Temperature},
    {6, "Outptu temperature", "2210F9\r", "*C", -100.0f, &calcFun_Temperature, 10.0f, false, NULL},
    {7, "Oil Ash Residue", "22178C\r", "g", -100.0f, &calcFun_AB, 10.0f, false, NULL},
    {0, "", "", "", 0.0f, NULL, 0.0f, false},
};

uint8_t measurementIndex = 0;

configuration_t config;

void initDisplay() {
  display = new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);
  display->init();
  display->setFont(ArialMT_Plain_10);
}

void displayText(uint8_t x, uint8_t y, String text) {
  display->drawString(x, y, text);
  display->display();
}

void clearDisplay() {
  display->clear();
  logLineNumber = 0;
}

void addToLog(String text) {
  if (logLineNumber >= max_log_lines) {
    clearDisplay();
  }

  displayText(0, logLineNumber, text);
  Serial.println(text);

  logLineNumber += 10;
}

void addBtResponseToSerialLog(String text) {
  Serial.printf("<[%d] %s\n", text.length(), text);
}

void addBtCommandToSerialLog(String text) {
  Serial.printf(">[%d] %s\n", text.length(), text);
}

void addResultToLog(bool result) {
  if (result == true) {
    addToLog(" done");
  }
  else {
    addToLog(" ERROR!");
  }
}

bool connect() {
  bool result = false;
  /*
  Connecting by name does not always work well.
  I had a problem with connecting to "OBDII" interface.
  The get_name_from_eir function (in BluetoothSerial.cpp file) returns an incorrect device name length (peer_bdname_len field).
  So, this is a workaround.*/
  if (connectByName) {
    addToLog("Connecting to: " + config.bt_if_name);
    result = btSerial.connect(config.bt_if_name);
    connectByName = false;
  }
  else {
    BTScanResults* btDeviceList = btSerial.getScanResults();
    if (btDeviceList->getCount() > 0) {
      BTAddress addr;
      Serial.printf("Found %d devices\n", btDeviceList->getCount());
      for (uint8_t deviceIdx = 0; deviceIdx < btDeviceList->getCount(); deviceIdx++) {
        BTAdvertisedDevice* device = btDeviceList->getDevice(deviceIdx);
        Serial.printf(" -- Address: %s, Name: %s\n", device->getAddress().toString().c_str(), String(device->getName().c_str()));
        if (device->getName().compare(config.bt_if_name.c_str()) == 0) {
          addToLog("Connecting to: " + String(device->getAddress().toString().c_str()));
          result = btSerial.connect(device->getAddress());
        }
      }
    }
    connectByName = true;
  }

  return result;
}

void printDeviceStatus() {
  Serial.printf("Connected: %s\n", (connected == true ? "Y" : "N"));
}

size_t btSerialRead(char* buffer, uint16_t timeout = 1500) {
  size_t data_len = 0;
  char c = '\0';
  buffer[0] = '\0';
  unsigned long btSerialReadTimeout = millis() + (max_bt_response_time * 1000) + timeout;

  do {
    if (btSerial.available() > 0) {
      c = btSerial.read();
      if ((c != '>') && (c != '\r') && (c != '\n')) {
        buffer[data_len++] = c;
      }
      if (data_len > rx_buffer_size) {
        data_len = 0;
      }
    }
    if (btSerialReadTimeout < millis()) {
      Serial.println("BtSerial read timeout error");
      break;
    }
  } while (c != '>');
  buffer[data_len + 1] = '\0';

#ifdef ENABLE_WIFI
  ws.textAll("R:");
  ws.textAll(buffer);
  ws.textAll("\n");
#endif
  return data_len;
}

size_t btSerialReadAndAddToLog(char* buffer, uint16_t timeout = 1500) {
  size_t rxLen = btSerialRead(buffer, timeout);
  addBtResponseToSerialLog(String(buffer));
  return rxLen;
}

void btSerialSendCommand(String command) {
  btSerial.flush();
  addBtCommandToSerialLog(command);
  btSerial.print(command);
#ifdef ENABLE_WIFI
  ws.textAll("W:");
  ws.textAll(command.c_str(), command.length());
  ws.textAll("\n");
#endif
}

void btSerialInit(char* buffer) {
  addToLog("OBD initialization...");
  btSerialSendCommand("ATZ\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("ATE0\r");
  btSerialReadAndAddToLog(buffer);

  btSerialSendCommand("STI\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("VTI\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("ATD\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("ATE0\r");
  btSerialReadAndAddToLog(buffer);

  btSerialSendCommand("ATSP0\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("ATE0\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("ATH1\r");
  btSerialReadAndAddToLog(buffer);

  btSerialSendCommand("ATM0\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("ATS0\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("ATAT1\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("ATAL\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("ATST64\r");
  btSerialReadAndAddToLog(buffer);

  btSerialSendCommand("0100\r"); // SEARCHING...
  btSerialReadAndAddToLog(buffer, 5000);

  btSerialSendCommand("ATDPN\r");
  btSerialReadAndAddToLog(buffer);

  btSerialSendCommand("ATSH7E0\r");
  btSerialReadAndAddToLog(buffer);
  btSerialSendCommand("10031\r");
  btSerialReadAndAddToLog(buffer);
}

void btSerialCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
  switch (event) {
  case ESP_SPP_CLOSE_EVT:
  {
    addToLog("Disconnected!");
    connected = false;
  }
  break;
  default:

    break;
  }
}

bool isCanError(char* response) {
  char* ret = NULL;
  ret = strstr(response, "SEARCHING");
  if (ret) {
    return true;
  }

  ret = strstr(response, "CAN ERROR");
  if (ret) {
    return true;
  }

  ret = strstr(response, "STOPPED");
  if (ret) {
    return true;
  }

  ret = strstr(response, "UNABLE");
  if (ret) {
    return true;
  }

  if (response[0] == '\0') {
    return true;
  }

  return false;
}

int32_t getByteFromData(char* data, size_t data_len, uint8_t index) {
  char buffer[3] = { 0, 0, 0 };
  buffer[0] = data[index];
  buffer[1] = data[index + 1];
  return (strtol(buffer, NULL, 16));
}

bool calcFun_AB(char* data, size_t data_len, float* val, float divider) {
#ifdef RANDOM_DATA
  * val = random(1, 100) / divider;
  return (bool)random(0, 2);
#endif
  if (isCanError(data)) {
    *val = -100.0f;
    return false;
  }
  *val = (((getByteFromData(data, data_len, 11) * 256) + (getByteFromData(data, data_len, 13))) / divider);
  return true;
}

bool calcFun_ABCD(char* data, size_t data_len, float* val, float divider) {
#ifdef RANDOM_DATA
  * val = random(1, 100) / divider;
  return (bool)random(0, 2);
#endif
  if (isCanError(data)) {
    *val = -100.0f;
    return false;
  }
  *val = (((getByteFromData(data, data_len, 11) * 16777216) + (getByteFromData(data, data_len, 13) * 65536) + (getByteFromData(data, data_len, 15) * 256) + (getByteFromData(data, data_len, 17))) / divider);
  return true;
}

bool calcFun_Temperature(char* data, size_t data_len, float* val, float divider) {
#ifdef RANDOM_DATA
  * val = random(1, 100) / divider;
  return (bool)random(0, 2);
#endif
  if (isCanError(data)) {
    *val = -100.0f;
    return false;
  }
  *val = ((((getByteFromData(data, data_len, 11) * 256) + (getByteFromData(data, data_len, 13))) - 2731) / divider);
  return true;
}

void dataReadFun_Temperature(float value) {
  buzzer_set_temperature(value);
}

void displayData(bool correctData, measurement_t* m) {
  uint8_t xpos = 0, ypos = 0;

  clearDisplay();
  display->setFont(ArialMT_Plain_16);
  displayText(xpos, ypos, m->caption);

  display->setFont(ArialMT_Plain_24);
  if (correctData) {
    displayText(xpos, ypos + 25, String(m->value, 2) + " " + String(m->unit));
  }
  else {
    displayText(xpos, ypos + 25, "???");
  }

  display->setFont(ArialMT_Plain_10);
  displayText(115, last_line - 5, (correctData == true ? "V" : "X"));
  Serial.printf("caption: %s, isCorrectData: %s, value: %.2f\n", m->caption, (correctData == true ? "Y" : "N"), m->value);
  Serial.println("========================");
}

void drawProgressBar() {
  displayText(0, last_line, "[");
  displayText(100, last_line, "]");

  for (uint8_t columnIdx = 1; columnIdx < 100; columnIdx++) {
    displayText(columnIdx, last_line - 3, ".");
    delay(50);
  }
}

#ifdef ENABLE_WIFI
#ifdef DEBUG
void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client %u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client %u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}
#endif

void handleRoot(AsyncWebServerRequest* request) {
  request->send_P(200, "text/html", config_get_page(&config, measurements));
}

bool initBluetooth() {
  if (btStarted()) {
    btStop();
  }

  if (!btStart()) {
    Serial.println("Failed to initialize controller");
    return false;
  }

  esp_err_t espErr = esp_bluedroid_init();
  if (espErr != ESP_OK) {
    Serial.printf("Failed to initialize bluedroid: %s\n", esp_err_to_name(espErr));
    return false;
  }

  espErr = esp_bluedroid_enable();
  if (esp_bluedroid_enable() != ESP_OK) {
    Serial.printf("Failed to enable bluedroid: %s\n", esp_err_to_name(espErr));
    return false;
  }
  return true;
}

char* bda2str(const uint8_t* bda, char* str, size_t size) {
  if (bda == NULL || str == NULL || size < 18) {
    return NULL;
  }
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
    bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
  return str;
}

void handleRemove(AsyncWebServerRequest* request) {
  removeBondedDevices = true;
  request->send_P(200, "text/html", config_get_page(&config, measurements));
}

void handleSave(AsyncWebServerRequest* request) {
  Serial.println("Got handleSave");
  if (request->hasParam(CFG_BT_IF_NAME)) {
    config.bt_if_name = request->getParam(CFG_BT_IF_NAME)->value();
  }
  if (request->hasParam(CFG_BT_IF_PIN)) {
    config.bt_if_pin = request->getParam(CFG_BT_IF_PIN)->value();
  }
  if (request->hasParam(CFG_DISPLAY_FLIP_SCREEN)) {
    config.display_flip_screen = request->getParam(CFG_DISPLAY_FLIP_SCREEN)->value() == CFG_DISPLAY_FLIP_SCREEN ? true : false;
  }

  for (uint8_t msmIdx = 0; true; msmIdx++) {
    if (measurements[msmIdx].id == 0 || measurements[msmIdx].calcFunPtr == NULL) {
      break;
    }
    String pname = "m_" + String(measurements[msmIdx].id);
    if (request->hasParam(pname)) {
      measurements[msmIdx].enabled = true;
    }
    else {
      measurements[msmIdx].enabled = false;
    }
  }

  Serial.println("Save configuration");
  config_save(&config, measurements);

  Serial.println("Restart device");
  ESP.restart();
}

void initWebserver() {
  WiFi.softAP(config.wifi_ssid.c_str(), config.wifi_passwd.c_str());
  IPAddress myIP = WiFi.softAPIP();
  addToLog("AP IP address: ");
  addToLog(myIP.toString());
  server.on("/", HTTP_GET, handleRoot);
  server.on("/remove", HTTP_GET, handleRemove);
  server.on("/save", HTTP_GET, handleSave);
#ifdef DEBUG
  ws.onEvent(onEvent);
#endif
  server.addHandler(&ws);
  server.begin();
}
#endif

void deleteBondedDevices() {
  char bdaStr[18];
  uint8_t pairedDeviceBtAddr[max_bt_paired_devices][6];

  initBluetooth();
  int32_t count = esp_bt_gap_get_bond_device_num();
  if (!count) {
    Serial.println("No bonded device found.");
  }
  else {
    Serial.printf("Bonded device count: %d\n", count);
    if (max_bt_paired_devices < count) {
      count = max_bt_paired_devices;
      Serial.printf("Reset bonded device count: %d\n", count);
    }
    esp_err_t espErr = esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
    if (espErr == ESP_OK) {
      for (int16_t deviceIdx = 0; deviceIdx < count; deviceIdx++) {
        Serial.printf("Found bonded device # %d -> %s\n", deviceIdx, bda2str(pairedDeviceBtAddr[deviceIdx], bdaStr, 18));
        espErr = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[deviceIdx]);
        if (espErr == ESP_OK) {
          Serial.printf("Removed bonded device # %d\n", deviceIdx);
        }
        else {
          Serial.printf("Failed to remove bonded device # %d\n", deviceIdx);
        }
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  bool result = false;

  initDisplay();
  addToLog("Display initialized");

  addToLog("Init configuration...");
  config_init();

  addToLog("Load configuration...");
  config_load(&config, measurements);

  if (config.display_flip_screen) {
    display->flipScreenVertically();
  }

  addToLog("Enable buzzer");
  buzzer_init(BUZZER_PIN);
  buzzer_set_threshold(config.temperature_threshold);

  addToLog("Setup bluetooth...");
  result = btSerial.begin(CFG_DEVICE_NAME_DEFAULT, true);
  addResultToLog(result);

  addToLog("Set bluetooth PIN...");
  result = btSerial.setPin(config.bt_if_pin.c_str());
  addResultToLog(result);

  btSerial.enableSSP();

  addToLog("Set btSerial callback...");
  result = btSerial.register_callback(btSerialCallback);
  addResultToLog(result == ESP_OK ? true : false);

  addToLog("Is bluetooth ready ...");
  result = btSerial.isReady(true, 0);
  addResultToLog(result);

#ifdef ENABLE_WIFI
  initWebserver();
#endif

#ifdef RANDOM_DATA
  connected = true;
#endif
}

void loop() {
  char rxData[rx_buffer_size];
  size_t rxLen = 0;

#ifdef ENABLE_WIFI
  ws.cleanupClients();

  if (removeBondedDevices) {
    removeBondedDevices = false;
    deleteBondedDevices();
  }
#endif

  printDeviceStatus();

  if (!connected) {
    clearDisplay();

    connected = connect();
    addResultToLog(connected);

    if (!connected) {
      delay(50);
      return;
    }

    addToLog("Connected");
    btSerialInit(rxData);
    measurementIndex = 0;
  }

  if (!connected) {
    delay(50);
    return;
  }

  measurement_t* m = &measurements[measurementIndex];
  btSerialSendCommand(m->command);
  rxLen = btSerialReadAndAddToLog(rxData);
  bool correctData = m->calcFunPtr(rxData, rxLen, &m->value, m->divider);
  if (correctData && m->dataReadFunPtr) {
    m->dataReadFunPtr(m->value);
  }

  Serial.println("Display data");
  displayData(correctData, m);

  drawProgressBar();
  do {
    measurementIndex++;
    if (measurements[measurementIndex].id == 0 ||
      measurements[measurementIndex].calcFunPtr == NULL) {
      measurementIndex = 0;
    }
  } while (!measurements[measurementIndex].enabled);
}