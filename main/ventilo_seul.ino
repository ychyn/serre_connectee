#include <Wire.h>
#include <Adafruit_Sensor.h>


#define SEALEVELPRESSURE_HPA (1013.25)
#define AOUT_PIN 4 // ESP32 pin GPIO36 (ADC0) that connects to AOUT pin of soil moisture sensor


unsigned long delayTime = 3000;
bool status;
int min_h = 7130;
int max_h = 4770;

int humidity = 0;
int m = 0;

const int VENTILO_PIN = 5; // Exemple de pin reliée au relais du ventilo
int threshold_humidity = 30;           // Seuil d'humidité pour activer le ventilo

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db); // set the ADC attenuation to 11 dB (up to ~3.3V input)
  
  
  pinMode(VENTILO_PIN, OUTPUT);
  digitalWrite(VENTILO_PIN, LOW);  // Ventilo éteint au départ
}

void loop() {
  delay(1000);
  
  m = analogRead(AOUT_PIN);
  humidity = map(m, min_h, max_h, 0, 100); // Remplace par ta fonction de lecture réelle
  Serial.println(humidity);
  if (humidity >= threshold_humidity) {
    digitalWrite(VENTILO_PIN, HIGH);  // Allume le ventilo
  } 
  else {
    digitalWrite(VENTILO_PIN, LOW);   // Éteint le ventilo
  }

}
