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
#include "HX711.h"
#include "soc/rtc.h"

//DEVICE SPECIFIC - These next 5 lines are device specific
float t;

const int LOADCELL_DOUT_PIN = 15;
const int LOADCELL_SCK_PIN = 4;

HX711 scale;

// define a web server for OTA updates
WebServer server(80);

// create the WiFi client and the MQTT client
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// Create the task handles which allows us to suspend/resume tasks
TaskHandle_t TimerTask;
TaskHandle_t GetTask;
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
    }

  } else {
    Serial.print("Irrelevant message");
  }
}

void setup() {
  Serial.begin(115200);
  rtc_cpu_freq_config_t config;
  rtc_clk_cpu_freq_get_config(&config);
  rtc_clk_cpu_freq_to_config(RTC_CPU_FREQ_80M, &config);
  rtc_clk_cpu_freq_set_config_fast(&config);
  connectAWS(net, client);
  status = "CONNECTED";

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  setMessageHandlerCallback(updateVariables);


  // device specific -> start the HX711 Scale

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(SCALE);
  scale.tare();

  // setup the Server and allow OTA Updates
  server.on("/", []() {
    server.send(200, "text/plain", "Hi! This is ElegantOTA Demo.");
  });
  ElegantOTA.begin(&server);
  server.begin();

  // Start the FreeRTOS Tasks for timestamping, getting data, and pushing data
  xTaskCreate(TimestampTaskCode, "TimerTask", 10000, NULL, 2, &TimerTask);
  delay(500);

  xTaskCreate(GetDataCode, "GetTask", 10000, NULL, 2, &GetTask);
  delay(500);

  xTaskCreate(PushDataCode, "PushTask", 10000, NULL, 1, &PushTask);
  delay(500);

  // Start a FreeRTOS task for getting OTA Updates
  //this task will be suspended once the GOOD_MORNING status is received
  xTaskCreate(OTACode, "OTATask", 10000, NULL, 0, &OTATask);
  delay(500);
}

void publishData(QueueData data) {
  StaticJsonDocument<200> doc;

  //Create a JSON Doc with the data to push to AWS
  doc["type"] = data.type;
  doc["content"]["timestamp"] = data.timestamp;
  doc["content"]["value"] = data.value;
  doc["content"]["device"] = data.device;
  doc["content"]["run_id"] = data.run_id;
  doc["content"]["counter"] = data.counter;

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

void PushDataCode(void* parameter) {
  Serial.print("Pushing data on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    // dataQueue.printQueue();
    // If the device is in START (e.g a run is going on)
    if ((status == "START") || (status == "WAIT")) {
      //Dequeue from the front
      QueueData dequeuedData = dataQueue.dequeue();
      Serial.println("Removing Data ..");
      //And publish the data to AWS
      publishData(dequeuedData);
      Serial.println("Publishing...");
    }

    if (status == "STOP") {
      while (dataQueue.isEmpty() != true) {
        QueueData dequeuedData = dataQueue.dequeue();
        Serial.println("Removing Data after STOP..");
        //And publish the data to AWS
        publishData(dequeuedData);
        Serial.println("Publishing...");
      }
    }
    client.loop();
    delay(1000);
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

void GetDataCode(void* parameter) {
  Serial.print("Getting data on core ");
  Serial.println(xPortGetCoreID());

  for (;;) {
    Serial.println("Getting Data ..");

    //DEVICE SPECIFIC -> get the weight
    t = scale.get_units(6);

    // Create an instance of the QueueData using the gathered data
    QueueData data1 = { "DATA", timestamp, t, THINGNAME, run_id, counter };

    if (status != "CONNECTED" && status != "GOOD_MORNING" && status != "STOP") {
      //If in a run, queue the data
      dataQueue.enqueue(data1);
      //Increment the counter
      counter = counter + 1;
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