#include <pgmspace.h>
 
#define SECRET
#define THINGNAME "FanThing"                         //change this
#define VERSION "1.1.0"
#define FLAVOUR "Fan"

const char WIFI_SSID[] = "WIFI_SSID_HERE";               //change this
const char WIFI_PASSWORD[] = "WIFI_PASSWORD_HERE";           //change this

const char AWS_IOT_ENDPOINT[] = "AWS_IOT_ENDPOINT_HERE";       //change this
 
// Amazon Root CA 1
//Copy the AWS Root CA 1 certificate here
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
)EOF";
 
// Device Certificate                                               //change this
// Copy the AWS IoT Thing Device Certificate here
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----

-----END CERTIFICATE-----
 
 
)KEY";
 
// Device Private Key                                               //change this
// Copy the AWS IoT Thing Device Private Key into here
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----

-----END RSA PRIVATE KEY-----
 
 
)KEY";