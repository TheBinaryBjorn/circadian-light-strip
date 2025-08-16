#include <FastLED.h>
#include "secrets.h"
#include <WiFi.h>
#include <time.h>
#include <vector>


// Define the data pin I'm using (D23)
#define DATA_PIN 23

// RGB Strip Constants
// Define Number of LEDs
#define NUM_LEDS 120

// Define LED Aarray.
CRGB leds[NUM_LEDS];

/*
  1 - Off
  2 - Off
  3 - Off
  4 - Off
  5 - Off
  6 - Sunrise - On - Bluest
  7 - Blue
  8
  9
  10
  11
  12
  13
  14
  15
  16
  17
  18
  19 - Sunset - Lightly Warm
  20 - Warm
  21 - Warmest
  22 - Sleep - Off
  23
  24
  0
*/
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

// NTP Server Constants - Network Time Protocol
const long GMT_OFFSET_SEC = 10800;
const int DAYLIGHT_OFFSET_SEC = 0;
const char* NTP_SERVER = "pool.ntp.org";

// Time Constants (Milisec)
const long ONE_MINUTE = 60000;
const long FIVE_MINUTES = 5 * ONE_MINUTE;
const long TEN_MINUTES = 2 * FIVE_MINUTES;
const long THIRTY_MINUTES = 3 * TEN_MINUTES;
const long ONE_HOUR = 6 * TEN_MINUTES;
const int SUNRISE = 6;
const int SUNSET = 19;

// Flags to ensure color is switched only once at needed time.
bool warm_set = false;
bool cold_set = false;


// ---------------------------------------------------- SETUP --------------------------------------------------------

void setup() {
  // Set up the FastLED library
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  color_all_leds(CRGB::Green);
  led_show();

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

  // Initialize the time from the NTP Server
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
}
// --------------------------------------------------- LOOP -------------------------------------------------------------

void loop() {
  // Change strip colors at sunrise and sunset.
  // Get the current time.
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // Print local time to Serial Monitor
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

  int current_hour = timeinfo.tm_hour;
  Serial.println(current_hour);
  if(current_hour >= SUNSET && !warm_set) {
    switch_color(WARM);
    warm_set = true;
    cold_set = false;
  }

  if(current_hour >= SUNRISE && current_hour < SUNSET && !cold_set) {
    switch_color(COLD);
    cold_set = true;
    warm_set = false;
  }

  // Check the time 1 minute
  delay(ONE_MINUTE);
}

// ----------------------------------------------- Color Functions ----------------------------------------------------------

// Sets LEDs in a given range to a given color.
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

// Sets all the LEDs in the strip to a given color - in my setup, 0 to 119
void color_all_leds(CRGB color) {
  color_leds_range(color,0,NUM_LEDS);
}

// Wraper Design pattern, in case FastLED renames/decrypts function.
void led_show() {
  FastLED.show();
}

// =================================================== Not Important ============================================================

void switch_color(CRGB new_color) {
  color_all_leds(new_color);
  pulse();
}

void pulse() {
  std::vector<int> brightness_array = {0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 255};
  for(int i = 0; i < brightness_array.size(); i++) {
    set_brightness(brightness_array[i]);
    delay(25);
  }
}

void set_brightness(int brightness) {
  FastLED.setBrightness(brightness);
  led_show();
}

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

void police_pattern() {
    pattern_counter++%2 == 0 ? color_all_leds(CRGB::Red) : color_all_leds(CRGB::Blue);
    if(pattern_counter>20)
      pattern_counter = 0;
    delay(500);
}


