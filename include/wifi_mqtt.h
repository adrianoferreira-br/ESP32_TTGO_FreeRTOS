#ifndef WIFI_MQTT_H_
#define WIFI_MQTT_H_

// Bibliotecas necessárias
#include <WiFi.h>
#include <PubSubClient.h>
#include "constants.h"

// Declaração dos objetos globais
extern WiFiClient espClient;
extern PubSubClient client;

// Wifi
void setup_wifi(void);
void loop_wifi(void);

//mqtt
void setup_mqtt(void);
void reconnect(void);
void loop_mqqt(void);

//partitions
void show_partitions();
void show_ota_info();

//Ota
void setup_ota();
void loop_ota();  

//NTP
void setup_ntp(void);

//void mqtt_send_data(String, String);
bool mqtt_send_data(const char* nome_equipamento, const char* horario, long id_message_batch, const char* observacao);
bool mqtt_send_info();
bool mqtt_send_settings();
bool mqtt_send_settings_confirmation();
bool mqtt_send_settings_device();
bool mqtt_send_settings_equip();
bool mqtt_send_settings_client();



#endif // WIFI_MQTT_H_

