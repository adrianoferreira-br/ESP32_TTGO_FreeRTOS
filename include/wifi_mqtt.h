#ifndef WIFI_MQTT_H_
#define WIFI_MQTT_H_

// Hardware-specific library
#include <WiFi.h>
#include <PubSubClient.h>
#include "constants.h"
#include "main.h"


// Wifi
void setup_wifi(void);
void loop_wifi(void);

//mqtt
void setup_mqtt(void);
void reconnect(void);
void callback(char*, byte*, unsigned int);
void loop_mqqt(void);

//partitions
void show_partitions();

//Ota
void setup_ota();
void loop_ota();  

//NTP
void setup_ntp(void);

//void mqtt_send_data(String, String);
bool mqtt_send_data(const char* nome_equipamento, const char* horario, long id_leitura, const char* observacao);
bool mqtt_send_readings();
bool mqtt_send_info();



#endif // WIFI_MQTT_H_

