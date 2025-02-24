#ifndef WIFI_MQTT_H_
#define WIFI_MQTT_H_

// Hardware-specific library
#include <WiFi.h>
#include <PubSubClient.h>
#include "constants.h"
#include "main.h"


void setup_mqtt(void);
void reconnect(void);
void callback(char*, byte*, unsigned int);
void setup_wifi(void);
void loop_mqqt(void);
void loop_wifi(void);
void mqtt_send_data(float, float, float, float, float, int, float);


#endif // WIFI_MQTT_H_