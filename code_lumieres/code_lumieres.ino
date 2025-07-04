#include <Adafruit_NeoPixel.h>

#define PIN        25
#define NUMPIXELS  12

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int luminosite;

void setup() {
  Serial.begin(9600);
  analogSetAttenuation(ADC_11db); // Autorise une plage jusqu'à ~3.3V en entrée ADC

  strip.begin();
  strip.show(); // Éteint tous les pixels au démarrage
}

void loop() {
  luminosite = analogRead(33);
  Serial.println(luminosite);
  eclairage();
  delay(50);
}

void eclairage(){
  if (luminosite > 450) {
    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
  } else {
    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, strip.Color(240, 220, 200));
    }
  }
  strip.show();
}