#include <Arduino.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <ArduinoMqttClient.h>
#include "KasaSmartPlug.h"

//For WiFi
char ssid[] = "";        // your network SSID (name)
char pass[] = "";

int hours = 0;
int minutes = 0;

KASAUtil kasaUtil;
KASASmartPlug *testPlug;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "172.20.10.2";
int        port     = 1884;
const char* username = "iotproject";
const char* password = "coen446";

int led = 27;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup()
{
  int found;
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  while (!Serial)
  {
    delay(100);
  }

  Serial.println();
  Serial.println("******************************************************");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
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

  if (!mqttClient.connect(broker, port))
  {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1)
      ;
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  mqttClient.subscribe("/MotionDetected");

  found = kasaUtil.ScanDevices();
  Serial.printf("\r\n Found device = %d", found);
  
  //Setup for smart Kasa plug
  // Print out devices name and ip address found.
  for (int i = 0; i < found; i++)
  {
    KASASmartPlug *p = kasaUtil.GetSmartPlugByIndex(i);
    if (p != NULL)
    {
      Serial.printf("\r\n %d. %s IP: %s Relay: %d", i, p->alias, p->ip_address, p->state);
    }
  }
  //Purpose of this snippet is to check if plug is working or not
  testPlug = kasaUtil.GetSmartPlug("Coen446");
  if (testPlug != NULL)
  {
    testPlug->SetRelayState(1); //turn on
    testPlug->QueryInfo();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    Serial.printf("\r\nKeep the plug off");
    testPlug->SetRelayState(0); //turn off
    testPlug->QueryInfo();
  }
}

void loop()
{
  // Update NTP time
  timeClient.update();
  timeClient.setTimeOffset(-18000);
  // Get current timestamp
  int currentHours = timeClient.getHours();
  int currentMinutes = timeClient.getMinutes();
  
  // check for incoming messages
  int messageSize = mqttClient.parseMessage();
  static bool plugTurnedOn = false;

  if (messageSize) {
    // we received a message, print out the topic and contents
    Serial.print("Received a message with topic '");
    Serial.print(mqttClient.messageTopic());
    Serial.print("', length ");
    Serial.print(messageSize);
    Serial.println(" bytes:");

    // use the Stream interface to print the contents
    while (mqttClient.available()) {

      int incoming_broker = mqttClient.read() - '0';
      Serial.println("incoming from broker ");
      Serial.println(incoming_broker);

      Serial.println("hours minutes");
      Serial.println(currentHours);
      Serial.println(currentMinutes);

      // Check if it's between 8:00 AM and 8:30 AM, motion status is 1, and the plug hasn't been turned on yet
      while ((currentHours == 8 && (currentMinutes >= 0 && currentMinutes <= 30)) && incoming_broker == 1) {
        if (plugTurnedOn == false) {
          testPlug->SetRelayState(1); // turn on plug
          Serial.println("Motion detected between 8:00 AM and 8:30 PM - Turning on plug");
          plugTurnedOn = true; // mark that the plug has been turned on
          digitalWrite(led, incoming_broker);
          delay(20000); // keep the plug on for 20 seconds
        }
        else if (plugTurnedOn = true) {
          digitalWrite(led, 0);
          testPlug->SetRelayState(0); // turn on plug
        }
      }
    }
  }
}
