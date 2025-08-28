#include <WebServer.h>
#include <LittleFS.h>
#include <WiFi.h>
#include "secrets.h"
#include <PicoMQTT.h>
#include <time.h>

#define WEB_SERVER_PORT 80

// Create a web server.
WebServer server(WEB_SERVER_PORT);

// Create MQTT Broker
PicoMQTT::Server mqttBroker;

// Set up NTP Protocol
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

// Colors:
const String WARM = "FF2800";
const String COLD = "00FFFF";

// Handles the press of the set to warm button, to set the strip to warm color.
void handleWarm() {
  publishColor(WARM.c_str());
  warm_set = true;
  cold_set = false;
  auto_mode = false;
  server.send(200,"text/plain","Switched to Warm");
}

// Handles the press of the set to cold button, to set the strip to cold color.
void handleCold() {
  publishColor(COLD.c_str());
  warm_set = false;
  cold_set = true;
  auto_mode = false;
  server.send(200,"text/plain","Switched to Cold");
}

// Handles the press of the set to auto button, and sets the strip to the circadian automation.
void handleAuto() {
  auto_mode = true;
  server.send(200,"text/plain","Switched to Auto");
}

void handleRoot() {
  // Open index.html (gui) in read mode.
  File file = LittleFS.open("/index.html","r");

  // Check if the file opened correctly.
  if(!file) {
    // If it didn't send a 404 not found error.
    // server.send(response number, response type, response message)
    server.send(404, "text/plain", "index.html not found");
    Serial.println("FileSystem(LittleFS): Failed to open index.html.");
    return;
  }

  // Read the html content into a string.
  String html = file.readString();

  // Close the file descriptor (it's a good practice!).
  file.close();

  // Return the stringifyied html to the one that accessed the endpoint.
  server.send(200, "text/html", html);
  Serial.println("Server: index.html served successfully.");
}

void handleColor() {
  // Extract color from server parameters.
  String color = server.arg("hex");
  // Send message to registered clients (RGB Controllers) to change color.
  // MQTT BROKER
  publishColor(color.c_str());
}

void publishColor(const char* colorHex) {
    mqttBroker.publish("color", colorHex);
} 

void setup() {
  /* 
    Open a Serial communication channel with a selected baud rate.
    This opens the gate (or pipe or cable, however you feel like calling it)
    for the ESP32 board to communicate with the console (serial) in the 
    Arduino IDE. It's like opening a gate at the selected cirumfence to let
    data flood into the computer and back.
  */
  const int ESP32_STANDARD_BAUD_RATE = 115200;
  Serial.begin(ESP32_STANDARD_BAUD_RATE);
  if(Serial) {
    Serial.println("Serial: Up.");
  } else {
    // Abort function/logic
  }

  /*
    Initialize the file system.
    Basically boots up the "windows explorer" on the ESP32
    LittleFS.begin(true) starts the file system and returns
    true/false if it succeed or failed.
  */
  if(LittleFS.begin(true)) {
    Serial.println("FileSystem(LittleFS): Up.");
  } else {
    // Abort function/logic
    Serial.println("FileSystem(LittleFS): Failed.");
  }

  /*
    Initialize Wi-Fi Connection:
    Basically just connects to your wifi network with
    the given network name and password (in the secrets.h file),
    just like you would from your computer or phone.
  */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to WiFi...");

  long startTime = millis();
  const long timeout = 10000;

  while(WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
    delay(500);
    Serial.print(".");
  }

  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi: Connected.");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi: Failed.");
    // Abort function/logic
    Serial.println("Aborting.");
  }

  // Initialize the time from the NTP Server
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);

  /*
    Initialize MQTT Broker
  */
  mqttBroker.begin();
  Serial.println("MQTT Broker: Up.");
  /*
    Initialize Web Server.
    How would I test that? Just try to access the server?

  */
  server.on("/",handleRoot);
  server.on("/setColor", handleColor);
  server.on("/cold", handleCold);
  server.on("/warm",handleWarm);
  server.on("/auto",handleAuto);
  server.begin();
  Serial.println("HTTP Server: Up.");
  Serial.print("Server IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
  mqttBroker.loop();
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
        publishColor(WARM.c_str());
        warm_set = true;
        cold_set = false;
      }

      if(current_hour >= SUNRISE && current_hour < SUNSET && !cold_set) {
        publishColor(COLD.c_str());
        cold_set = true;
        warm_set = false;
      }
    }
  }
}

/*
  TO DO LIST:
  1. beginSerialConnection function
  2. beginFileSystem function
  3. beginWiFiConnection function
  4. abort function - closes everything and turns the board off
  5. Look into PlatformIO - Automated unit testing (VSCode extension).
*/
