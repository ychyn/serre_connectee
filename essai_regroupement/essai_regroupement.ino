#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <LiquidCrystal.h> // Librairie pour Ecran LCD
#include <ESP32Servo.h>
#include <WiFi.h>
#include "time.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

/*#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5*/

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

Adafruit_BME280 bme; // I2C

Adafruit_NeoPixel strip(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

unsigned long delayTime = 500;
bool status;
int min_h = 2665;
int max_h = 1650;
int m;
int moisture;
float temperature;
float humidity;
int luminosite;


LiquidCrystal lcd(19, 23, 18, 17, 16, 15); // On initialise la librairie avec les pins ou l'ecran est branche

// // PWM pour le ventilateur
const int PWM_CHANNEL = 0;    // ESP32 has 16 channels which can generate 16 independent waveforms
const int PWM_FREQ = 5000;     // Recall that Arduino Uno is ~490 Hz. Official ESP32 example uses 5,000Hz
const int PWM_RESOLUTION = 8; // We'll use same resolution as Uno (8 bits, 0-255) but ESP32 can go up to 16 bits

// The max duty cycle value based on PWM resolution (will be 255 if resolution is 8 bits)
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1); 

const int Ventilo_OUTPUT_PIN = 4;  // Le pin où est branché le ventilateur

const int DELAY_MS = 4;  // delay between fade incrementsDan

// On va coder une plage de température sur laquelle la vitesse de notre ventilateur sera progressive
const float TEMP_MIN = 10.0; // Température à laquelle le ventilateur s'allume
const float TEMP_MAX = 30.0; // Température à laquelle le ventilateur est à sa vitesse maximale

AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);
  delay(1000);
  pinMode(LED, OUTPUT);

  analogSetAttenuation(ADC_11db); // set the ADC attenuation to 11 dB (up to ~3.3V input)
  
  lcd.begin(16, 2);
  Wire.begin();

  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    delay(delayTime);
  }

  strip.begin();
  strip.show(); // Éteint tous les pixels au démarrage


  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  //WiFi.disconnect(true);
  //WiFi.mode(WIFI_OFF);

    Serial.println();

    // Print the ESP32's IP address
  Serial.print("ESP32 Web Server's IP address: ");
  Serial.println(WiFi.localIP());

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

  // Start the server
  server.begin();
  
  myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 500, 2400);

  //Ventilateur PWM setup___________
  //ledcAttach(Ventilo_OUTPUT_PIN, PWM_FREQ, PWM_RESOLUTION);

  Serial.println();
}


void loop() { 
  printValues();
  delay(delayTime);
  delay(50);
  digitalWrite(LED,HIGH);
  delay(1000);
  digitalWrite(LED,LOW);
  printLocalTime();
  eclairage();
  waterPlant();

  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    lcd.setCursor(0, 0);
    lcd.print("Erreur NTP");
    return;
  }

  // Buffer pour l'heure et la date = petits tableaux dans lesquels on va pouvoir stocker nos str
  char bufferBME[10]; //format "T=II°C H=II"
  // char bufferDate[12];  //format "JJ/MM/AAAA"
  char bufferSM[16]; //format "M=II%"
  // Formater l'heure et la date
  // strftime(bufferHeure, sizeof(bufferHeure), "%H:%M:%S", &timeinfo);
  // strftime(bufferDate, sizeof(bufferDate), "%d/%m/%Y", &timeinfo);
  snprintf(bufferBME, sizeof(bufferBME), "T=%.2fC", bme.readTemperature());
  snprintf(bufferSM, sizeof(bufferSM), "H=%.2f%% SM=%d%% ",bme.readHumidity(), moisture);

  // Affichage sur l'écran LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(bufferBME);
  lcd.setCursor(0, 1);
  // lcd.print(bufferDate);
  lcd.print(bufferSM);


  //Ventilateur PWM loop___________
  moisture = constrain(moisture, TEMP_MIN, TEMP_MAX); // Température limitée entre des bornes définie au dessus
  int duty = map(moisture, TEMP_MIN, TEMP_MAX, 0, MAX_DUTY_CYCLE); //On interpôle cette température pour que TEMP_MIN corresponde à 0 et TEMP_MAX à 255 (valeur max du duty_cycle)

  ledcWrite(PWM_CHANNEL, duty);

  delay(delayTime);
}

void printValues() {
  temperature = bme.readTemperature();
  Serial.print("Temperature = ");
  Serial.print(temperature);
  Serial.println(" *C");

  
  Serial.print("Pressure = ");
  Serial.print(bme.readPressure() / 100.0F);
  Serial.println(" hPa");

  humidity = bme.readHumidity();
  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Soil moisture value: ");
  m = analogRead(AOUT_PIN);
  moisture = map(m, min_h, max_h, 0, 100);
  Serial.print(moisture);
  Serial.println(" %");

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

void eclairage(){
  if (luminosite > 450) {
    // Si la luminosité est forte, couleur orange
    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0)); // Orange
    }
  } else {
    // Sinon, couleur bleu clair
    for (int i = 0; i < NUMPIXELS; i++) {
      strip.setPixelColor(i, strip.Color(240, 220, 200)); // Bleu clair
    }
  }
  strip.show();
}