#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "DHT.h"
#include "aws.h"
#include <AsyncElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <stdlib.h>
#include "queue.h"

// DEVICE -> FAN THING
int PWM_CHANNEL = 0;     // ESP32 has 16 channels which can generate 16 independent waveforms
int PWM_FREQ = 500;      // Recall that Arduino Uno is ~490 Hz. Official ESP32 example uses 5,000Hz
int PWM_RESOLUTION = 8;  // We'll use same resolution as Uno (8 bits, 0-255) but ESP32 can go up to 16 bits
                         // The max duty cycle value based on PWM resolution (will be 255 if resolution is 8 bits)
int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1);
int FAN_OUTPUT_PIN = D6;
int DELAY_MS = 4;  // delay between fade increments

float voltage;


uint64_t device_startup_time;
uint64_t time_offset;
uint64_t arrival_time;
// define a web server for OTA updates
AsyncWebServer server(80);

// create the WiFi client and the MQTT client
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

BearSSL::X509List cert(AWS_CERT_CA);
BearSSL::X509List client_crt(AWS_CERT_CRT);
BearSSL::PrivateKey key(AWS_CERT_PRIVATE);

String run_id;       // this is where the run id will be stored
uint64_t timestamp;  // this is where the current timestamp tracked by the device will be stored
String status;       // this will store the device status (e.g CONNECTED, START)

// Any message from AWS IoT will enter this function and be dealt with accordingly
void updateVariables(char* topic, byte* payload, unsigned int length) {

  Serial.print("incoming: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);

  const char* messageString = doc["content"]["message"];
  const char* typeString = doc["type"];
  const char* timestampString = doc["content"]["timestamp"];
  const char* runString = doc["content"]["run_id"];

  if (strcmp(typeString, "STATUS") == 0) {
    if (strcmp(messageString, "GOOD_MORNING") == 0) {
      //set timestamp
      status = String(messageString);
      timestamp = strtoull(timestampString, NULL, 10) * 1000;
      Serial.println(timestamp);
        arrival_time = millis();
        Serial.println(arrival_time);

  // Calculate the offset
  time_offset = timestamp - (arrival_time - device_startup_time);
        Serial.println(time_offset);

      publishStatus();
    }
  } else if (strcmp(typeString, "ACTION") == 0) {
    if ((strcmp(messageString, "START") == 0) || (strcmp(messageString, "WAIT") == 0)) {
      run_id = runString;
      status = messageString;
      publishStatus();
    } else if (strcmp(messageString, "STOP") == 0) {
      status = messageString;
      run_id = "";
      publishStatus();
    } else if (strcmp(messageString, "FAN_CHANGE") == 0) {
      status = messageString;
      const char* fan_speed = doc["content"]["new_speed"];
      int new_speed = atoi(fan_speed);
  Serial.println("Received fan speed...");

      fan_change(new_speed);
      publishStatus();
    }
  } else {
    Serial.print("Irrelevant message");
  }
}

void fan_change(int pwmValue) {
  analogWrite(FAN_OUTPUT_PIN, pwmValue);
  voltage = pwmValue / 255 * 12;
  Serial.println(pwmValue);
  Serial.println("Updating fan speed...");
}

void setup() {
  Serial.begin(115200);
  device_startup_time = millis();


  connectAWS(net, client, cert, client_crt, key);
  status = "CONNECTED";

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  setMessageHandlerCallback(updateVariables);

  // setup the Server and allow OTA Updates
  server.on("/", [](AsyncWebServerRequest *request) {
request->send(200, "text/plain", "This is ElegantOTA Demo!");
  });
  AsyncElegantOTA.begin(&server);
  server.begin();
}

void publishSpeed(){
  StaticJsonDocument<200> doc;

  //Create a JSON Doc with the data to push to AWS
  doc["type"] = "DATA";
  doc["content"]["timestamp"] = (time_offset + (millis() - device_startup_time))/1000;
  doc["content"]["value"] = voltage;
  doc["content"]["device"] = THINGNAME;
  doc["content"]["run_id"] = run_id;


  char jsonBuffer[512];
  //Turn the JSON into a char array
  serializeJson(doc, jsonBuffer);  // print to client

  // Push this data to AWS
  client.publish(AWS_IOT_PUBLISH_TOPIC.c_str(), jsonBuffer);
  Serial.println("Published");

}

void publishStatus() {
  StaticJsonDocument<200> doc;
  String ip = WiFi.localIP().toString().c_str();
  //Create a JSON Doc with the data to push to AWS
  doc["type"] = "STATUS";
  doc["content"]["message"] = status;
  doc["content"]["device"]["name"] = THINGNAME;
  doc["content"]["device"]["ip"] = ip;

  char jsonBuffer[512];
  //Turn the JSON into a char array
  serializeJson(doc, jsonBuffer);  // print to client

  // Push this data to AWS
  client.publish(AWS_IOT_STATUS_TOPIC.c_str(), jsonBuffer);
  Serial.println("Published");
}



void loop() {
  // server.handleClient();
    AsyncElegantOTA.loop();
    client.loop();
      if ((status == "START") || (status == "WAIT") ) {
        //And publish the data to AWS
        publishSpeed();
        Serial.println("Publishing...");
      }

    delay(3000);
}