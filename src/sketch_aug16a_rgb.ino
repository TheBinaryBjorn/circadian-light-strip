#include <FastLED.h>
#include "secrets.h"
#include <WiFi.h>
#include <time.h>
#include <vector>
#include <WebServer.h>
// ------------------------------- Function Declerations ---------------------------
// Handles insertion of the base server url (IP) in the brightness (shows the gui)
void handle_root();
// Handles the press of the set to warm button, to set the strip to warm color.
void handle_warm();
// Handles the press of the set to cold button, to set the strip to cold color.
void handle_cold();
// Handles the press of the set to auto button, and sets the strip to the circadian automation.
void handle_auto();
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
bool auto_mode = true;
unsigned long last_time_check = 0;
int global_brightness = 150;

// ------------------------------- SERVER -----------------------------------------

#define WEB_SERVER_PORT 80

WebServer server(WEB_SERVER_PORT);

// Define the data pin I'm using (D23)
#define DATA_PIN 23

// Handles insertion of the base server url (IP) in the brightness (shows the gui)
void handle_root() {
  String html = "<html><body>";
  html += "<h1>LED Strip Control</h1>";
  html += "<p>Click a button to change the color.</p>";
  html += "<p><a href='/warm'><button>Set to Warm</button></a></p>";
  html += "<p><a href='/cold'><button>Set to Cold</button></a></p>";
  html += "<p><a href='/auto'><button>Set to Auto</button></a></p>";
  html += "<h2>Brightness Control</h2>";
  html += "<form action='/brightness'>";
  html += "    <input type='range' name='value' min='0' max='255' step='1' value='" + String(global_brightness) + "'>";
  html += "    <input type='submit' value='Set Brightness'>";
  html += "</form>";
  html += "</body></html>";
  server.send(200,"text/html",html);
}

// Handles the press of the set to warm button, to set the strip to warm color.
void handle_warm() {
  switch_color(WARM);
  warm_set = true;
  cold_set = false;
  auto_mode = false;
  server.send(200,"text/plain","Switched to Warm");
}

// Handles the press of the set to cold button, to set the strip to cold color.
void handle_cold() {
  switch_color(COLD);
  warm_set = false;
  cold_set = true;
  auto_mode = false;
  server.send(200,"text/plain","Switched to Cold");
}

// Handles the press of the set to auto button, and sets the strip to the circadian automation.
void handle_auto() {
  auto_mode = true;
  server.send(200,"text/plain","Switched to Auto");
}

void handle_brightness() {
  if(server.hasArg("value")) {
    int new_brightness = server.arg("value").toInt();
    set_brightness(new_brightness);
    server.sendHeader("Location","/");
    server.send(302,"text/plain","Brightness set successfully.");
  } else {
    server.send(400, "text/plain", "Mising brightness parameter");
  }
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

  // Initialize the time from the NTP Server
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  // Set up web server routes
  server.on("/",handle_root);
  server.on("/warm",handle_warm);
  server.on("/cold",handle_cold);
  server.on("/auto",handle_auto);
  server.on("/brightness",handle_brightness);

  //Start the server
  server.begin();
  Serial.println("HTTP server started on: ");
  Serial.print(WiFi.localIP());
}
// --------------------------------------------------- LOOP -------------------------------------------------------------

void loop() {
  // Change strip colors at sunrise and sunset.
  server.handleClient();
  // Get the current time.
  if(auto_mode) {
    if(millis() - last_time_check > ONE_MINUTE) {
      last_time_check = millis();
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
    }
  }
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


