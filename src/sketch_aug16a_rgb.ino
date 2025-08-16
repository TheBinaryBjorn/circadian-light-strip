#include <FastLED.h>
#include "secrets.h"

// Define the data pin I'm using (D23)
#define DATA_PIN 23
// Define Number of LEDs
#define NUM_LEDS 120

// Define LED Aarray.
CRGB leds[NUM_LEDS];

int pattern_counter = 0;
const CRGB WARM = CRGB(255,40,0);
const CRGB COLD = CRGB::Cyan;

void setup() {
  // Set up the FastLED library
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  color_all_leds(WARM);
}

void loop() {
  /*
  // Turn the first LED on (red)
  leds[0] = CRGB::Red;
  FastLED.show(); // Push the data to the LEDs
  delay(1000); // Wait for one second.

  // Turn the first LED off
  leds[0] = CRGB::Black;
  FastLED.show();
  delay(1000);
  */
}

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

void color_leds_range(CRGB color, int start, int end) {
  // check if ranges are valid.
  for(int i = start; i < end; i++)
    leds[i] = color;
  FastLED.show();
}

void color_all_leds(CRGB color) {
  color_leds_range(color,0,NUM_LEDS);
}
