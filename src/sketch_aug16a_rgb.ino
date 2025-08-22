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
int global_brightness = 127;

// ------------------------------- SERVER -----------------------------------------

#define WEB_SERVER_PORT 80

WebServer server(WEB_SERVER_PORT);

// Define the data pin I'm using (D23)
#define DATA_PIN 23

// Handles insertion of the base server url (IP) in the brightness (shows the gui)
void handle_root() {

  String meta = "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";

  const String BUTTON_PRIMARY = "#ffffff";
  const String BUTTON_SECONDARY = "#3B82F6";
  //#38BDF8
  //#60A5FA
  // Blue - 500 - #3B82F6

  String css = "body {background-color: #F3F4F6; font-family: Arial, sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; min-height: 100vh;}\n";
  css += ".styled-button {width: 160px; border-radius: 40px; cursor: pointer; padding: 8px 16px; background-color: " + BUTTON_PRIMARY + "; border: 1px solid " + BUTTON_SECONDARY + "; color: " + BUTTON_SECONDARY + ";}\n";
  css += ".styled-button:hover {background-color: " + BUTTON_SECONDARY + "; color: #ffffff; transition-property: background-color, color; transition-duration: 0.5s;}";
  css += ".organized-col {display: flex; flex-direction: column; align-items:center; justify-content:center; gap: 20px; text-align: center;}";
  css += ".container {width: 80vw; max-width: 500px; background-color: #ffffff; padding: 16px; border-radius: 15px; box-shadow: rgba(149, 157, 165, 0.2) 0px 8px 24px; display: flex; flex-direction: column; align-items:center; gap:10px;}";
  css += "h1, h2, p, button, input, form {margin: 1px; padding: 1px;}";

  // A generic API Call JavaScript to handle all kinds of API Calls.
  String js = "async function sendApiCall(endpoint) {\n";
  js += "  try {\n";
  js += "    const response = await fetch(endpoint);\n";
  js += "    if (response.ok) {\n";
  js += "      const message = await response.text();\n";
  js += "      alert('Success: ' + message);\n";
  js += "    } else {\n";
  js += "      alert('Error: ' + response.status + ' ' + response.statusText);\n";
  js += "    }\n";
  js += "  } catch (error) {\n";
  js += "    alert('Network error: ' + error);\n";
  js += "  }\n";
  js += "}\n";
  //Vectors and icons by <a href="https://www.svgrepo.com" target="_blank">SVG Repo</a>
  String html = "<html><head>";
  html += meta;
  html += "<style>" + css + "</style>";
  html += "<script>" + js + "</script>";
  html += "</head><body>";
  html += "<div class='organized-col'>";
  html += "<div class='container'>";
  html += "<svg height='64px' width='64px' version='1.1' id='Layer_1' xmlns='http://www.w3.org/2000/svg' xmlns:xlink='http://www.w3.org/1999/xlink' viewBox='0 0 512 512' xml:space='preserve' fill='#000000'><g id='SVGRepo_bgCarrier' stroke-width='0'></g><g id='SVGRepo_tracerCarrier' stroke-linecap='round' stroke-linejoin='round'></g><g id='SVGRepo_iconCarrier'> <path style='fill:#B9B9B9;' d='M512,297.793v-83.585l-52.337-5.233c-5.266-22.911-14.302-44.37-26.41-63.713l33.319-40.724 l-59.111-59.111l-40.724,33.319c-19.343-12.108-40.802-21.144-63.713-26.41L297.793,0h-83.585l-5.233,52.337 c-22.911,5.266-44.37,14.302-63.713,26.41l-40.724-33.319l-59.111,59.111l33.319,40.724c-12.108,19.343-21.144,40.802-26.41,63.713 L0,214.207v83.585l52.337,5.233c5.266,22.911,14.302,44.37,26.41,63.713l-33.319,40.724l59.111,59.111l40.724-33.319 c19.343,12.108,40.802,21.144,63.713,26.41L214.207,512h83.585l5.233-52.337c22.911-5.266,44.37-14.302,63.713-26.41l40.724,33.319 l59.111-59.111l-33.319-40.724c12.108-19.343,21.144-40.802,26.41-63.713L512,297.793z M257.834,431.532 c-0.608,0.011-1.227,0.011-1.834,0.011c-96.942,0-175.543-78.589-175.543-175.543S159.058,80.457,256,80.457 c0.608,0,1.227,0,1.834,0.011C335.298,81.684,397.785,159.8,397.785,256S335.298,430.316,257.834,431.532z'></path> <path style='fill:#737373;' d='M257.866,80.47v82.743c50.408,0.997,90.972,42.142,90.972,92.788s-40.564,91.791-90.972,92.788 v82.743c96.095-0.996,173.677-79.196,173.677-175.531S353.961,81.465,257.866,80.47z'></path> <path style='fill:#969696;' d='M257.834,80.468c-0.608-0.011-1.227-0.011-1.834-0.011c-96.942,0-175.543,78.589-175.543,175.543 S159.058,431.543,256,431.543c0.608,0,1.227,0,1.834-0.011c77.464-1.215,139.95-79.332,139.95-175.532 S335.298,81.684,257.834,80.468z M256.002,348.835c-51.271,0-92.835-41.564-92.835-92.835s41.564-92.835,92.835-92.835 s92.835,41.564,92.835,92.835S307.273,348.835,256.002,348.835z'></path> <path style='fill:#B9B9B9;' d='M256,143.473l-10.165,17.554l10.167,16.204c43.503,0,78.769,35.266,78.769,78.769 s-35.266,78.769-78.769,78.769l-10.517,16.159L256,368.527c62.149,0,112.527-50.379,112.527-112.527S318.149,143.473,256,143.473z'></path> <path style='fill:#DCDCDC;' d='M177.233,256c0-43.502,35.265-78.768,78.767-78.769v-33.758 c-62.149,0-112.527,50.379-112.527,112.527S193.851,368.527,256,368.527v-33.758C212.498,334.768,177.233,299.502,177.233,256z'></path> </g></svg>";
  html += "<h1>Settings</h1>";
  html += "<p>Manage your overall setup and preferences for the light strip, such as color and brightness.</p>";
  html += "</div>";
  html += "<div class='container'>";
  html += "<h2>Color</h2>";
  html += "<p>Click a button to change the color.</p>";
  html += "<button class='styled-button' onclick=\"sendApiCall('/warm')\">Set to Warm</button>";
  html += "<button class='styled-button' onclick=\"sendApiCall('/cold')\">Set to Cold</button>";
  html += "<button class='styled-button' onclick=\"sendApiCall('/auto')\">Set to Auto</button>";
  html += "</div>";
  html += "<div class='container organized-col'>";
  html += "<h2>Brightness</h2>";
  html += "<form class='organized-col' action='/brightness'>";
  html += "    <input type='range' name='value' min='0' max='255' step='1' value='" + String(global_brightness) + "'>";
  html += "    <input class='styled-button' type='submit' value='Set Brightness'>";
  html += "</form>";
  html += "</div>";
  html += "</div>";
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
    server.send(400, "text/plain", "Missing brightness parameter");
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


