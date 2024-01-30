#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoMqttClient.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Firebase credentials
#define API_KEY ""          // Your Firebase API key
#define DATABASE_URL ""     // Your Firebase database URL
#define USER_EMAIL ""       // Your Firebase user email
#define USER_PASSWORD ""    // Your Firebase user password

FirebaseData fbdo;         // Firebase data object
FirebaseAuth auth;          // Firebase authentication object
FirebaseConfig config;      // Firebase configuration object

int sensorPin = 27;         // Pin connected to the motion sensor

int hours = 0;              // Variable to store hours
int minutes = 0;            // Variable to store minutes

// For WiFi
char ssid[] = "";           // Your WiFi network SSID
char pass[] = "";           // Your WiFi network password

WiFiClient wifiClient;       // WiFi client
MqttClient mqttClient(wifiClient);  // MQTT client

const char broker[] = "172.20.10.2";     // MQTT broker IP address
int port = 1884;                        // MQTT broker port
const char* username = "iotproject";    // MQTT broker username
const char* password = "coen446";       // MQTT broker password

const char esp_publish_topic[] = "/MotionDetected";  // MQTT topic for motion detection

const long interval = 1000;             // Interval for sending MQTT messages
unsigned long previousMillis = 0;       // Variable to store the last time a message was sent

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
  pinMode(sensorPin, INPUT);
  Serial.begin(115200);
  while (!Serial) {
    delay(100);
  }

  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);
  Serial.println(username);
  mqttClient.setUsernamePassword(username, password);
  Serial.println(password);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1)
      ;
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  // Firebase required information for login
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize NTP
  timeClient.begin();
}

void loop() {
  // Update NTP time
  timeClient.update();
  timeClient.setTimeOffset(-18000);
  // Get current timestamp
  int currentHours = timeClient.getHours();
  int currentMinutes = timeClient.getMinutes();

  int motion_state = digitalRead(sensorPin);

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time a message was sent
    previousMillis = currentMillis;

    if (motion_state == 1) {
      Serial.println("Somebody is in this area!");
      mqttClient.beginMessage(esp_publish_topic);
      mqttClient.print(motion_state);
      mqttClient.endMessage();
      Serial.println(motion_state);
    } else if (motion_state == 0) {
      // do nothing when motion is equal to 0
      Serial.println("No one.");
    }
  }

  mqttClient.poll();
  if (mqttClient.available()) {
    String mqtt_topic_retrieved = mqttClient.messageTopic();
    if (mqtt_topic_retrieved == esp_publish_topic) {
      // Send the motion status to Firebase Realtime Database
      if (Firebase.RTDB.setInt(&fbdo, "motion_status", motion_state)) {
        Serial.println("Motion status sent to Firebase");
      } else {
        Serial.println("Failed to send motion status to Firebase");
        Serial.println("Error: " + fbdo.errorReason());
      }
      // Send timestamp to Firebase
      if (Firebase.RTDB.setInt(&fbdo, "timestamp/Hours", currentHours) && Firebase.RTDB.setInt(&fbdo, "timestamp/Minutes", currentMinutes)) {
        Serial.println("Timestamp sent to Firebase");
      } else {
        Serial.println("Failed to send timestamp to Firebase");
        Serial.println("Error: " + fbdo.errorReason());
      }
    }
  }
}
