#include <WiFi.h>
#include <HTTPClient.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

const int pulsePin = 34;  // Analog pin for pulse sensor

const char* ssid = "";
const char* password = "";
String serverName = "";

unsigned long lastBeatTime = 0;
int beatCount = 0;
int bpm = 0;
bool lastPulseState = false;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  dht.begin();
  pinMode(pulsePin, INPUT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected!");
}

void loop() {
  //  Read DHT sensor
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();

  if (isnan(temp) || isnan(hum)) {
    Serial.println("Failed to read DHT sensor!");
    return;
  }

  // ️ Pulse Sensor Reading
  int sensorValue = analogRead(pulsePin);
  bool pulseDetected = sensorValue > 2000; // Threshold (tune this)

  if (pulseDetected && !lastPulseState) {
    beatCount++;
    lastBeatTime = millis();
  }
  lastPulseState = pulseDetected;

  // Calculate BPM every 15 seconds
  static unsigned long lastCalcTime = 0;
  if (millis() - lastCalcTime >= 15000) {
    bpm = (beatCount * 4); // 15s * 4 = BPM
    beatCount = 0;
    lastCalcTime = millis();
    Serial.print("BPM: ");
    Serial.println(bpm);
  }

  //  Send data to ThingSpeak
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = serverName +
                 "&field1=" + String(temp) +
                 "&field2=" + String(hum) +
                 "&field3=" + String(bpm);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      Serial.println("Data sent successfully!");
    } else {
      Serial.println("Error sending data.");
    }
    http.end();
  }

  // Alert logic
  if (temp > 40 || temp < 30) {
    Serial.println("Temperature Alert!");
  }
  if (bpm < 50 || bpm > 120) {
    Serial.println("Heart Rate Alert!");
  }

  delay(100); // small delay for stable pulse reading
}
