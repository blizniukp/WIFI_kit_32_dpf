#include "main.hpp"

BluetoothSerial btSerial;
SSD1306Wire *display;

#ifdef ENABLE_WIFI
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
#endif

bool connected = false;
bool removeBondedDevices = false;
int logLineNumber = 0;
char rxData[READ_BUFFER_SIZE];
uint8_t rxIndex = 0;
bool connectByName = true;
uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
char bdaStr[18];

bool calcFun_AB(char *command, float *val, float divider);
bool calcFun_ABCD(char *command, float *val, float divider);
bool calcFun_Temperature(char *command, float *val, float divider);
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

int measurementIndex = 0;

configuration_t config;

void initDisplay()
{
  display = new SSD1306Wire(0x3c, SDA_OLED, SCL_OLED, RST_OLED, GEOMETRY_128_64);
  display->init();
  display->setFont(ArialMT_Plain_10);
}

void displayText(int x, int y, String text)
{
  display->drawString(x, y, text);
  display->display();
}

void clearDisplay()
{
  display->clear();
  logLineNumber = 0;
}

void addToLog(String text)
{
  if (logLineNumber >= MAX_LOG_LINES)
  {
    clearDisplay();
  }

  displayText(0, logLineNumber, text);
  Serial.println(text);

  logLineNumber += 10;
}

void addBtResponseToSerialLog(String text)
{
  Serial.printf("< %s\n", text);
}

void addBtCommandToSerialLog(String text)
{
  Serial.printf("> %s\n", text);
}

void addResultToLog(bool result)
{
  if (result == true)
    addToLog(" done");
  else
    addToLog(" ERROR!");
}

bool connect()
{
  bool result = false;
  /*
  Connecting by name does not always work well.
  I had a problem with connecting to "OBDII" interface.
  The get_name_from_eir function (in BluetoothSerial.cpp file) returns an incorrect device name length (peer_bdname_len field).
  So, this is a workaround.*/
  if (connectByName == true)
  {
    addToLog("Connecting to: " + config.bt_if_name);
    result = btSerial.connect(config.bt_if_name);
    connectByName = false;
  }
  else
  {
    BTScanResults *btDeviceList = btSerial.getScanResults();
    if (btDeviceList->getCount() > 0)
    {
      BTAddress addr;
      Serial.printf("Found %d devices\n", btDeviceList->getCount());
      for (int i = 0; i < btDeviceList->getCount(); i++)
      {
        BTAdvertisedDevice *device = btDeviceList->getDevice(i);
        Serial.printf(" -- Address: %s, Name: %s\n", device->getAddress().toString().c_str(), String(device->getName().c_str()));
        if (strcmp(config.bt_if_name.c_str(), device->getName().c_str()) == 0)
        {
          addToLog("Connecting to: " + String(device->getAddress().toString().c_str()));
          result = btSerial.connect(device->getAddress());
        }
      }
    }
    connectByName = true;
  }

  return result;
}

void printDeviceStatus()
{
  Serial.printf("Connected: %s\n", (connected == true ? "Y" : "N"));
}

void btSerialRead(int timeout = 1500)
{
  char c = '\0';
  rxData[0] = '\0';
  unsigned long btSerialReadTimeout = millis() + (MAX_BT_RESPONSE_TIME * 1000) + timeout;

  do
  {
    if (btSerial.available() > 0)
    {
      c = btSerial.read();
      if ((c != '>') && (c != '\r') && (c != '\n'))
      {
        rxData[rxIndex++] = c;
      }
      if (rxIndex > READ_BUFFER_SIZE)
      {
        rxIndex = 0;
      }
    }
    if (btSerialReadTimeout < millis())
    {
      Serial.println("BtSerial read timeout error");
      break;
    }
  } while (c != '>');
  rxData[rxIndex++] = '\0';
  rxIndex = 0;

#ifdef ENABLE_WIFI
  ws.textAll("R:");
  ws.textAll(rxData);
  ws.textAll("\n");
#endif
}

void btSerialReadAndAddToLog(int timeout = 1500)
{
  btSerialRead(timeout);
  addBtResponseToSerialLog(String(rxData));
}

void btSerialSendCommand(String command)
{
  btSerial.flush();
  addBtCommandToSerialLog(command);
  btSerial.print(command);
#ifdef ENABLE_WIFI
  ws.textAll("W:");
  ws.textAll(command.c_str(), command.length());
  ws.textAll("\n");
#endif
}

void btSerialInit()
{
  addToLog("OBD initialization...");
  btSerialSendCommand("ATZ\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATE0\r");
  btSerialReadAndAddToLog();

  btSerialSendCommand("STI\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("VTI\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATD\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATE0\r");
  btSerialReadAndAddToLog();

  btSerialSendCommand("ATSP0\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATE0\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATH1\r");
  btSerialReadAndAddToLog();

  btSerialSendCommand("ATM0\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATS0\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATAT1\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATAL\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATST64\r");
  btSerialReadAndAddToLog();

  btSerialSendCommand("0100\r"); // SEARCHING...
  btSerialReadAndAddToLog(5000);

  btSerialSendCommand("ATDPN\r");
  btSerialReadAndAddToLog();

  btSerialSendCommand("ATSH7E0\r");
  btSerialReadAndAddToLog();
  btSerialSendCommand("10031\r");
  btSerialReadAndAddToLog();
}

void btSerialCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  switch (event)
  {
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

bool isReadCanError()
{
  char *ret = NULL;
  ret = strstr(rxData, "SEARCHING");
  if (ret)
    return true;

  ret = strstr(rxData, "CAN ERROR");
  if (ret)
    return true;

  ret = strstr(rxData, "STOPPED");
  if (ret)
    return true;

  ret = strstr(rxData, "UNABLE");
  if (ret)
    return true;

  if (rxData[0] == '\0')
    return true;

  return false;
}

int getByteFromData(int index)
{
  char buffer[3] = {0, 0, 0};
  buffer[0] = rxData[index];
  buffer[0 + 1] = rxData[index + 1];
  return (strtol(&buffer[0], NULL, 16));
}

bool calcFun_AB(char *command, float *val, float divider)
{
#ifdef RANDOM_DATA
  *val = random(1, 100) / divider;
  return (bool)random(0, 2);
#endif
  btSerialSendCommand(command);
  btSerialReadAndAddToLog();
  if (isReadCanError())
  {
    *val = -100.0f;
    return false;
  }
  *val = (((getByteFromData(11) * 256) + (getByteFromData(13))) / divider);
  return true;
}

bool calcFun_ABCD(char *command, float *val, float divider)
{
#ifdef RANDOM_DATA
  *val = random(1, 100) / divider;
  return (bool)random(0, 2);
#endif
  btSerialSendCommand(command);
  btSerialReadAndAddToLog();
  if (isReadCanError())
  {
    *val = -100.0f;
    return false;
  }
  *val = (((getByteFromData(11) * 16777216) + (getByteFromData(13) * 65536) + (getByteFromData(15) * 256) + (getByteFromData(17))) / divider);
  return true;
}

bool calcFun_Temperature(char *command, float *val, float divider)
{
#ifdef RANDOM_DATA
  *val = random(1, 100) / divider;
  return (bool)random(0, 2);
#endif
  btSerialSendCommand(command);
  btSerialReadAndAddToLog();
  if (isReadCanError())
  {
    *val = -100.0f;
    return false;
  }
  *val = ((((getByteFromData(11) * 256) + (getByteFromData(13))) - 2731) / divider);
  return true;
}

void dataReadFun_Temperature(float value)
{
  buzzer_set_temperature(value);
}

void displayData(bool correctData, measurement_t *m)
{
  int xpos = 0, ypos = 0;

  clearDisplay();
  display->setFont(ArialMT_Plain_16);
  displayText(xpos, ypos, m->caption);

  display->setFont(ArialMT_Plain_24);
  if (correctData)
    displayText(xpos, ypos + 25, String(m->value, 2) + " " + String(m->unit));
  else
    displayText(xpos, ypos + 25, "???");

  display->setFont(ArialMT_Plain_10);
  displayText(115, LAST_LINE - 5, (correctData == true ? "V" : "X"));
  Serial.printf("caption: %s, isCorrectData: %s, value: %.2f\n", m->caption, (correctData == true ? "Y" : "N"), m->value);
  Serial.println("========================");
}

void drawProgressBar()
{
  displayText(0, LAST_LINE, "[");
  displayText(100, LAST_LINE, "]");

  for (int i = 1; i < 100; i++)
  {
    displayText(i, LAST_LINE - 3, ".");
    delay(50);
  }
}

#ifdef ENABLE_WIFI
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
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

void handleRoot(AsyncWebServerRequest *request)
{
  request->send_P(200, "text/html", config_get_page(&config, measurements));
}

bool initBluetooth()
{
  if (btStarted())
  {
    btStop();
  }

  if (!btStart())
  {
    Serial.println("Failed to initialize controller");
    return false;
  }

  esp_err_t espErr = esp_bluedroid_init();
  if (espErr != ESP_OK)
  {
    Serial.printf("Failed to initialize bluedroid: %s\n", esp_err_to_name(espErr));
    return false;
  }

  espErr = esp_bluedroid_enable();
  if (esp_bluedroid_enable() != ESP_OK)
  {
    Serial.printf("Failed to enable bluedroid: %s\n", esp_err_to_name(espErr));
    return false;
  }
  return true;
}

char *bda2str(const uint8_t *bda, char *str, size_t size)
{
  if (bda == NULL || str == NULL || size < 18)
  {
    return NULL;
  }
  sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
          bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
  return str;
}

void handleRemove(AsyncWebServerRequest *request)
{
  removeBondedDevices = true;
  request->send_P(200, "text/html", config_get_page(&config, measurements));
}

void handleSave(AsyncWebServerRequest *request)
{
  Serial.println("Got handleSave");
  if (request->hasParam(CFG_BT_IF_NAME))
    config.bt_if_name = request->getParam(CFG_BT_IF_NAME)->value();
  if (request->hasParam(CFG_BT_IF_PIN))
    config.bt_if_pin = request->getParam(CFG_BT_IF_PIN)->value();
  if (request->hasParam(CFG_DISPLAY_FLIP_SCREEN))
    config.display_flip_screen = request->getParam(CFG_DISPLAY_FLIP_SCREEN)->value() == CFG_DISPLAY_FLIP_SCREEN ? true : false;

  for (int i = 0; true; i++)
  {
    if (measurements[i].id == 0 || measurements[i].calcFunPtr == NULL)
      break;
    String pname = "m_" + String(measurements[i].id);
    if (request->hasParam(pname))
      measurements[i].enabled = true;
    else
      measurements[i].enabled = false;
  }

  Serial.println("Save configuration");
  config_save(&config, measurements);

  Serial.println("Restart device");
  ESP.restart();
}

void initWebserver()
{
  WiFi.softAP(config.wifi_ssid.c_str(), config.wifi_passwd.c_str());
  IPAddress myIP = WiFi.softAPIP();
  addToLog("AP IP address: ");
  addToLog(myIP.toString());
  server.on("/", HTTP_GET, handleRoot);
  server.on("/remove", HTTP_GET, handleRemove);
  server.on("/save", HTTP_GET, handleSave);
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
}
#endif

void deleteBondedDevices()
{
  initBluetooth();
  int count = esp_bt_gap_get_bond_device_num();
  if (!count)
  {
    Serial.println("No bonded device found.");
  }
  else
  {
    Serial.printf("Bonded device count: %d\n", count);
    if (PAIR_MAX_DEVICES < count)
    {
      count = PAIR_MAX_DEVICES;
      Serial.printf("Reset bonded device count: %d\n", count);
    }
    esp_err_t tError = esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
    if (ESP_OK == tError)
    {
      for (int i = 0; i < count; i++)
      {
        Serial.printf("Found bonded device # %d -> %s\n", i, bda2str(pairedDeviceBtAddr[i], bdaStr, 18));
        if (REMOVE_BONDED_DEVICES)
        {
          tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
          if (ESP_OK == tError)
            Serial.printf("Removed bonded device # %d\n", i);
          else
            Serial.printf("Failed to remove bonded device # %d\n", i);
        }
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  bool result = false;

  initDisplay();
  addToLog("Display initialized");

  addToLog("Init configuration...");
  config_init();

  addToLog("Load configuration...");
  config_load(&config, measurements);

  if (config.display_flip_screen)
    display->flipScreenVertically();

  addToLog("Enable buzzer");
  buzzer_init(BUZZER_PIN);
  buzzer_set_threshold(config.temperautre_threshold);

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

void loop()
{
#ifdef ENABLE_WIFI
  ws.cleanupClients();

  if (removeBondedDevices)
  {
    removeBondedDevices = false;
    deleteBondedDevices();
  }
#endif

  printDeviceStatus();

  if (!connected)
  {
    clearDisplay();

    connected = connect();
    addResultToLog(connected);

    if (!connected)
    {
      delay(50);
      return;
    }

    addToLog("Connected");
    btSerialInit();
    measurementIndex = 0;
  }

  if (!connected)
  {
    delay(50);
    return;
  }

  measurement_t *m = &measurements[measurementIndex];
  bool correctData = m->calcFunPtr(m->command, &m->value, m->divider);

  if (correctData && m->dataReadFunPtr)
    m->dataReadFunPtr(m->value);

  Serial.println("Display data");
  displayData(correctData, m);

  drawProgressBar();
  do
  {
    measurementIndex++;
    if (measurements[measurementIndex].id == 0 ||
        measurements[measurementIndex].calcFunPtr == NULL)
    {
      measurementIndex = 0;
    }
  } while (!measurements[measurementIndex].enabled);
}