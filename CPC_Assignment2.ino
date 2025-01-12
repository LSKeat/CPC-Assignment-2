#include <PubSubClient.h>
#include "DHT.h"
#include <WiFi.h>
#define DHTTYPE DHT11 // DHT11

int MinMoistureValue = 4095;
int MaxMoistureValue = 2060;
int MinMoisture = 0;
int MaxMoisture = 100;
int Moisture = 0;
const int SOIL_DRY_THRESHOLD = 30;

int MinDepthValue = 4095;
int MaxDepthValue = 1500;
int MinDepth = 0;
int MaxDepth = 100;
int depth = 0;

//define device id
const char* MoistureSensor = "";
const char* DepthSensor = "";
const char* DHTSensor = "";
const char* LedG = "";
const char* LedR= "";

// Used Pins
const int relayPin = 39; // Relay
const int soilPin = A4;  // Soil Sensor
const int depthPin = 5;   // Depth Sensor

const int ledPinR = 9;   // Red LED
const int ledPinY = 6;   // Yellow LED
const int ledPinG = 7;   // Green LED

const int dht11Pin = 42; // Digital pin connected to the DHT11 sensor

DHT dht(dht11Pin, DHTTYPE);

const char* WIFI_SSID = "cslab"; // Your WiFi SSID
const char* WIFI_PASSWORD = "aksesg31"; // Your WiFi password
const char* MQTT_SERVER = "34.68.20.78"; // Your VM instance public IP address
const char* MQTT_TOPIC = "iot"; // MQTT topic for subscription
const int MQTT_PORT = 1883; // Non-TLS communication port
char buffer[128] = ""; // Text buffer

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  } 

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
} 

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(ledPinY, OUTPUT);
  pinMode(soilPin, INPUT);
  pinMode(depthPin, INPUT);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
  client.setServer(MQTT_SERVER, MQTT_PORT); // Set up the MQTT client
}

int readSoilSensor() {
  int soilVal = analogRead(soilPin);
  Moisture = map(soilVal, MinMoistureValue, MaxMoistureValue, MinMoisture, MaxMoisture);
  return Moisture;
}

int readDepthSensor() {
  int depthVal = analogRead(depthPin);
  depth = map(depthVal, MinDepthValue, MaxDepthValue, MinDepth, MaxDepth);
  return depth;
}

void controlFan(bool state) {
  digitalWrite(relayPin, state ? HIGH : LOW);
  Serial.println(state ? "Fan is ON." : "Fan is OFF.");
}

void readDHTSensor(float &humidity, float &temperature) {
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from the DHT sensor");
    humidity = -1;
    temperature = -1;
  }
}

void reconnect() {
  while (!client.connected())
  { 
  Serial.println("Attempting MQTT connection...");
  
  if(client.connect("ESP32Client")) {
  Serial.println("Connected to MQTT server");
  }

  else { 
    Serial.print("Failed, rc=");
    Serial.print(client.state());
    Serial.println(" Retrying in 5 seconds...");
    delay(5000);
    } 
  } 
} 

void loop() {
  if(!client.connected()) {
    reconnect();
  } 
  client.loop();
  delay(5000);

  // Read sensor data
  int moisture = readSoilSensor();
  int depth = readDepthSensor();
  float h = dht.readHumidity();
  int t = dht.readTemperature();
  
  sprintf(buffer, "Soil moisture: %d, Water Level: %d, Humidity: %.2f%%, Temperature: %d degree Celsius", moisture, depth, h, t);
  Serial.println(buffer);
  client.publish(MQTT_TOPIC, buffer);
}