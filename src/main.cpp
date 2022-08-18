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
char canErrorMessage[] = "CAN ERROR";
const int READ_BUFFER_SIZE = 128;

bool connected = false;
int logLineNumber = 0;
char rxData[READ_BUFFER_SIZE];
uint8_t rxIndex = 0;
bool connectByName = true;

float smm_value = -100.0f; /*Soot mass measured*/
float smc_value = -100.0f; /*Soot mass calculated*/

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
  char *ptr = strstr(rxData, canErrorMessage);
  if (ptr == NULL)
    return false;
  if (rxData[0] == '\0')
    return false;
  return true;
}

int getByteFromData(int index)
{
  char buffer[3] = {0, 0, 0};
  buffer[0] = rxData[index];
  buffer[0 + 1] = rxData[index + 1];
  return (strtol(&buffer[0], NULL, 16));
}

void btSerialGetSMCData()
{
  btSerialSendCommand("22114E1\r", 200);
  btSerialReadAndAddToLog();
  smm_value = (((getByteFromData(11) * 256) + (getByteFromData(13))) / 100.0f);

  btSerialSendCommand("22114F1\r", 200);
  btSerialReadAndAddToLog();
  smc_value = (((getByteFromData(11) * 256) + (getByteFromData(13))) / 100.0f);
}

void btSerialGetData()
{
  btSerialGetSMCData();
}

void displayData()
{
  int xpos = 0, ypos = 0;

  clearDisplay();

  displayText(xpos, ypos, "smm:");
  ypos += 10;
  displayText(xpos, ypos, "smc:");

  xpos = 40;
  ypos = 0;
  displayText(xpos, ypos, (smm_value <= -100.0f ? "?" : String(smm_value, 2)));
  ypos += 10;
  displayText(xpos, ypos, (smc_value <= -100.0f ? "?" : String(smc_value, 2)));

  Serial.println("========================");
  Serial.print("smm_value: ");
  Serial.print(String(smm_value, 2));
  Serial.print(" g, smc_value: ");
  Serial.print(String(smm_value, 2));
  Serial.println(" g");
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

void initWebserver()
{
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  addToLog("AP IP address: ");
  addToLog(myIP.toString());
  server.on("/", HTTP_GET, handleRoot);
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

    btSerialInit();
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