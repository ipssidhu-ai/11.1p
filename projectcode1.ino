#include <Wire.h>
#include <MPU6050.h>
#include <WiFi.h>     // or use <WiFi.h> if ESP32
#include <PubSubClient.h>

MPU6050 mpu;

// --- WiFi & MQTT Setup ---
const char* ssid = "ishat";
const char* password = "ishat000";
const char* mqtt_server = "192.168.137.26";  // Raspberry Pi IP

WiFiClient espClient;
PubSubClient client(espClient);

int vibrationPin = 2;  // SW-420 signal pin
float magnitude = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266_EarthquakeSensor")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  pinMode(vibrationPin, INPUT);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // --- Read MPU6050 Acceleration Magnitude ---
  int16_t ax, ay, az;
  mpu.getAcceleration(&ax, &ay, &az);

  float a = sqrt(ax * ax + ay * ay + az * az) / 16384.0;
  magnitude = abs(a - 1.0);  // remove gravity component

  // --- Read Vibration Sensor ---
  int vibration = digitalRead(vibrationPin);

  // --- Combine data ---
  char msg[50];
  snprintf(msg, 50, "%.2f,%d", magnitude, vibration);

  client.publish("earthquake/magnitude", msg);
  Serial.print("Published -> ");
  Serial.println(msg);

  delay(1000);
}

