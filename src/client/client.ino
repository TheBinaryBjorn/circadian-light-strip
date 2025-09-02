#include <FastLED.h>
#include "secrets.h"
#include <WiFi.h>
#include <vector>
#include <PicoMQTT.h>
PicoMQTT::Client mqttClient("10.0.0.10");

// ------------------------------- Function Declerations ---------------------------
// Colors all the leds in the given range in the given color. Non Inclusive (start to end - 1)
void color_leds_range(CRGB color, int start, int end);
/* 
  Sets all the LEDs in the strip to a given color - in my setup, 0 to 119.
  Utilizes NUM_LEDS const.
*/
void color_all_leds(CRGB color);
// Wrapper function for FastLED.show(), shows the changes in the led.
void led_show();
// Switches the led color to the given color with a pulse animation.
void switch_color(CRGB new_color);
// A pulse animation for the led strip.
void pulse();
// Changes the strip's brightness to the given brightness
void set_brightness(int brightness);
// Changes the strip's brightness to the given brightness, without changing the global brightness variable.
void set_temp_brightness(int brightness);
// A wave pattern for the led strip.
void wave_pattern();
// A police pattern for the led strip, alternates Red and Blue.
void police_pattern();
// ------------------------------- RGB Strip ---------------------------------------
// RGB Strip Constants
// Define Number of LEDs
#define NUM_LEDS 120

// Define LED Aarray.
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
const CRGB COLD = CRGB::Cyan;
const CRGB WARM = CRGB(255,40,0);
// For future use, gradual warming.
const CRGB WARM_LIGHT = CRGB(255,60,0);
const CRGB WARM_MEDIUM = CRGB(255,50,0);
const CRGB WARM_HARD = CRGB(255,40,0);
const CRGB BLACK = CRGB::Black;


int pattern_counter = 0;

// WiFi constants:
const char* SSID = WIFI_SSID;
const char* PASSWORD = WIFI_PASSWORD;

// Communications Constats
const int ESP32_STANDARD_BAUD_RATE = 115200;

// Time Constants (Milisec)
const long ONE_MINUTE = 60000;
const long FIVE_MINUTES = 5 * ONE_MINUTE;
const long TEN_MINUTES = 2 * FIVE_MINUTES;
const long THIRTY_MINUTES = 3 * TEN_MINUTES;
const long ONE_HOUR = 6 * TEN_MINUTES;
const int SUNRISE = 6;
const int SUNSET = 19;

// Flags to ensure color is switched only once at needed time.
int global_brightness = 127;

long last_hour = -1;
bool circadian_mode;

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
    color_all_leds(new_color);
}

/*
  Receives an hour (0 to 23) and calls for handle color with
  the corresponding circadian color.
*/
void handleHour(const char* hour) {
  long current_hour = strtol(hour, NULL, 10);
  if(current_hour != last_hour)
    handleColor(circadianColors[current_hour].c_str());
  last_hour = current_hour;
}

void handleBrightness(const char* new_brightness) {
  long long_new_brightness = strtol(new_brightness, NULL, 10);
  Serial.println(long_new_brightness);
  set_brightness(static_cast<int>(long_new_brightness));
}

// ---------------------------------------------------- SETUP --------------------------------------------------------

void setup() {
  // Set up the FastLED library
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  color_all_leds(CRGB::Green);
  led_show();
  set_brightness(global_brightness);

  for(int i = 0; i<3;i++) {
    pulse();
    delay(300);
  }

  // Initialize Serial Communication on the ESP32 board. Sunset and Sunrise Color Change
  Serial.begin(ESP32_STANDARD_BAUD_RATE);

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
void color_leds_range(CRGB color, int start, int end) {
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
  color_leds_range(color,0,NUM_LEDS);
}

// Wrapper function for FastLED.show(), shows the changes in the led.
void led_show() {
  FastLED.show();
}

// Switches the led color to the given color with a pulse animation.
void switch_color(CRGB new_color) {
  color_all_leds(new_color);
  pulse();
}

// Changes the strip's brightness to the given brightness, without changing the global brightness variable.
void set_temp_brightness(int brightness) {
  FastLED.setBrightness(brightness);
  led_show();
}

// Changes the strip's brightness to the given brightness, including the global brightness variable.
void set_brightness(int brightness) {
  global_brightness = brightness;
  FastLED.setBrightness(brightness);
  led_show();
}

// -------------------------------------------------- Animations -----------------------------------------------------
// Changes the strip's brightness to the given brightness
void pulse() {
  std::vector<int> brightness_array = {0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255};
  for(int i = 0; i < brightness_array.size(); i++) {
    if(brightness_array[i]<=global_brightness) {
      set_temp_brightness(brightness_array[i]);
      delay(25);
    } else {
      set_temp_brightness(global_brightness);
      break;
    }
  }
}

// A wave pattern for the led strip.
void wave_pattern(CRGB color) {
    for(int i = 0; i<NUM_LEDS;i++) {
    leds[i] = color;
    led_show();
    delay(5);
  }
    delay(75);
  for(int i = 0; i<NUM_LEDS;i++) {
    leds[i] = CRGB::Black;
    led_show();
    delay(5);
  }
}

// A police pattern for the led strip, alternates Red and Blue.
void police_pattern() {
    pattern_counter++%2 == 0 ? color_all_leds(CRGB::Red) : color_all_leds(CRGB::Blue);
    if(pattern_counter>20)
      pattern_counter = 0;
    delay(500);
}


