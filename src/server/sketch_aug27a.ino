#include <WebServer.h>
#include <LittleFS.h>
#include <WiFi.h>
#include "secrets.h"
#include <PicoMQTT.h>
#define WEB_SERVER_PORT 80

// Create a web server.
WebServer server(WEB_SERVER_PORT);

// Create MQTT Broker
PicoMQTT::Server mqttBroker;

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
  // Example - 0xFFFFFF - 16,777,215, int goes all the way up to 2,147,483,647.
  // 0xFFFFFF
  // 1111 1111 1111 1111 1111 1111
  // In this setting, both long and int are 4 bytes.
  String color = server.arg("hex");
  /*
  Serial.println(color);
  // Convert string to int.
  int rgb = std::stoi(color.c_str(), NULL, 16);
  Serial.println(rgb);
  // Extract R, G and B values.
  // 1111 1111 1111 1111 1111 1111 => (Shift right 16 bits) 0000 0000 0000 0000 1111 1111
  //                                                        0000 0000 0000 0000 1111 1111
  int red = (rgb >> 16) & 0xFF;
  Serial.println(red);
  // 1111 1111 1111 1111 1111 1111 => (Shift right 8 bits) 0000 0000 1111 1111 1111 1111
  //                                                       0000 0000 0000 0000 1111 1111
  int green = (rgb >> 8) & 0xFF;
  Serial.println(green);
  // 1111 1111 1111 1111 1111 1111
  // 0000 0000 0000 0000 1111 1111
  int blue = rgb & 0xFF;
  Serial.println(blue);
  */
  // Send message to registered clients (RGB Controllers) to change color.
  // MQTT BROKER
  mqttBroker.publish("color", color.c_str());
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
  server.begin();
  Serial.println("HTTP Server: Up.");
  Serial.print("Server IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
  mqttBroker.loop();
}

/*
  TO DO LIST:
  1. beginSerialConnection function
  2. beginFileSystem function
  3. beginWiFiConnection function
  4. abort function - closes everything and turns the board off
  5. Look into PlatformIO - Automated unit testing (VSCode extension).
*/
