#include <FastLED.h>

#define NUM_LEDS 60
#define DATA_PIN 12

CRGB leds[NUM_LEDS];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(DATA_PIN, OUTPUT);
  
  // put your setup code here, to run once:
  FastLED.addLeds<TM1809, DATA_PIN>(leds, NUM_LEDS);
}

void loop() {
  // put your main code here, to run repeatedly:

  for (int i=0; i < NUM_LEDS; i++){
    leds[i] = CRGB::White; 
    FastLED.show(); 
    delay(30); 
  }

  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);      

}
