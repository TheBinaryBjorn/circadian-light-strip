#include <FastLED.h>
#include "secrets.h"
#include <WiFi.h>
#include <time.h>


// Define the data pin I'm using (D23)
#define DATA_PIN 23

// RGB Strip Constants
// Define Number of LEDs
#define NUM_LEDS 120

// Define LED Aarray.
CRGB leds[NUM_LEDS];

const CRGB WARM = CRGB(255,40,0);
const CRGB COLD = CRGB::Cyan;

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
const int SUNRISE = 6;
const int SUNSET = 19;

// Flags to ensure color is switched only once at needed time.
bool warm_set = false;
bool cold_set = false;


// ---------------------------------------------------- SETUP --------------------------------------------------------

void setup() {
  // Initialize Serial Communication on the ESP32 board.
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

  // Set up the FastLED library
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
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
    color_all_leds(WARM);
    warm_set = true;
    cold_set = false;
  }

  if(current_hour >= SUNRISE && current_hour < SUNSET && !cold_set) {
    color_all_leds(COLD);
    cold_set = true;
    warm_set = false;
  }

  // Check the time every minute
  delay(ONE_MINUTE);
}

// ----------------------------------------------- Color Functions ----------------------------------------------------------

// Sets LEDs in a given range to a given color.
void color_leds_range(CRGB color, int start, int end) {
  // check if ranges are valid.
  if(end >= NUM_LEDS || end < 0) {
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
  FastLED.show();
}

// Sets all the LEDs in the strip to a given color - in my setup, 0 to 119
void color_all_leds(CRGB color) {
  color_leds_range(color,0,NUM_LEDS - 1);
}

// =================================================== Not Important ============================================================

void wave_pattern(CRGB color) {
    for(int i = 0; i<NUM_LEDS;i++) {
    leds[i] = color;
    FastLED.show();
    delay(5);
  }
    delay(75);
  for(int i = 0; i<NUM_LEDS;i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
    delay(5);
  }
}

void police_pattern() {
    pattern_counter++%2 == 0 ? color_all_leds(CRGB::Red) : color_all_leds(CRGB::Blue);
    if(pattern_counter>20)
      pattern_counter = 0;
    delay(500);
}


