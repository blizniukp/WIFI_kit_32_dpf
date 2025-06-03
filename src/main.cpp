#include "main.hpp"

static const uint8_t max_log_lines = 60;
static const uint8_t last_line = 56;
static const uint8_t max_bt_response_time = 10; /*10 seconds*/
static const uint8_t max_bt_paired_devices = 20;
static const uint8_t remove_bonded_devices = 1;
static const uint8_t rx_buffer_size = 128;
static const uint32_t data_read_interval = 5000; /*5 seconds*/

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

void dataReadFun_Temperature(float value);
void dataReadFun_SootLoad(float value);

std::vector<measurement_t> measurements;
configuration_t config;

void initMeasurements(std::vector<measurement_t>& m) {
    m.push_back((measurement_t){1, "Soot mass measured", "22114E1\r", "g", -100.0f, &calcFun_AB, 100.0f, NULL, true, NULL});
    m.push_back((measurement_t){2, "Soot mass calculated", "22114F1\r", "g", -100.0f, &calcFun_AB, 100.0f, NULL, true, NULL});
    m.push_back((measurement_t){3, "Distance since last regen.", "221156\r", "km", -100.0f, &calcFun_ABCD, 1000.0f, NULL, true, NULL});
    m.push_back((measurement_t){4, "Time since last regen", "22115E\r", "min", -100.0f, &calcFun_ABCD, 60.0f, NULL, true, NULL});
    m.push_back((measurement_t){5, "Input temperature", "2211B2\r", "*C", -100.0f, &calcFun_Temperature, 10.0f, NULL, true, &dataReadFun_Temperature});
    m.push_back((measurement_t){6, "Output temperature", "2210F9\r", "*C", -100.0f, &calcFun_Temperature, 10.0f, NULL, true, NULL});
    m.push_back((measurement_t){7, "Oil Ash Residue", "22178C\r", "g", -100.0f, &calcFun_AB, 10.0f, NULL, true, NULL});
    m.push_back((measurement_t){8, "Soot load (%)", "22114E1\r", "%", -100.0f, &calcFun_SootLoad, 100.0f, &config.max_soot_mass, true, &dataReadFun_SootLoad});
#if DEBUG
    for (uint8_t msmIdx = 0; msmIdx < m.size(); msmIdx++) {
        Serial.printf("measurement[%d]: %s\n", m[msmIdx].id, m[msmIdx].caption);
    }
#endif
}

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
    } else {
        addToLog(" ERROR!");
    }
}

bool connect() {
    bool connected = false;
    /*
    Connecting by name does not always work well.
    I had a problem with connecting to "OBDII" interface.
    The get_name_from_eir function (in BluetoothSerial.cpp file) returns an incorrect device name length (peer_bdname_len field).
    So, this is a workaround.*/
    if (connectByName) {
        addToLog("Connecting to: " + config.bt_if_name);
        connected = btSerial.connect(config.bt_if_name);
        connectByName = false;
    } else {
        BTScanResults* btDeviceList = btSerial.getScanResults();
        if (btDeviceList->getCount() > 0) {
            BTAddress addr;
            Serial.printf("Found %d devices\n", btDeviceList->getCount());
            for (uint8_t deviceIdx = 0; deviceIdx < btDeviceList->getCount(); deviceIdx++) {
                BTAdvertisedDevice* device = btDeviceList->getDevice(deviceIdx);
                Serial.printf(" -- Address: %s, Name: %s\n", device->getAddress().toString().c_str(), String(device->getName().c_str()));
                if (device->getName().compare(config.bt_if_name.c_str()) == 0) {
                    addToLog("Connecting to: " + String(device->getAddress().toString().c_str()));
                    connected = btSerial.connect(device->getAddress());
                }
            }
        }
        connectByName = true;
    }

    return connected;
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
    std::vector<String> initCommands = {
        "ATZ\r", "ATE0\r", "STI\r", "VTI\r", "ATD\r", "ATE0\r",
        "ATSP0\r", "ATE0\r", "ATH1\r", "ATM0\r", "ATS0\r", "ATAT1\r",
        "ATAL\r", "ATST64\r"};
    std::vector<String> initCommands2 = {
        "ATDPN\r", "ATSH7E0\r", "10031\r"};

    for (uint8_t initCmdIdx = 0; initCmdIdx < initCommands.size(); initCmdIdx++) {
        btSerialSendCommand(initCommands[initCmdIdx]);
        btSerialReadAndAddToLog(buffer);
    }

    btSerialSendCommand("0100\r");  // SEARCHING...
    btSerialReadAndAddToLog(buffer, 5000);

    for (uint8_t initCmdIdx = 0; initCmdIdx < initCommands2.size(); initCmdIdx++) {
        btSerialSendCommand(initCommands2[initCmdIdx]);
        btSerialReadAndAddToLog(buffer);
    }
}

void btSerialCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t* param) {
    switch (event) {
        case ESP_SPP_CLOSE_EVT: {
            addToLog("Disconnected!");
            connected = false;
        } break;
        default:

            break;
    }
}

void dataReadFun_Temperature(float value) {
    buzzer_set_temperature(value);
}

void dataReadFun_SootLoad(float value) {
    buzzer_set_soot_load(value);
}

void displayData(bool correctData, measurement_t* m) {
    uint8_t xpos = 0, ypos = 0;

    clearDisplay();
    display->setFont(ArialMT_Plain_16);
    displayText(xpos, ypos, m->caption);

    display->setFont(ArialMT_Plain_24);
    if (correctData) {
        displayText(xpos, ypos + 25, String(m->value, 2) + " " + String(m->unit));
    } else {
        displayText(xpos, ypos + 25, "???");
    }

    display->setFont(ArialMT_Plain_10);
    displayText(115, last_line - 5, (correctData == true ? "V" : "X"));
    Serial.printf("caption: %s, isCorrectData: %s, value: %.2f\n", m->caption, (correctData == true ? "Y" : "N"), m->value);
    Serial.println("========================");
}

void drawProgressBar(uint32_t delayProgress) {
    displayText(0, last_line, "[");
    displayText(100, last_line, "]");

    for (uint8_t columnIdx = 1; columnIdx < 100; columnIdx++) {
        displayText(columnIdx, last_line - 3, ".");
        delay(delayProgress);
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
    std::ostringstream configPage;
    config_get_page(&configPage, &config, &measurements);
    request->send_P(200, "text/html", configPage.str().c_str());
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
    std::ostringstream configPage;
    config_get_page(&configPage, &config, &measurements);
    request->send_P(200, "text/html", configPage.str().c_str());
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
    if (request->hasParam(CFG_PARAMS_THRESHOLD)) {
        config.temperature_threshold = request->getParam(CFG_PARAMS_THRESHOLD)->value().toFloat();
    }
    if (request->hasParam(CFG_PARAMS_MAX_SHOOT)) {
        config.max_soot_mass = request->getParam(CFG_PARAMS_MAX_SHOOT)->value().toFloat();
    }

    for (uint8_t msmIdx = 0; msmIdx < measurements.size(); msmIdx++) {
        String pname = "m_" + String(measurements[msmIdx].id);
        if (request->hasParam(pname)) {
            measurements[msmIdx].enabled = true;
        } else {
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
    IPAddress apIP = WiFi.softAPIP();
    addToLog("AP IP address: ");
    addToLog(apIP.toString());
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
    } else {
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
                } else {
                    Serial.printf("Failed to remove bonded device # %d\n", deviceIdx);
                }
            }
        }
    }
}

void setup() {
    Serial.begin(115200);
    bool result = false;

    initMeasurements(measurements);

    initDisplay();
    addToLog("Display initialized");

    addToLog("Init configuration...");
    config_init();

    addToLog("Load configuration...");
    config_load(&config, measurements);

    if (config.display_flip_screen) {
        display->flipScreenVertically();
        display->clear();
    }

    addToLog("Enable buzzer");
    buzzer_init(BUZZER_PIN);
    buzzer_set_temperature_threshold(config.temperature_threshold);

#ifdef ENABLE_WIFI
    initWebserver();
#endif

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

#ifdef RANDOM_DATA
    connected = true;
#endif
}

void loop() {
    static uint8_t measurementIdx = 0;
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

#ifdef ENABLE_WIFI
        IPAddress apIP = WiFi.softAPIP();
        addToLog("AP IP address: ");
        addToLog(apIP.toString());
#endif
        connected = connect();
        addResultToLog(connected);

        if (!connected) {
            delay(50);
            return;
        }

        addToLog("Connected");
        btSerialInit(rxData);
        measurementIdx = 0;
        clearDisplay();
    }

    if (!connected) {
        delay(50);
        return;
    }

    uint32_t measureTime = millis();
    measurement_t* m = &measurements[measurementIdx];
#ifdef RANDOM_DATA
    rxLen = 0;
#else
    btSerialSendCommand(m->command);
    rxLen = btSerialReadAndAddToLog(rxData);
#endif
    bool correctData = m->calcFunPtr(rxData, rxLen, &m->value, m->divider, m->calcFunParam);
    if (correctData && m->dataReadFunPtr) {
        m->dataReadFunPtr(m->value);
    }

    Serial.println("Display data");
    displayData(correctData, m);

    uint32_t delayProgress = 0;
    if ((millis() - measureTime) < data_read_interval) {
        delayProgress = (data_read_interval - (millis() - measureTime)) / 100;
    }
    drawProgressBar(delayProgress);

    do {
        measurementIdx++;
        if (measurementIdx >= measurements.size() ||
            measurements[measurementIdx].calcFunPtr == NULL) {
            measurementIdx = 0;
        }
    } while (!measurements[measurementIdx].enabled);
}