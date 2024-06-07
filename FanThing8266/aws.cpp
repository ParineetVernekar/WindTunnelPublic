// aws_functions.cpp
#include "aws.h"
#include "secrets.h"
#include "Arduino.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <time.h>


// make the inital message handler point to nothing
static MessageHandlerCallback messageHandlerCallback = nullptr;

time_t now;
time_t nowish = 1510592825;
 

void NTPConnect(void)
{
  Serial.print("Setting time using SNTP");
  configTime(0 * 3600, 0 * 3600, "pool.ntp.org", "time.nist.gov");
  now = time(nullptr);
  while (now < nowish)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void connectAWS(WiFiClientSecure& net, PubSubClient& client, BearSSL::X509List &cert, BearSSL::X509List &client_crt, BearSSL::PrivateKey &key ) {


  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("--");
  }
  NTPConnect();

  // Configure WiFiClientSecure to use the AWS IoT device credentials

  net.setTrustAnchors(&cert);
  net.setClientRSACert(&client_crt, &key);

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
