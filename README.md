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
    Display DPF status on WIFI_kit_32 or Wemos Lolin32 OLED device.
    <br />
    <br />
    <a href="https://github.com/blizniukp/WIFI_kit_32_dpf/issues">Report Bug</a>
    ·
    <a href="https://github.com/blizniukp/WIFI_kit_32_dpf/issues">Request Feature</a>
  </p>
</div>

*Read this in other language: [English](README.md), [Polski](README.pl.md).*

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li><a href="#about-the-project">About The Project</a></li>
    <li><a href="#hardware">Hardware</a></li>
    <li><a href="#uploading-firmware-to-the-device">Uploading firmware to the device</a></li>
    <li><a href="#device-configuration">Device configuration</a></li>
    <li><a href="#roadmap">Roadmap</a></li>
    <li><a href="#license">License</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About The Project

The device connects to the OBD interface via Bluetooth and displays DPF status information.

![pic03](/docs/pic03.png)
![pic04](/docs/pic04.png)

The following parameters are displayed:

* Soot mass measured (smm)
* Soot mass calculated (smc)
* Distance since last regeneration (dslr)
* Time since last regeneration (tslr)
* Oil Ash Residue (oar)
* Input temperature (itemp)
* Output temperature (otemp)


The data is refreshed every 10 seconds. On the right side of the display, a correct `V` or incorrect `X` reading of a given parameter is marked.
A progress bar is displayed at the bottom, counting down the time until the next parameter is checked.

Each of the readable parameters can be turned on or off.

I tested the device only with the iCar2 Vgate interface, but I suspect it could work with any interface with the ELM 327.

I based the project https://github.com/yangosoft/dpf

<p align="right">(<a href="#top">back to top</a>)</p>

## Hardware

The project was to be as uncomplicated as possible, so I was looking for a board with an ESP32 module with the display already built in.

I found two solutions for my project:
* WIFI_kit_32 
* WeMos Lolin32 OLED

The program is prepared for both versions.

The board selection is made in the `platformio.ini` file by leaving one of the options uncommented:

```
-DWIFI_KIT_32_BOARD
;-DWEMOS_BOARD
```

<p align="right">(<a href="#top">back to top</a>)</p>

## Uploading firmware to the device

The firmware can be uploaded using the application [Esp Download Tool](https://www.espressif.com/en/support/download/other-tools)


The latest firmware version is available in [Releases](https://github.com/blizniukp/WIFI_kit_32_dpf/releases)


We should have four files in the firmware package:
- bootloader_dio_40m.bin
- partitions.bin
- boot_app0.bin
- firmware.bin

You should extract them to some directory on your disk.

After launching the `ESP32 DOWNLOAD TOOL` application, you need to point to these files and complete the fields analogously to the screenshot below.

![esp32_download_tool](/docs/esp32_download_tool.png)

```
bootloader_dio_40m.bin - 0x1000
partitions.bin - 0x8000
boot_app0.bin - 0xe000
firmware.bin - 0x10000
```

Press the `Start` button to start uploading the firmware to the device.

<p align="right">(<a href="#top">back to top</a>)</p>


## Device configuration

The device has no configuration option yet.

When the device starts up, a WiFi hotspot is created with the following name and password:

SSID: `WIFI_kit_32_dpf`

Password: `wifikit32`

After connecting and accessing the 192.168.4.1 address, we have the ability to preview communication and remove paired devices.

![Screenshot](docs/esp_website.jpg)


The 'Remove bonded devices' button starts the procedure for removing paired Bluetooth devices on the ESP32 module.
This is useful when a connection error occurs. And the problem occurs when you connect to the OBD interface from your phone, and then want to connect again using the device.

<p align="right">(<a href="#top">back to top</a>)</p>

## Roadmap

- [x] Increase the font and display only one parameter at a time
- [x] Ability to configure the device (specifying the name of the OBD interface and PIN code)
- [x] Update the screenshot from ESP32 DOWNLOAD TOOL (I'm using Linux, and this app is for Windows)
- [ ] Alarm (buzzer) when DPF regeneration begins and ends
- [ ] Automatic removal of paired devices in case of connection error
- [x] Save configurations to memory
- [x] Changing the display orientation

<p align="right">(<a href="#top">back to top</a>)</p>

## License

Distributed under the MIT License. 

See [LICENSE](LICENSE) for more information.

<p align="right">(<a href="#top">back to top</a>)</p>


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
