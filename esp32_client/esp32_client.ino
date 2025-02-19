#include <WiFi.h>
#include <HTTPClient.h>
#include <PubSubClient.h>
#include <DHT.h>

// WiFi and MQTT credentials
const char* ssid = "QCom-2025";
const char* pswrd = "ucsc@123";
const char* mqtt_server = "10.0.0.53";

// ----- Sensor Pin Definitions -----
// Ultrasonic sensor pins
const int trigPin = 19;  // Trigger pin for ultrasonic sensor
const int echoPin = 21;  // Echo pin for ultrasonic sensor
const int ledRed = 32;
const int ledYellow = 35;
const int ledGreen = 34;

// DHT sensor configuration (use DHT22 or change to DHT11 as needed)
#define DHTPIN 4       // Data pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// ----- MQTT Client Setup -----
WiFiClient espClient;
PubSubClient client(espClient);

// Connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, pswrd);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

// Reconnect to MQTT broker if connection is lost
void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32SmartBinClient")) {
      Serial.println("connected");
      // Subscribe to a command topic if needed in the future
      client.subscribe("esp/cmd");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Callback function for incoming MQTT messages (for future commands)
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  Serial.println(message);
  // You can add code here to handle commands from the broker
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Initialize ultrasonic sensor pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(ledRed, OUTPUT);
  pinMode(ledYellow, OUTPUT);
  pinMode(ledGreen, OUTPUT);

  // Initialize the DHT sensor
  dht.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // ----- Ultrasonic Sensor Reading -----
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  // Calculate distance in centimeters (speed of sound ~0.034 cm/µs)
  float distance = (duration * 0.034) / 2;
  if (distance < 10) {
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledYellow, LOW);
    digitalWrite(ledGreen, LOW);
  } else if (distance < 60) {
    digitalWrite(ledYellow, HIGH);
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, LOW);
  } else {
    digitalWrite(ledGreen, HIGH);
    digitalWrite(ledRed, LOW);
    digitalWrite(ledYellow, LOW);
  }

  // ----- DHT Sensor Reading -----
  float temperature = dht.readTemperature(); // in Celsius
  float humidity = dht.readHumidity();
  
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  // ----- Prepare and Publish Payload -----
  // Format: "distance,temperature,humidity"
  String payload = String(distance, 1) + "," +
                   String(temperature, 1) + "," +
                   String(humidity, 1);
  
  client.publish("home/sensors/data", payload.c_str());
  Serial.print("Sensor data sent: ");
  Serial.println(payload);

  delay(5000); // Publish every 5 seconds
}
