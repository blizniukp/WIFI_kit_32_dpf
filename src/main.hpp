#include <Arduino.h>
#include <Wire.h>
#include <BluetoothSerial.h>
#include "oled/SSD1306Wire.h"
#ifdef ENABLE_WIFI
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiServer.h>
#include <ESPAsyncWebServer.h>
#endif

#define SDA_OLED 5
#define SCL_OLED 4
#define RST_OLED 16

#define MAX_LOG_LINES (60)
#define LAST_LINE (55)
#define MAX_BT_RESPONSE_TIME (10) /*10 seconds*/

#ifdef ENABLE_WIFI
const char *ssid = "WIFI_kit_32_dpf";
const char *password = "wifikit32";

static const char FRM_PASS[] PROGMEM = R"rawliteral(<html>
<meta name='viewport' content='width=device-width, initial-scale=1'>
<body style='background-color:#161317; color: white'>            
    <p>Bluetooth serial dump:</p>
    <p>
        <textarea name="serial" id="serial" rows="10" cols="40"></textarea>
    </p>
    
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
</html>
)rawliteral";
#endif