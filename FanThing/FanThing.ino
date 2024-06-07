#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include "DHT.h"
#include "aws.h"
#include <ElegantOTA.h>
#include <WebServer.h>
#include <stdlib.h>
#include "queue.h"

// DEVICE -> FAN THING
int PWM_CHANNEL = 0;     // ESP32 has 16 channels which can generate 16 independent waveforms
int PWM_FREQ = 500;      // Recall that Arduino Uno is ~490 Hz. Official ESP32 example uses 5,000Hz
int PWM_RESOLUTION = 8;  // We'll use same resolution as Uno (8 bits, 0-255) but ESP32 can go up to 16 bits
                         // The max duty cycle value based on PWM resolution (will be 255 if resolution is 8 bits)
int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1);
int FAN_OUTPUT_PIN = 18;
int DELAY_MS = 4;  // delay between fade increments

float voltage;

// define a web server for OTA updates
WebServer server(80);

// create the WiFi client and the MQTT client
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// Create the task handles which allows us to suspend/resume tasks
TaskHandle_t TimerTask;
TaskHandle_t PushTask;
TaskHandle_t OTATask;

String run_id;       // this is where the run id will be stored
uint64_t timestamp;  // this is where the current timestamp tracked by the device will be stored
String status;       // this will store the device status (e.g CONNECTED, START)
int counter = 0;     // this will act as a backup to the timestamp

// Define a queue to store data objects
Queue dataQueue(40);

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
      //set timestamp and start counter
      status = String(messageString);
      timestamp = strtoull(timestampString, NULL, 10);
      //Suspend the OTA Task
      vTaskSuspend(OTATask);
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
      vTaskResume(OTATask);
    } else if (strcmp(messageString, "FAN_CHANGE") == 0) {
      status = messageString;
      const char* fan_speed = doc["content"]["new_speed"];
      int new_speed = atoi(fan_speed);

      fan_change(new_speed);
      publishStatus();
    }
  } else {
    Serial.print("Irrelevant message");
  }
}

void fan_change(int pwmValue) {
  ledcWrite(PWM_CHANNEL, pwmValue);
  voltage = pwmValue / 255 * 12;
}

void setup() {
  Serial.begin(115200);
  connectAWS(net, client);
  status = "CONNECTED";

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  setMessageHandlerCallback(updateVariables);

  // setup the Server and allow OTA Updates
  server.on("/", []() {
    server.send(200, "text/plain", "Hi! This is ElegantOTA Demo.");
  });
  ElegantOTA.begin(&server);
  server.begin();

  // Start the FreeRTOS Tasks for timestamping, getting data, and pushing data
  xTaskCreate(TimestampTaskCode, "TimerTask", 10000, NULL, 2, &TimerTask);
  delay(500);

  xTaskCreate(PushDataCode, "PushTask", 10000, NULL, 1, &PushTask);
  delay(500);

  // Start a FreeRTOS task for getting OTA Updates
  //this task will be suspended once the GOOD_MORNING status is received
  xTaskCreate(OTACode, "OTATask", 10000, NULL, 0, &OTATask);
  delay(500);

  // device specific -> start the temperature sensor

  //device specific -> setup the fan voltage readings
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  // ledcAttachPin(uint8_t pin, uint8_t channel);
  ledcAttachPin(FAN_OUTPUT_PIN, PWM_CHANNEL);
}

void publishSpeed(){
  StaticJsonDocument<200> doc;

  //Create a JSON Doc with the data to push to AWS
  doc["type"] = "DATA";
  doc["content"]["timestamp"] = timestamp;
  doc["content"]["value"] = voltage;
  doc["content"]["device"] = THINGNAME;
  doc["content"]["run_id"] = run_id;
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

void PushDataCode(void* parameter) {
  Serial.print("Pushing data on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    publishSpeed();

    client.loop();
    delay(3000);
  }
}

void TimestampTaskCode(void* parameter) {
  Serial.print("Timestamp task is running on core ");
  Serial.println(xPortGetCoreID());
  for (;;) {
    if (status != "CONNECTED") {
      // Increment the timestamp as long as it has been received from AWS first
      timestamp = timestamp + 1;
      Serial.println(timestamp);
    } else {
      timestamp = 0;
    }
    delay(1000);
  }
}

void OTACode(void* parameter) {
  Serial.println("Setting up OTA Task..");
  for (;;) {
    //Deal with OTA Updates when in the CONNECTED state
    // will be suspended during runs
    server.handleClient();
    ElegantOTA.loop();
    Serial.println("OTA Task...");
    delay(500);
  }
}

void loop() {
}