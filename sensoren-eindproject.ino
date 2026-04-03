#define BLYNK_TEMPLATE_ID "TMPL5CYzdexes"
#define BLYNK_TEMPLATE_NAME "sensoren eindproject"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <DHT.h>
#include <Wire.h>
#include <BH1750.h>
#include <ESP32Servo.h>

char auth[] = "xUKmUjrXs9N9cv45wWvdi1aNt7BXwJDn";
char ssid[] = "Your Wifi";
char pass[] = "Wifi password";

#define SOIL_PIN 4
#define WATER_PIN 5
#define DHTPIN 7
#define SERVO_PIN 6

#define VPIN_SOIL V0
#define VPIN_WATER V1
#define VPIN_TEMP V2
#define VPIN_LIGHT V3
#define VPIN_SERVO V4
#define VPIN_AUTO V5
#define VPIN_STATUS V6

DHT dht(DHTPIN, DHT11);
BH1750 lightMeter;
Servo waterServo;

BlynkTimer timer;

int servoOpen = 100;
int servoClose = 10;

bool autoMode = true;

BLYNK_WRITE(VPIN_SERVO)
{
  int value = param.asInt();

  if (!autoMode) {
    if (value == 1) {
      waterServo.write(servoOpen);
      Blynk.virtualWrite(VPIN_STATUS, "Handmatig water geven");
    } else {
      waterServo.write(servoClose);
      Blynk.virtualWrite(VPIN_STATUS, "Water gestopt");
    }
  }
}

BLYNK_WRITE(VPIN_AUTO)
{
  autoMode = param.asInt();
}

void sendSensorData()
{
  int soil = analogRead(SOIL_PIN);
  int water = analogRead(WATER_PIN);
  float temp = dht.readTemperature();
  float lux = lightMeter.readLightLevel();

  Serial.println("----- SENSOR DATA -----");
  Serial.print("Bodemvocht: "); Serial.println(soil);
  Serial.print("Waterniveau: "); Serial.println(water);
  Serial.print("Temperatuur: "); Serial.println(temp);
  Serial.print("Licht: "); Serial.println(lux);

  Blynk.virtualWrite(VPIN_SOIL, soil);
  Blynk.virtualWrite(VPIN_WATER, water);
  Blynk.virtualWrite(VPIN_TEMP, temp);
  Blynk.virtualWrite(VPIN_LIGHT, lux);

  int droogDrempel = 3000;  
  int natDrempel = 2500;    

  if (autoMode) {

    if (soil > droogDrempel && water > 100) {
      waterServo.write(servoOpen);
      Blynk.virtualWrite(VPIN_STATUS, "Auto: Plant krijgt water (grond droog)");
      Serial.println("AUTO WATER AAN");
    }

    else if (soil < natDrempel) {
      waterServo.write(servoClose);
      Blynk.virtualWrite(VPIN_STATUS, "Auto: Water uit (grond vochtig)");
      Serial.println("AUTO WATER UIT");
    }
  }

  if (water < 500) {
    Blynk.virtualWrite(VPIN_STATUS, "Waterreservoir bijna leeg");
  }

  Serial.println("-----------------------\n");
}

void setup()
{
  Serial.begin(115200);

  pinMode(SOIL_PIN, INPUT);
  pinMode(WATER_PIN, INPUT);

  waterServo.setPeriodHertz(50);
  waterServo.attach(SERVO_PIN, 500, 2400);
  waterServo.write(servoClose);

  dht.begin();
  Wire.begin(8, 9);   
  lightMeter.begin();

  Serial.println("Verbinden met Blynk...");
  Blynk.begin(auth, ssid, pass);

  timer.setInterval(3000L, sendSensorData);

  Serial.println("Systeem gestart");
}

void loop()
{
  Blynk.run();
  timer.run();
}
