// aws_functions.cpp
#include "aws.h"
#include "secrets.h"
#include "Arduino.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>


// make the inital message handler point to nothing
static MessageHandlerCallback messageHandlerCallback = nullptr;

void connectAWS(WiFiClientSecure& net, PubSubClient& client) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC.c_str());
  client.subscribe(AWS_IOT_PUBLIC_TOPIC.c_str());
  Serial.println(AWS_IOT_SUBSCRIBE_TOPIC.c_str());
  Serial.println(AWS_IOT_PUBLIC_TOPIC.c_str());
  Serial.println(AWS_IOT_PUBLIC_TOPIC.c_str());

  Serial.println("AWS IoT Connected!");
  String ip = WiFi.localIP().toString().c_str();
  StaticJsonDocument<200> doc;

  doc["type"] = "STATUS";
  doc["content"]["message"] = "CONNECTED";
  doc["content"]["device"]["name"] = THINGNAME;
  doc["content"]["device"]["ip"] = ip;
  doc["content"]["software"]["version"] = VERSION;
  doc["content"]["software"]["flavour"] = FLAVOUR;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);  // print to client

  client.publish(AWS_IOT_STATUS_TOPIC.c_str(), jsonBuffer);
  Serial.println("Published");
}

void messageHandler(char* topic, byte* payload, unsigned int length) {

  if (messageHandlerCallback != nullptr) {
    // Call the callback function and pass the parameters
    messageHandlerCallback(topic, payload, length);
  }
}

void setMessageHandlerCallback(MessageHandlerCallback callback) {
  // Set the callback function
  messageHandlerCallback = callback;
}
