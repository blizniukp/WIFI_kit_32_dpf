#include "main.hpp"

BluetoothSerial btSerial;
SSD1306Wire *display;

String bluetoothName = "WIFI_kit_32_dpf";
String bluetoothPin = "1234";
String defaultObdIfName = "V-LINK";
char canErrorMessage[] = "CAN ERROR";
const int READ_BUFFER_SIZE = 128;

bool connected = false;
int logLineNumber = 0;
char rxData[READ_BUFFER_SIZE];
uint8_t rxIndex = 0;

long smc_value = -1;
long km_value = -1;
long inttemp_value = -1;
float battery_value = -1.0f;

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

void addToLog(String text)
{
  if (logLineNumber >= MAX_LOG_LINES)
  {
    logLineNumber = 0;
    display->clear();
  }

  displayText(0, logLineNumber, text);
  Serial.println(text);

  logLineNumber += 10;
}

void addToSerialLog(String text)
{
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
  addToLog("Connecting to: " + defaultObdIfName);
  bool result = btSerial.connect(defaultObdIfName);

  return result;
}

void printDeviceStatus()
{
  String status = "Connected: ";
  status += connected == true ? "Y" : "N";
}

void btSerialRead()
{
  char c;
  rxData[0] = '\0';
  if (btSerial.available() <= 0)
    return;

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
    else
      break;
  } while (c != '>');
  rxData[rxIndex++] = '\0';
  rxIndex = 0;
}

void btSerialReadAndAddToLog()
{
  btSerialRead();
  addToLog(String(rxData));
}

void btSerialSendCommand(String command, int delayTime)
{
  btSerial.flush();
  addToSerialLog(command);
  btSerial.print(command);
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

  btSerialSendCommand("0100\r", 500);
  btSerialReadAndAddToLog();
}

void btSerialCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  switch (event)
  {
  case ESP_SPP_INIT_EVT:
  {
    addToLog("ESP_SPP_INIT_EVT!");
  }
  break;
  case ESP_SPP_UNINIT_EVT:
  {
    addToLog("ESP_SPP_UNINIT_EVT!");
  }
  break;
  case ESP_SPP_DISCOVERY_COMP_EVT:
  {
    addToLog("ESP_SPP_DISCOVERY_COMP_EVT!");
  }
  break;
  case ESP_SPP_OPEN_EVT:
  {
    addToLog("ESP_SPP_OPEN_EVT!");
  }
  break;
  case ESP_SPP_START_EVT:
  {
    addToLog("ESP_SPP_START_EVT!");
  }
  break;
  case ESP_SPP_CL_INIT_EVT:
  {
    addToLog("ESP_SPP_CL_INIT_EVT!");
  }
  break;
  case ESP_SPP_DATA_IND_EVT:
  {
    addToLog("ESP_SPP_DATA_IND_EVT!");
  }
  break;
  case ESP_SPP_CONG_EVT:
  {
    addToLog("ESP_SPP_CONG_EVT!");
  }
  break;
  case ESP_SPP_WRITE_EVT:
  {
    addToLog("ESP_SPP_WRITE_EVT!");
  }
  break;
  case ESP_SPP_SRV_STOP_EVT:
  {
    addToLog("ESP_SPP_SRV_STOP_EVT!");
  }
  break;

  case ESP_SPP_SRV_OPEN_EVT:
  {
    addToLog("Connected!");
  }
  break;
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
  return true;
}

void btSerialGetIntTemp()
{
  btSerialSendCommand("010F\r", 5);
  btSerialRead();
  if (isReadCanError())
  {
    inttemp_value = -1;
    return;
  }
  inttemp_value = strtol(&rxData[6], 0, 16) - 40;
}

void btSerialGetBattery()
{
  btSerialSendCommand("0142\r", 5);
  btSerialRead();
  if (isReadCanError())
  {
    battery_value = -1;
    return;
  }
  battery_value = ((strtol(&rxData[6], 0, 16) * 256) + strtol(&rxData[9], 0, 16));
  battery_value = battery_value / 1000.0f;
}

void btSerialGetData()
{
  btSerialGetBattery();
  btSerialGetIntTemp();
}

void displayData()
{
  int xpos = 0, ypos = 0;

  display->clear();

  displayText(xpos, ypos, "smc:");
  ypos += 10;
  displayText(xpos, ypos, "km:");
  ypos += 10;
  displayText(xpos, ypos, "inttemp:");
  ypos += 10;
  displayText(xpos, ypos, "battery:");

  xpos = 40;
  ypos = 0;
  displayText(xpos, ypos, (smc_value == -1 ? "?" : String(smc_value)));
  ypos += 10;
  displayText(xpos, ypos, (km_value == -1 ? "?" : String(km_value)));
  ypos += 10;
  displayText(xpos, ypos, (inttemp_value == -1 ? "?" : String(inttemp_value)));
  ypos += 10;
  displayText(xpos, ypos, (battery_value <= -1.0f ? "?" : String(battery_value)));
}

void drawProgressBar()
{
  displayText(0, LAST_LINE, "[");
  displayText(100, LAST_LINE, "]");

  for (int i = 1; i < 100; i++)
  {
    displayText(i, LAST_LINE - 3, ".");
    delay(100);
  }
}

void setup()
{
  Serial.begin(115200);
  bool result = false;

  initDisplay();
  addToLog("Display initialized");

  addToLog("Setup bluetooth...");
  result = btSerial.begin(bluetoothName, true);
  addResultToLog(result);

  addToLog("Set btSerial callback...");
  result = btSerial.register_callback(btSerialCallback);
  addResultToLog(result == ESP_OK ? true : false);

  addToLog("Is bluetooth ready ...");
  result = btSerial.isReady(true, 0);
  addResultToLog(result);

  addToLog("Set bluetooth PIN...");
  result = btSerial.setPin(bluetoothPin.c_str());
  addResultToLog(result);
}

void loop()
{
  printDeviceStatus();

  if (!connected)
  {
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