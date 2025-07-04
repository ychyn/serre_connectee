#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include "time.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define AOUT_PIN 36 // ESP32 pin GPIO36 (ADC0) that connects to AOUT pin of soil moisture sensor
#define LED 2
#define PIN 25
#define NUMPIXELS 12

Servo myservo;
int servoPin = 27;

const char* ssid     = "Mi Teléfono";
const char* password = "tomamiwifi";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

Adafruit_BME280 bme; // Initialisation capteur temperature et humidité
Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800); // Initialisation pins lumières led
LiquidCrystal lcd(19, 23, 18, 17, 16, 15); // Initialisation pins ecran LCD

AsyncWebServer server(80); // Initialisation serveur web pour verser nos mesures

// Definition de variables utiles
unsigned long delayTime = 500;
bool status;
int min_h = 2665;
int max_h = 1650;
int m;
int moisture;
float temperature;
float humidity;
int luminosite;

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(LED, OUTPUT);
  analogSetAttenuation(ADC_11db); // set the ADC attenuation to 11 dB (up to ~3.3V input)
  
  lcd.begin(16, 2); //Lancer l'écran LCD
  Wire.begin(); // Lancer le capteur BME
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    delay(delayTime);
  }

  strip.begin(); // Lancer le système de lumières
  strip.show(); // Éteint tous les pixels au démarrage


  // Connexion au Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Configuration pour l'obtention de la date et heure
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  Serial.println();

  // Obtention de l'adresse IP du serveur Web
  Serial.print("ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());

  // Configuration du serveur web
  server.on("/data", [](AsyncWebServerRequest *request) {
  StaticJsonDocument<200> doc;
  doc["Temperature"] = temperature;
  doc["soil moisture"] = moisture;
  doc["Air moisture"] = humidity;
  doc["Luminosite"] = luminosite;

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
  });

  // Lancement du server
  server.begin();
  
  // Configuration servo pour vanne d'arrosage
  myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 500, 2400);

  Serial.println();
}


void loop() { 
  printValues();
  delay(delayTime);
  delay(50);
  digitalWrite(LED,HIGH);
  delay(1000);
  digitalWrite(LED,LOW);
//  printLocalTime();

  eclairage(); 
  waterPlant();

  //Obtention de l'heure
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    lcd.setCursor(0, 0);
    lcd.print("Erreur NTP");
    return;
  }

  // Préparation de variables à afficher sur l'écran
  char bufferBME[10]; //format "T=II°C H=II"
  char bufferSM[16]; //format "M=II%"
  snprintf(bufferBME, sizeof(bufferBME), "T=%.2fC", bme.readTemperature());
  snprintf(bufferSM, sizeof(bufferSM), "H=%.2f%% SM=%d%% ",bme.readHumidity(), moisture);

  // Affichage sur l'écran LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(bufferBME);
  lcd.setCursor(0, 1);
  lcd.print(bufferSM);

  delay(delayTime);
}

void printValues() {
  temperature = bme.readTemperature();
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.print(" *C;   ");

  
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.print(" hPa;   ");

  humidity = bme.readHumidity();
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.print(" %;   ");

  Serial.print("Soil moisture value: ");
  m = analogRead(AOUT_PIN);
  moisture = map(m, min_h, max_h, 0, 100);
  Serial.print(moisture);
  Serial.print(" %;   ");

  luminosite = analogRead(33);
  Serial.print("Luminosité = ");
  Serial.println(luminosite);

  Serial.print("Status");
  Serial.println(status);
  Serial.println();

}

void waterPlant(){
  if (moisture < 20){
    myservo.write(60);
  }
  else{
    myservo.write(170);
  }
}

void eclairage(){
  if (luminosite > 450) {
    // Si la luminosité est forte, leds éteintes
    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0));
    }
  } else {
    // Sinon, on l'allume sur une couleur blanche
    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, strip.Color(240, 220, 200));
    }
  }
  strip.show();
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%H:%M:%S");  //"%A, %B %d %Y 
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");

  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();
}