# WIFI_kit_32_dpf

*Read this in other language: [English](README.md), [Polski](README.pl.md).*

Wyświetla status DPF na urządzeniu WIFI_kit_32 lub Wemos Lolin32 OLED.

Urządzenia były testowane w Audi A4 B8 2.0 z silnikiem CAGA i interfejsem iCar2 Vgate.

Założeniem projektu było użycie gotowego modulu ESP32 z wyswietlaczem OLED.

Znalazłem dwa rozwiązania do swojego projektu:

* WIFI_kit_32 
* WeMos Lolin32 OLED

Program jest przygotowany pod obie płytki.

Bazowałem na projekcie https://github.com/yangosoft/dpf

Po uruchomieniu, urządzenie tworzy AccessPoint WiFi. Po wejściu na adres 192.168.4.1 mamy możliwość podejrzenia komunikacji oraz usunięcia sparowanych urządzeń.

![Screenshot](docs/esp_website.png)


Przycisk 'Remove bonded devices' rozpoczyna procedurę usunięcia sparowanych urządzeń Bluetooth w module ESP32.
Jest to przydatne w momencie występienia błędu podczas łączenia. A problem pojawia się, gdy do interfejsu OBD połączymy się z telefonu, a następnie ponownie chcemy połączyć się za pomocą urządzenia.
