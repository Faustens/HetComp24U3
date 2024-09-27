#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
#include <time.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
// Pins
#define LED_RED 5
#define LED_GRN 19
#define SDA_PIN 21
#define SCL_PIN 22
#define RAIN_AIN 33

// WiFi credentials
const char* ssid     = "Blowjob 4 Internet";
const char* password = "63125233436437408416";
// Zeitzonen String für MEZ/MESZ
const char* tz = "CET-1CEST,M3.5.0,M10.5.0/3";
// BME280 Variablen
#define BME280_ADDRESS 0x76
Adafruit_BME280 bme;
// Mosquitto Variablen
const char* mqtt_server = "192.168.2.133";  // Ersetze dies mit der IP-Adresse deines MQTT Brokers
const int mqtt_port = 10883;
const char* mqtt_user = "default_user";   // Ersetze dies mit deinem MQTT-Benutzernamen
const char* mqtt_password = "DefaultPassword";
const char* topic = "weather";
// MQTT client
WiFiClient espClient;
PubSubClient client(espClient);

struct TimeData {
  int year;
  int month;
  int day;
  int hour;
  int minute;
};

struct BmeData {
  float temperature;
  float pressure;
  float humidity;
};

// Function to initialize every IC and Network Connection
bool systemInit() {
// Connect to WiFi ------------------------------
  Serial.print("[INFO] Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("[SUCC] connected!");
// Synchronize local time -----------------------
  configTzTime(tz, "pool.ntp.org");
  Serial.println("[SUCC] Time synchronized");
// Initialize BME280 ----------------------------
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!bme.begin(BME280_ADDRESS)) {
    Serial.println("[ERROR] Could not find a valid BME280 sensor, check wiring!");
    return false;
  }
  Serial.println("[SUCC] BME280 initialized.");
  return true;
}

// Methode zum Trennen aller Netzwerk und I2C Verbindungen
void systemDecon() {
// I2C Verbindung trennen -----------------------
  Wire.end();
// WiFi Verbindung trennen ----------------------
  WiFi.disconnect(true);  // true bedeutet, dass die Verbindung auch vom Access Point getrennt wird
  WiFi.mode(WIFI_OFF);    // Setze den Wi-Fi-Modus auf OFF, um alle Wi-Fi-Funktionen zu deaktivieren
  Serial.println("Wi-Fi-Verbindung getrennt und Wi-Fi-Modus auf OFF gesetzt.");
}

// Gehe bis zu den nächsten vollen 10 Minuten in den Sleep (aktuell delay). 
// z.B. 17:00 sleep bis 17:10, 17:14 sleep bis 17:20.
void sleepToFullSixth() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("[ERROR] Fehler beim Abrufen der Zeit");
    return;
  }
  // Aktuelle Minuten und Sekunden abrufen
  int aktuelleMinuten = timeinfo.tm_min;
  int aktuelleSekunden = timeinfo.tm_sec;
  // Berechne verbleibende Zeit bis zur nächsten vollen 10-Minuten-Marke
  int minutenBisVoll = 10 - (aktuelleMinuten % 10);  // Minuten bis zur nächsten vollen 10-Minuten-Marke
  unsigned long sekundenBisVoll = (minutenBisVoll * 60) - aktuelleSekunden; // Berechnung der Sekunden bis zur nächsten vollen Minutenmarke

  Serial.print("[INFO] Schlafen bis zur nächsten vollen 10-Minuten-Marke in ");
  Serial.print(sekundenBisVoll);
  Serial.println(" Sekunden");

  Serial.println("[INFO] Gehe in Sleep...");
  delay(sekundenBisVoll*1000);
}

// Return BME sensor Data as float array
BmeData readBME280Data() {
  BmeData bmeData;
  bmeData.temperature = bme.readTemperature();
  bmeData.humidity    = bme.readHumidity();
  bmeData.pressure    = bme.readPressure()/100.0F;
  return bmeData;
}

// Function to get data from MH-RD rain sensor
bool readRainSensor() {
  int rainValue = analogRead(RAIN_AIN);
  return rainValue<=3000;
}

TimeData* getCurrentTime() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return nullptr;
  }
  TimeData* timeDataPtr = new TimeData;

  timeDataPtr->year   = timeinfo.tm_year + 1900;
  timeDataPtr->month  = timeinfo.tm_mon + 1;
  timeDataPtr->day    = timeinfo.tm_mday;
  timeDataPtr->hour   = timeinfo.tm_hour;
  timeDataPtr->minute = timeinfo.tm_min;

  return timeDataPtr;
}

String createJsonPayload( const TimeData &timeData, 
                          const BmeData &bmeData, 
                          bool isRaining)
{
  StaticJsonDocument<256> doc;
  doc["year"]         = timeData.year;
  doc["month"]        = timeData.month;
  doc["day"]          = timeData.day;
  doc["hour"]         = timeData.hour;
  doc["minute"]       = timeData.minute;
  doc["temperature"]  = bmeData.temperature;
  doc["pressure"]     = bmeData.pressure;
  doc["humidity"]     = bmeData.humidity;
  doc["rain"]         = isRaining;

  String output;
  serializeJson(doc, output);
  return output;
}

void sendJsonPayload(String payload) {
  if (!connectToMQTT()) {
    Serial.println("[ERROR] Fehler beim Verbinden mit dem Broker");
    return;
  }
  // Konvertiere den String in ein C-String (char array)
  char msg[payload.length() + 1];
  payload.toCharArray(msg, payload.length() + 1);

  // Sende den Payload an das spezifizierte Thema
  if (client.publish(topic, msg)) {
    Serial.println("===========================================");
    Serial.println("[SUCC] Payload erfolgreich gesendet: \n" + payload);
    Serial.println("===========================================");
  } else {
    Serial.println("[ERROR] Fehler beim Senden des Payloads");
  }

  // Trenne die Verbindung zum Broker
  client.disconnect();
  Serial.println("[INFO] Verbindung zum Broker getrennt");
}

bool connectToMQTT() {
  Serial.print("Verbinde mit MQTT-Broker...");
  // Verwende connect mit Benutzername und Passwort
  if (client.connect("ESP32WeatherClient", mqtt_user, mqtt_password)) {
    Serial.println("verbunden");
    return true;
  } else {
    Serial.print("[ERROR] Verbindung fehlgeschlagen, rc=");
    Serial.println(client.state());
    return false;
  }
}

void blinkLed(int LED) {
  digitalWrite(LED,HIGH);
  delay(200);
  digitalWrite(LED,LOW);
  delay(1000);
}

void setup() {
  // Start serial communication
  Serial.begin(9600);
  // Initialize Status LEDs
  pinMode(LED_RED,OUTPUT);
  pinMode(LED_GRN,OUTPUT);
  // Setup for MQTT
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if(!systemInit()) {
    digitalWrite(LED_RED,HIGH);
    while (1);
  }
  blinkLed(LED_GRN);

  BmeData bmeData = readBME280Data();
  bool isRaining = readRainSensor();
  TimeData* timeDataPtr = getCurrentTime();
  blinkLed(LED_GRN);

  String jsonDataString = "";
  if(timeDataPtr != nullptr) {
    jsonDataString = createJsonPayload(*timeDataPtr, bmeData, isRaining);
    sendJsonPayload(jsonDataString);
    blinkLed(LED_GRN);
  } else {
    Serial.print("[ERROR] Fehler beim Lesen der Zeit");
    blinkLed(LED_RED);
  }

  systemDecon();
  sleepToFullSixth();
}

