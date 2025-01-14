#ifndef WIFI_MQTT_H
#define WIFI_MQTT_H

// Hardware-specific library
#include <WiFi.h>
#include <PubSubClient.h>
#include "main.h"


void reconnect(void);
void callback(char*, byte*, unsigned int);
void setup_wifi(void);
void loop_mqqt(void);


#endif // WIFI_MQTT_H