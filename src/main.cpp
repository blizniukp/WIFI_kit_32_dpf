#include "main.hpp"

BluetoothSerial btSerial;
SSD1306Wire *display;

#ifdef ENABLE_WIFI
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
#endif

String bluetoothName = "WIFI_kit_32_dpf";
String bluetoothPin = "1234";
String defaultObdIfName = "V-LINK";
const int READ_BUFFER_SIZE = 128;

bool connected = false;
bool removeBondedDevices = false;
int logLineNumber = 0;
char rxData[READ_BUFFER_SIZE];
uint8_t rxIndex = 0;
bool connectByName = true;
uint8_t pairedDeviceBtAddr[PAIR_MAX_DEVICES][6];
char bda_str[18];

float smm_value = -100.0f;   /*Soot mass measured*/
float smc_value = -100.0f;   /*Soot mass calculated*/
float dslr_value = -100.0f;  /*Distance since last regeneration*/
float tslr_value = -100.0f;  /*Time since last regen*/
float itemp_value = -100.0f; /*Input Temp*/
float otemp_value = -100.0f; /*Output Temp*/

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

void addToSerialLog(String text)
{
  Serial.println(text);
}

void addBtResponseToSerialLog(String text)
{
  Serial.print("< ");
  Serial.println(text);
}

void addBtCommandToSerialLog(String text)
{
  Serial.print("> ");
  Serial.println(text);
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
  if (connectByName == true)
  {
    addToLog("Connecting to: " + defaultObdIfName);
    result = btSerial.connect(defaultObdIfName);
    connectByName = false;
  }
  else
  {
    uint8_t address[6] = {0x86, 0xdc, 0xa6, 0xab, 0xf7, 0xf1};
    result = btSerial.connect(address);
    connectByName = true;
  }

  return result;
}

void printDeviceStatus()
{
  String status = "Connected: ";
  status += connected == true ? "Y" : "N";
  addToSerialLog(status);
}

void btSerialRead()
{
  char c = '\0';
  rxData[0] = '\0';
  unsigned long btSerialReadTimeout = millis() + (MAX_BT_RESPONSE_TIME * 1000);

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
      addToSerialLog("BtSerial read timeout error");
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

void btSerialReadAndAddToLog()
{
  btSerialRead();
  addBtResponseToSerialLog(String(rxData));
}

void btSerialSendCommand(String command, int delayTime)
{
  btSerial.flush();
  addBtCommandToSerialLog(command);
  btSerial.print(command);
#ifdef ENABLE_WIFI
  ws.textAll("W:");
  ws.textAll(command.c_str(), command.length());
  ws.textAll("\n");
#endif
  delay(delayTime);
}

void btSerialInit()
{
  addToLog("Initialize OBD...");
  btSerialSendCommand("ATZ\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATE0\r", 500);
  btSerialReadAndAddToLog();

  btSerialSendCommand("STI\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("VTI\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATD\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATE0\r", 500);
  btSerialReadAndAddToLog();

  btSerialSendCommand("ATSP0\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATE0\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATH1\r", 700);
  btSerialReadAndAddToLog();

  btSerialSendCommand("ATM0\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATS0\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATAT1\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATAL\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("ATST64\r", 500);
  btSerialReadAndAddToLog();

  btSerialSendCommand("0100\r", 5000); // SEARCHING...
  btSerialReadAndAddToLog();

  btSerialSendCommand("ATDPN\r", 500);
  btSerialReadAndAddToLog();

  btSerialSendCommand("ATSH7E0\r", 500);
  btSerialReadAndAddToLog();
  btSerialSendCommand("10031\r", 500);
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
  printf("#isReadCanError: %s\n", rxData);

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

bool btSerialGetSMMData()
{
  btSerialSendCommand("22114E1\r", 200);
  btSerialReadAndAddToLog();
  if (isReadCanError())
  {
    smm_value = -100.0f;
    return false;
  }
  smm_value = (((getByteFromData(11) * 256) + (getByteFromData(13))) / 100.0f);
  return true;
}

bool btSerialGetSMCData()
{
  btSerialSendCommand("22114F1\r", 200);
  btSerialReadAndAddToLog();
  if (isReadCanError())
  {
    smc_value = -100.0f;
    return false;
  }
  smc_value = (((getByteFromData(11) * 256) + (getByteFromData(13))) / 100.0f);
  return true;
}

bool btSerialGetSdlrData()
{
  btSerialSendCommand("221156\r", 200);
  btSerialReadAndAddToLog();
  if (isReadCanError())
  {
    dslr_value = -100.0f;
    return false;
  }
  dslr_value = (((getByteFromData(15) * 256) + (getByteFromData(17))) / 1000.0f);
  return true;
}

bool btSerialGetTslrData()
{
  btSerialSendCommand("22115E\r", 200);
  btSerialReadAndAddToLog();
  if (isReadCanError())
  {
    tslr_value = -100.0f;
    return false;
  }
  tslr_value = (((getByteFromData(15) * 256) + (getByteFromData(17))) / 60.0f);
  return true;
}

bool btSerialGetItempData()
{
  btSerialSendCommand("2211B2\r", 200);
  btSerialReadAndAddToLog();
  if (isReadCanError())
  {
    itemp_value = -100.0f;
    return false;
  }
  itemp_value = ((((getByteFromData(11) * 256) + (getByteFromData(13)))-2731) / 10.0f);
  return true;
}

bool btSerialGetOtempData()
{
  btSerialSendCommand("2210F9\r", 200);
  btSerialReadAndAddToLog();
  if (isReadCanError())
  {
    otemp_value = -100.0f;
    return false;
  }
  otemp_value = ((((getByteFromData(11) * 256) + (getByteFromData(13)))-2731) / 10.0f);
  return true;
}

void markReadingValue(int row, bool readResult)
{
  int xpos = 100, ypos = 10 * (row - 1);
  displayText(xpos, ypos, (readResult == true ? "V" : "X"));
}

void btSerialGetData()
{
  bool readResult = btSerialGetSMMData();
  markReadingValue(1, readResult);

  readResult = btSerialGetSMCData();
  markReadingValue(2, readResult);

  readResult = btSerialGetSdlrData();
  markReadingValue(3, readResult);

  readResult = btSerialGetTslrData();
  markReadingValue(4, readResult);

  readResult = btSerialGetItempData();
  markReadingValue(5, readResult);

  readResult = btSerialGetOtempData();
  markReadingValue(6, readResult);
}

void displayData()
{
  int xpos = 0, ypos = 0;

  clearDisplay();

  displayText(xpos, ypos, "smm:");
  ypos += 10;
  displayText(xpos, ypos, "smc:");
  ypos += 10;
  displayText(xpos, ypos, "dslr:");
  ypos += 10;
  displayText(xpos, ypos, "tslr:");
  ypos += 10;
  displayText(xpos, ypos, "itemp:");
  ypos += 10;
  displayText(xpos, ypos, "otemp:");

  xpos = 40;
  ypos = 0;
  displayText(xpos, ypos, (smm_value <= -100.0f ? "?" : String(smm_value, 2)));
  ypos += 10;
  displayText(xpos, ypos, (smc_value <= -100.0f ? "?" : String(smc_value, 2)));
  ypos += 10;
  displayText(xpos, ypos, (dslr_value <= -100.0f ? "?" : String(dslr_value, 2)));
  ypos += 10;
  displayText(xpos, ypos, (tslr_value <= -100.0f ? "?" : String(tslr_value, 2)));
  ypos += 10;
  displayText(xpos, ypos, (itemp_value <= -100.0f ? "?" : String(itemp_value, 2)));
  ypos += 10;
  displayText(xpos, ypos, (otemp_value <= -100.0f ? "?" : String(otemp_value, 2)));

  Serial.println("========================");
  Serial.println("smm_value: " + String(smm_value, 2) + "g, smc_value: " + String(smc_value, 2) + "g");
  Serial.println("dslr_value (distance): " + String(dslr_value, 2) + "km, tslr_value (time): " + String(tslr_value, 2) + "min");
  Serial.println("itemp_value (input t): " + String(itemp_value, 2) + "C, otemp_value (output t): " + String(otemp_value, 2) + "C");
  addToSerialLog("========================");
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
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void handleRoot(AsyncWebServerRequest *request)
{
  request->send_P(200, "text/html", FRM_PASS);
}

bool initBluetooth()
{
  if (!btStart())
  {
    Serial.println("Failed to initialize controller");
    return false;
  }

  if (esp_bluedroid_init() != ESP_OK)
  {
    Serial.println("Failed to initialize bluedroid");
    return false;
  }

  if (esp_bluedroid_enable() != ESP_OK)
  {
    Serial.println("Failed to enable bluedroid");
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
  request->send_P(200, "text/html", FRM_PASS);
}

void initWebserver()
{
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  addToLog("AP IP address: ");
  addToLog(myIP.toString());
  server.on("/", HTTP_GET, handleRoot);
  server.on("/remove", HTTP_GET, handleRemove);
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  server.begin();
}
#endif

void setup()
{
  Serial.begin(115200);
  bool result = false;

  initDisplay();
  addToLog("Display initialized");

  addToLog("Setup bluetooth...");
  result = btSerial.begin(bluetoothName, true);
  addResultToLog(result);

  addToLog("Set bluetooth PIN...");
  result = btSerial.setPin(bluetoothPin.c_str());
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
}

void loop()
{
#ifdef ENABLE_WIFI
  ws.cleanupClients();

  if (removeBondedDevices)
  {
    removeBondedDevices = false;
    initBluetooth();
    int count = esp_bt_gap_get_bond_device_num();
    if (!count)
    {
      Serial.println("No bonded device found.");
    }
    else
    {
      Serial.print("Bonded device count: ");
      Serial.println(count);
      if (PAIR_MAX_DEVICES < count)
      {
        count = PAIR_MAX_DEVICES;
        Serial.print("Reset bonded device count: ");
        Serial.println(count);
      }
      esp_err_t tError = esp_bt_gap_get_bond_device_list(&count, pairedDeviceBtAddr);
      if (ESP_OK == tError)
      {
        for (int i = 0; i < count; i++)
        {
          Serial.print("Found bonded device # ");
          Serial.print(i);
          Serial.print(" -> ");
          Serial.println(bda2str(pairedDeviceBtAddr[i], bda_str, 18));
          if (REMOVE_BONDED_DEVICES)
          {
            esp_err_t tError = esp_bt_gap_remove_bond_device(pairedDeviceBtAddr[i]);
            if (ESP_OK == tError)
            {
              Serial.print("Removed bonded device # ");
            }
            else
            {
              Serial.print("Failed to remove bonded device # ");
            }
            Serial.println(i);
          }
        }
      }
    }
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
      delay(500);
      return;
    }

    addToLog("Connected");
    btSerialInit();
    displayData();
  }

  if (!connected)
  {
    delay(500);
    return;
  }

  addToSerialLog("Receive data from OBD...");
  btSerialGetData();
  addToSerialLog("Display data");
  displayData();

  addToSerialLog("Show progress bar");
  drawProgressBar();
}