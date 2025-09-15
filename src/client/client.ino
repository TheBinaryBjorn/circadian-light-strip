#include <FastLED.h>
#include "secrets.h"
#include <WiFi.h>
#include <vector>
#include <PicoMQTT.h>
PicoMQTT::Client mqttClient("led-control.local");

// ------------------------------- RGB Strip ---------------------------------------
// RGB Strip Constants
// Define Number of LEDs
#define NUM_LEDS 120

// Define LED Array.
CRGB leds[NUM_LEDS];

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


// WiFi constants:
const char* SSID = WIFI_SSID;
const char* PASSWORD = WIFI_PASSWORD;

// Communications Constants
const int ESP32_STANDARD_BAUD_RATE = 115200;


// Flags to ensure color is switched only once at needed time.
int global_brightness = 127;

// Define the data pin I'm using (D23)
#define DATA_PIN 23

/*
  Receives a color in hex value, converts it to rgb and switches
  the led lights.
*/
void handleColor(const char *payload) {
    long hex_value = strtol(payload, NULL, 16);
    byte r = (hex_value >> 16) & 0xFF;
    byte g = (hex_value >> 8) & 0xFF;
    byte b = hex_value & 0xFF;

    CRGB new_color = CRGB(r, g, b);
    CRGB old_color = leds[0];
    if(new_color != old_color)
      color_all_leds(new_color);
}

/*
  Receives an hour (0 to 23) and calls for handle color with
  the corresponding circadian color.
*/
void handleHour(const char* hour) {
  long current_hour = strtol(hour, NULL, 10);
  if(current_hour > 23 || current_hour < 0) {
    Serial.println("Error: Hour must be 0-23");
    return;
  }
  handleColor(circadianColors[current_hour].c_str());
}

void handleBrightness(const char* new_brightness) {
  long long_new_brightness = strtol(new_brightness, NULL, 10);
  if(long_new_brightness < 0 || long_new_brightness > 255) {
    Serial.println("Error: brightness should be 0-255");
    return;
  }
  Serial.println(long_new_brightness);
  set_brightness(static_cast<int>(long_new_brightness));
}

// ---------------------------------------------------- SETUP --------------------------------------------------------

void setup() {
  // Initialize Serial Communication first
  Serial.begin(ESP32_STANDARD_BAUD_RATE);
  
  // Set up the FastLED library
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  set_brightness(global_brightness);
  
  // Set all LEDs to green on startup
  color_all_leds(CRGB::Green);

  // Connect to Wi-Fi
  WiFi.begin(SSID,PASSWORD);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nWi-Fi Connected.");

  /*
    Initialize MQTT Client
  */
  mqttClient.subscribe("color", [](const char * payload) {handleColor(payload);});
  mqttClient.subscribe("hour", [](const char* payload) {handleHour(payload);});
  mqttClient.subscribe("brightness", [](const char* payload) {handleBrightness(payload);});
  mqttClient.begin();
}
// --------------------------------------------------- LOOP -------------------------------------------------------------

void loop() {
  mqttClient.loop();
}

// ----------------------------------------------- Color Functions ----------------------------------------------------------

// Colors all the leds in the given range in the given color. Non Inclusive (start to end - 1)
void colorLedsInRange(CRGB color, int start, int end) {
  // check if ranges are valid.
  if(end > NUM_LEDS || end < 0) {
    Serial.println("Error: end index out of bounds.");
    return;
  }
  if(start<0 || start >= NUM_LEDS) {
    Serial.println("Error: start index out of bounds.");
    return;
  }
  // Set the colors in the given range.
  for(int i = start; i < end; i++)
    leds[i] = color;
  led_show();
}

/* 
  Sets all the LEDs in the strip to a given color - in my setup, 0 to 119.
  Utilizes NUM_LEDS const.
*/
void color_all_leds(CRGB color) {
  colorLedsInRange(color,0,NUM_LEDS);
}

// Wrapper function for FastLED.show(), shows the changes in the led.
void led_show() {
  FastLED.show();
}

// Changes the strip's brightness to the given brightness, including the global brightness variable.
void set_brightness(int brightness) {
  global_brightness = brightness;
  FastLED.setBrightness(brightness);
  led_show();
}