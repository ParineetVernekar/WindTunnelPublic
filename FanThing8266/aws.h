// aws_functions.h

#ifndef AWS_H
#define AWS_H
#include <ESP8266WiFi.h>

#include <WiFiClientSecure.h>
#include <PubSubClient.h>

#define AWS_IOT_PUBLISH_TOPIC ("windtunnel/" + std::string(THINGNAME) + "/pub")
#define AWS_IOT_SUBSCRIBE_TOPIC ("windtunnel/" + std::string(THINGNAME) + "/sub")

#define AWS_IOT_PUBLIC_TOPIC (std::string("windtunnel/public"))
#define AWS_IOT_STATUS_TOPIC (std::string("windtunnel/status"))

typedef void (*MessageHandlerCallback)(char*, byte*, unsigned int);


void NTPConnect(void);
void connectAWS(WiFiClientSecure& net, PubSubClient& client, BearSSL::X509List &cert, BearSSL::X509List &client_crt, BearSSL::PrivateKey &key) ;
void messageHandler(char* topic, byte* payload, unsigned int length);
void setMessageHandlerCallback(MessageHandlerCallback callback);

#endif
