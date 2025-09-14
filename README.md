# ESP RGB IoT Light System
<img width="731" height="392" alt="IoT-Strip-Installed-Orange" src="https://github.com/user-attachments/assets/dff16145-7337-4c71-99ac-8131e7bf961a" />

<img width="784" height="750" alt="IoT-Strip-Controller-Box" src="https://github.com/user-attachments/assets/28f3efa7-c296-4709-ab3e-eebc42a90c3c" />

### ✨ Features
- Wireless Control: Control your lighting's RGB color and brightness from any web browser on your local network.
- Circadian Mode: An automatic mode that adjusts the light's color temperature throughout the day, providing less blue light at night for better sleep health 🌙.
- Multi-Controller Sync: Multiple ESP32 units can communicate and synchronize their state using MQTT, NTP, and mDNS networking protocols.

### 🔌 Hardware Requirements
- [ESP32 Development Board](https://he.aliexpress.com/item/1005007671225942.html?spm=a2g0o.order_list.order_list_main.17.18ae586aKyi2pk&gatewayAdapt=glo2isr)
- [WS2812B RGB LED Strip: (I used 120 LEDs in this code, but customizable)](https://he.aliexpress.com/item/1005004289391906.html?spm=a2g0o.order_list.order_list_main.5.18ae586aKyi2pk&gatewayAdapt=glo2isr)
- [5V Power Supply: (e.g., 5V 10A supply).](https://he.aliexpress.com/item/4001159381316.html?spm=a2g0o.order_list.order_list_main.35.18ae586aKyi2pk&gatewayAdapt=glo2isr) The current rating should be sufficient for your number of LEDs (roughly 60mA per LED at full brightness).
- [Electrolytic Capacitor: (1000µF, 6.3V or higher)](https://he.aliexpress.com/item/32812576995.html?spm=a2g0o.order_list.order_list_main.11.18ae586aKyi2pk&gatewayAdapt=glo2isr) to protect the LED strip from initial voltage spikes.
- [Resistor: (330 Ω)](https://he.aliexpress.com/item/32952657927.html?spm=a2g0o.order_list.order_list_main.23.18ae586aKyi2pk&gatewayAdapt=glo2isr) to protect the LED strip from voltage spikes.
- [Breadboard](https://he.aliexpress.com/item/1005004532352681.html?spm=a2g0o.order_list.order_list_main.28.18ae586aKyi2pk&gatewayAdapt=glo2isr)
- [Jumper Wires](https://he.aliexpress.com/item/1005004532352681.html?spm=a2g0o.order_list.order_list_main.29.18ae586aKyi2pk&gatewayAdapt=glo2isr) I recommend all types, male to male, male to female, female to male. I only ordered male to male and had to get creative...
- [DC Female Connector](https://he.aliexpress.com/item/1005006472747217.html?spm=a2g0o.productlist.main.1.581cau6fau6fbu&algo_pvid=31d23c7f-d3a5-401b-8b01-3019abd3a9c7&algo_exp_id=31d23c7f-d3a5-401b-8b01-3019abd3a9c7-0&pdp_ext_f=%7B%22order%22%3A%226223%22%2C%22eval%22%3A%221%22%7D&pdp_npi=6%40dis%21ILS%213.60%213.60%21%21%211.04%211.04%21%4021010d9017553594114463657e920e%2112000037321758211%21sea%21IL%212447827219%21X%211%210%21n_tag%3A-29919%3Bm03_new_user%3A-29895&curPageLogUid=pHIz1SmY3yet&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005006472747217%7C_p_origin_prod%3A), not sure if that's the right one because I also didn't order one and had to repurpose one from another led strip.

### ⚙️ How It Works (High-Level)
The system is built on a publish-subscribe model to ensure seamless communication between devices:
- MQTT: Based on the common IoT publish-subscribe protocol, this allows multiple ESP32 controllers to communicate and share data efficiently.
- Server: One ESP32 acts as the main server, functioning as an MQTT broker and an HTTP server to host the web interface.
- Clients: Client ESP32s subscribe to MQTT topics to receive commands (color hex codes, time data) and update their LED strips accordingly.
- Web Interface: The user interface is hosted directly on the microcontroller using LittleFS. This allows for rapid development and testing, you only need to reflash the code when a new core feature is added, not for every minor change to the HTML or JavaScript.

### Architecture Diagram
<img alt="IoT-RGB-Architecture" src="https://github.com/user-attachments/assets/94e82b6e-9f95-4d8a-b9bb-08408a2cb040" />

### 🗺️ Wiring Diagram
Connect your components as shown in the diagram below. A capacitor is essential to prevent damage to the LED strip from a sudden power surge.

<img alt="Circuit Diagram" src="https://github.com/user-attachments/assets/b78fec91-d8ff-47c7-aaf4-d1e0f1d9c371" />

### 💻 Software Setup
1. Install Arduino IDE: If you haven't already, download and install the Arduino IDE.

2. Add ESP32 Board Manager: Go to File > Preferences and add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json to the "Additional Board Manager URLs." Then, go to Tools > Board > Board Manager and search for "esp32" to install the library.

3. Install FastLED Library: Go to Sketch > Include Library > Manage Libraries and search for "FastLED" to install the library.

4. Create secrets.h File: In the same folder as your main .ino file, create a new file named secrets.h. This file keeps your Wi-Fi credentials private.

6. Use LittleFS to upload index.html to the server esp32.

#### secrets.h
```
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
```
Upload the Code: Copy the main project code into a new Arduino sketch, update the secrets.h file with your Wi-Fi credentials, and upload it to your ESP32.

### ⚙️ Customization
You can easily modify the behavior of the lights by changing the constants at the top of the code.

- Number of LEDs: Change the #define NUM_LEDS 120 line to match your strip.
```
// Define Number of LEDs
#define NUM_LEDS 120
```
- Colors: Adjust the Hex values to your liking (circadian mode).
```
const String circadianColors[] = {
  "000000", // 0:00 - Night (off)
  "000000", // 1:00 - Night (off)
  "000000", // 2:00 - Night (off)
  "000000", // 3:00 - Night (off)
  "000000", // 4:00 - Night (off)
  "0F0300", // 5:00 - 
  "FF2800", // 6:00 - Sunrise (Warm Orange) (G:60)
  "FF5000", // 7:00 - Orange (G:80)
  "FF6400", // 8:00 - Light Orange (G:100)
  "FF7800", // 9:00 - Warm Yellow (G:100)
  "FFC800", // 10:00 - Yellow (G:180)
  "00FFFF", // 11:00 - Cyan
  "00FFFF", // 12:00 - Cyan
  "00FFFF", // 13:00 - Cyan
  "00FFFF", // 14:00 - Cyan
  "FFC800", // 15:00 - Yellow (G:180)
  "FF7800", // 16:00 - Warm Yellow (G:100)
  "FF6400", // 17:00 - Light Orange (G:100)
  "FF5000", // 18:00 - Orange (G:80)
  "FF2800", // 19:00 - Sunset (2700K) (G:60)
  "BF1E00", // 20:00 - (G:40)
  "7F1400", // 21:00 - (G:40)
  "3F0A00", // 22:00 - (G:40)
  "000000"  // 23:00 - Night (off)
};
```

