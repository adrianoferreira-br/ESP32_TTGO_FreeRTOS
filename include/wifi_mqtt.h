#ifndef WIFI_MQTT_H_
#define WIFI_MQTT_H_

// Hardware-specific library
#include <WiFi.h>
#include <PubSubClient.h>
#include "constants.h"
#include "main.h"
//#include <WebServer.h>

// Wifi
void setup_wifi(void);
void loop_wifi(void);

//mqtt
void setup_mqtt(void);
void reconnect(void);
void callback(char*, byte*, unsigned int);
void loop_mqqt(void);
void mqtt_send_data(String, String);

//partitions
void show_partitions();

//Ota
void config_ota();
void loop_ota();  




#endif // WIFI_MQTT_H_