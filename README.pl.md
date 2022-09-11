<div id="top"></div>

[![Contributors][contributors-shield]][contributors-url]
[![Forks][forks-shield]][forks-url]
[![Stargazers][stars-shield]][stars-url]
[![Issues][issues-shield]][issues-url]
[![MIT License][license-shield]][license-url]
[![LinkedIn][linkedin-shield]][linkedin-url]

<br />
<div align="center">

<h3 align="center">WIFI_kit_32_dpf</h3>

  <p align="center">
    Wyświetlanie statusu DPF na urządzeniu WIFI_kit_32 lub Wemos Lolin32 OLED.
    <br />
    <br />
    <a href="https://github.com/blizniukp/WIFI_kit_32_dpf/issues">Zgłoś błąd</a>
    ·
    <a href="https://github.com/blizniukp/WIFI_kit_32_dpf/issues">Zaproponuj nową funkcjonalność</a>
  </p>
</div>

*Read this in other language: [English](README.md), [Polski](README.pl.md).*

<details>
  <summary>Spis treści</summary>
  <ol>
    <li><a href="#o-projekcie">O projekcie</a></li>
    <li><a href="#sprzęt">Sprzęt</a></li>
    <li><a href="#wgrywanie-firmware-do-urządzenia">Wgrywanie firmware do urządzenia</a></li>
    <li><a href="#konfiguracja-urządzenia">Konfiguracja urządzenia</a></li>
    <li><a href="#plan-rozwoju">Plan rozwoju</a></li>
    <li><a href="#licencja">Licencja</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## O projekcie

Urządznie łączy się z interfejsem OBD po Bluetooth i wyświetla informacje o stanie DPF.

![pic03](/docs/pic03.png)
![pic04](/docs/pic04.png)

Wyświetlane są następujące parametry:

* Zmierzona masa sadzy (smm)
* Wyliczona masa sadzy (smc)
* Dystans od ostatniej regeneracji (dslr)
* Czas od ostatniej regeneracji (tslr)
* Pozostałość popiołu olejowego (oar)
* Temperatura wejściowa (itemp)
* Temperatura wyjściowa (otemp)


Dane odświeżane są co 10 sekund. Po prawej stronie wyświetlacza zaznaczany jest prawidłowy `V` lub nieprawidłowy `X` odczyt danego parametru.
Na dole wyświetlany jest pasek postępu który odlicza czas do odpytania o kolejny parametr.

Każdy z odczytywanych parametrów można włączyć lub wyłączyć.

Urządzenie testowałem tylko z interfejsem iCar2 Vgate, ale podejrzewam, że może działać w oparciu o dowolny interfejs z ELM 327.

Bazowałem na projekcie https://github.com/yangosoft/dpf

<p align="right">(<a href="#top">powrót do góry</a>)</p>

## Sprzęt

Projekt miał być jak najmniej skomplikowany, dlatego szukałem płytki z modułem ESP32 z już wbudowanym wyświetlaczem.

Znalazłem dwa rozwiązania do swojego projektu:
* WIFI_kit_32 
* WeMos Lolin32 OLED

Program jest przygotowany pod obie wersje.

Wyboru płytki dokonujemy w pliku `platformio.ini` pozostawiając odkomentowaną jedną z opcji:

```
-DWIFI_KIT_32_BOARD
;-DWEMOS_BOARD
```

Opcjonalnie można dodać buzzer (z generatorem) który informuje o momencie rozpoczęcia oraz zakończenia wypalania DPF. Buzzer należy wpiąc pomiędzy pin GND oraz D12.

<p align="right">(<a href="#top">powrót do góry</a>)</p>

## Wgrywanie firmware do urządzenia

Firmware do urządzenia można wgrać za pomocą aplikacji [Esp Download Tool](https://www.espressif.com/en/support/download/other-tools)


Najnowsza wersja firmware jest dostępna w [Releases](https://github.com/blizniukp/WIFI_kit_32_dpf/releases)


W paczce z firmware powinniśmy mieć cztery pliki:
- bootloader_dio_40m.bin
- partitions.bin
- boot_app0.bin
- firmware.bin

Należy je wypakować do jakiegoś katalogu na dysku.

Po uruchomieniu aplikacji `ESP32 DOWNLOAD TOOL` należy wskazać te pliki oraz uzupełnić pola analogicznie jak na zrzucie ekranu ekranu poniżej.

![esp32_download_tool](/docs/esp32_download_tool.png)

```
bootloader_dio_40m.bin - 0x1000
partitions.bin - 0x8000
boot_app0.bin - 0xe000
firmware.bin - 0x10000
```

Naciśnij przycisk `START` aby rozpocząć wgrywanie firmware do urządzenia.

<p align="right">(<a href="#top">powrót do góry</a>)</p>


## Konfiguracja urządzenia

Urządzenie nie posiada jeszcze żadnej opcji konfiguracji.

W momencie uruchomienia urządzenia tworzony jest hotspot WiFi o następującej nazwie i haśle:

Nazwa sieci: `WIFI_kit_32_dpf`

Hasło to: `wifikit32`

Po połączeniu się i wejściu na adres 192.168.4.1 mamy możliwość podejrzenia komunikacji oraz usunięcia sparowanych urządzeń.

![Screenshot](docs/esp_website.jpg)


Przycisk 'Remove bonded devices' rozpoczyna procedurę usunięcia sparowanych urządzeń Bluetooth w module ESP32.
Jest to przydatne w momencie występienia błędu podczas łączenia. A problem pojawia się, gdy do interfejsu OBD połączymy się z telefonu, a następnie ponownie chcemy połączyć się za pomocą urządzenia.

<p align="right">(<a href="#top">powrót do góry</a>)</p>

## Plan rozwoju

- [x] Zwiększenie czcionki i wyświetlanie tylko jednego parametru na raz
- [x] Możliwość konfiguracji urządzenia (podanie nazwy interfejsu OBD oraz kodu PIN)
- [x] Zaktualizować zrzut ekranu z ESP32 DOWNLOAD TOOL (używam Linuxa, a ta aplikacja jest pod Windowsa)
- [x] Alarm (buzzer) w momencie rozpoczęcia i zakończenia regeneracji DPF
- [ ] Automatyczne usuwanie sparowanych urządzeń w przypadku błędu połączenia
- [x] Zapis konfiguracji do pamięci
- [x] Zmiana orientacji wyświetlacza

<p align="right">(<a href="#top">powrót do góry</a>)</p>

## Licencja

Projekt jest udostępniony na licencji MIT. 

Zerknij do pliku [LICENSE](LICENSE) aby poznać szczegóły.

<p align="right">(<a href="#top">powrót do góry</a>)</p>


<!-- MARKDOWN LINKS & IMAGES -->
<!-- https://www.markdownguide.org/basic-syntax/#reference-style-links -->
[contributors-shield]: https://img.shields.io/github/contributors/blizniukp/WIFI_kit_32_dpf.svg?style=for-the-badge
[contributors-url]: https://github.com/blizniukp/WIFI_kit_32_dpf/graphs/contributors
[forks-shield]: https://img.shields.io/github/forks/blizniukp/WIFI_kit_32_dpf.svg?style=for-the-badge
[forks-url]: https://github.com/blizniukp/WIFI_kit_32_dpf/network/members
[stars-shield]: https://img.shields.io/github/stars/blizniukp/WIFI_kit_32_dpf.svg?style=for-the-badge
[stars-url]: https://github.com/blizniukp/WIFI_kit_32_dpf/stargazers
[issues-shield]: https://img.shields.io/github/issues/blizniukp/WIFI_kit_32_dpf.svg?style=for-the-badge
[issues-url]: https://github.com/blizniukp/WIFI_kit_32_dpf/issues
[license-shield]: https://img.shields.io/github/license/blizniukp/WIFI_kit_32_dpf.svg?style=for-the-badge
[license-url]: https://github.com/blizniukp/WIFI_kit_32_dpf/blob/master/LICENSE
[linkedin-shield]: https://img.shields.io/badge/-LinkedIn-black.svg?style=for-the-badge&logo=linkedin&colorB=555
[linkedin-url]: https://linkedin.com/in/paweł-bliźniuk-433535183
[product-screenshot]: images/screenshot.png
