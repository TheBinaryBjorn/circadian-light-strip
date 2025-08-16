# ESP32 Circadian Rhythm LED Strip
This project creates an automated, Wi-Fi-connected circadian lighting system using an ESP32 microcontroller and a WS2812B RGB LED strip. The lights automatically adjust their color temperature throughout the day to match natural light cycles, promoting a healthier sleep-wake rhythm.

### 🌟 Features
- Circadian Lighting: Automatically switches between a cool blue light (for daytime) and a warm orange light (for evening).
- Time Synchronization: Connects to your Wi-Fi network and uses the Network Time Protocol (NTP) to get the current time, ensuring accurate timing for sunrise and sunset.
- Customizable Timings: Easily change the "sunrise" and "sunset" hours to match your schedule and location.
- Startup Indicator: A pulsing light effect on startup to show that the device is running and connecting to Wi-Fi.
- Modular Code: Written with clear, well-commented functions and constants for easy customization and future expansion.

### 🔌 Hardware Requirements
- [ESP32 Development Board](https://he.aliexpress.com/item/1005007671225942.html?spm=a2g0o.order_list.order_list_main.17.18ae586aKyi2pk&gatewayAdapt=glo2isr)
- [WS2812B RGB LED Strip: (I used 120 LEDs in this code, but customizable)](https://he.aliexpress.com/item/1005004289391906.html?spm=a2g0o.order_list.order_list_main.5.18ae586aKyi2pk&gatewayAdapt=glo2isr)
- [5V Power Supply: (e.g., 5V 10A supply).](https://he.aliexpress.com/item/4001159381316.html?spm=a2g0o.order_list.order_list_main.35.18ae586aKyi2pk&gatewayAdapt=glo2isr) The current rating should be sufficient for your number of LEDs (roughly 60mA per LED at full brightness).
- [Electrolytic Capacitor: (1000µF, 6.3V or higher)](https://he.aliexpress.com/item/32812576995.html?spm=a2g0o.order_list.order_list_main.11.18ae586aKyi2pk&gatewayAdapt=glo2isr) to protect the LED strip from initial voltage spikes.
- [Resistor: (330 Ω)](https://he.aliexpress.com/item/32952657927.html?spm=a2g0o.order_list.order_list_main.23.18ae586aKyi2pk&gatewayAdapt=glo2isr) to protect the LED strip from voltage spikes.
- [Breadboard](https://he.aliexpress.com/item/1005004532352681.html?spm=a2g0o.order_list.order_list_main.28.18ae586aKyi2pk&gatewayAdapt=glo2isr)
- [Jumper Wires](https://he.aliexpress.com/item/1005004532352681.html?spm=a2g0o.order_list.order_list_main.29.18ae586aKyi2pk&gatewayAdapt=glo2isr) I recommend all types, male to male, male to female, female to male. I only ordered male to male and had to get creative...
- [DC Female Connector](https://he.aliexpress.com/item/1005006472747217.html?spm=a2g0o.productlist.main.1.581cau6fau6fbu&algo_pvid=31d23c7f-d3a5-401b-8b01-3019abd3a9c7&algo_exp_id=31d23c7f-d3a5-401b-8b01-3019abd3a9c7-0&pdp_ext_f=%7B%22order%22%3A%226223%22%2C%22eval%22%3A%221%22%7D&pdp_npi=6%40dis%21ILS%213.60%213.60%21%21%211.04%211.04%21%4021010d9017553594114463657e920e%2112000037321758211%21sea%21IL%212447827219%21X%211%210%21n_tag%3A-29919%3Bm03_new_user%3A-29895&curPageLogUid=pHIz1SmY3yet&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005006472747217%7C_p_origin_prod%3A), not sure if that's the right one because I also didn't order one and had to repurpose one from another led strip.

### 🗺️ Wiring Diagram
Connect your components as shown in the diagram below. A capacitor is essential to prevent damage to the LED strip from a sudden power surge.

<img width="784" height="750" alt="Circuit Diagram" src="https://github.com/user-attachments/assets/b78fec91-d8ff-47c7-aaf4-d1e0f1d9c371" />

### 💻 Software Setup
1. Install Arduino IDE: If you haven't already, download and install the Arduino IDE.

2. Add ESP32 Board Manager: Go to File > Preferences and add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json to the "Additional Board Manager URLs." Then, go to Tools > Board > Board Manager and search for "esp32" to install the library.

3. Install FastLED Library: Go to Sketch > Include Library > Manage Libraries and search for "FastLED" to install the library.

4. Create secrets.h File: In the same folder as your main .ino file, create a new file named secrets.h. This file keeps your Wi-Fi credentials private.

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
- Colors: Adjust the CRGB values for COLD and WARM to your liking.
```
const CRGB COLD = [Your Cold Color]
const CRGB WARM = [Your Warm Color]
```
- Sunrise/Sunset Times: Change the SUNRISE and SUNSET constants to set the exact hours for the color changes (e.g., const int SUNRISE = 7;).
```
const int SUNRISE = [Your Sunrise Time];
const int SUNSET = [Your Sunset Time];
```
