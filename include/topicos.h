#ifndef TOPICOS_H_
#define TOPICOS_H_

// Bibliotecas necessárias
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "constants.h"

// Declaração dos objetos externos (definidos em wifi_mqtt.cpp)
extern WiFiClient espClient;
extern PubSubClient client;

// Declaração das funções externas (definidas em wifi_mqtt.cpp)
extern void setup_wifi(void);
extern void setup_mqtt(void);

// ============ DECLARAÇÕES DE VARIÁVEIS GLOBAIS ============
extern bool enabled_send_level_readings;        // Habilita envio de leituras de nível
extern bool enabled_send_temperature_readings;  // Habilita envio de leituras de temperatura
extern bool enabled_send_ticket_readings;       // Habilita envio de leituras de ticket
extern bool enabled_send_humidity_readings;     // Habilita envio de leituras de umidade



//mqtt - Callback e utilitários
void callback(char*, byte*, unsigned int);
void reconnect(void);
String getMqttErrorMessage(int errorCode);

//mqtt - Funções de envio de dados
bool mqtt_send_data(const char* nome_equipamento, const char* horario, long id_leitura, const char* observacao);
bool mqtt_send_readings();
bool mqtt_send_info();
bool mqtt_send_settings();
bool mqtt_send_settings_confirmation();
bool mqtt_send_settings_device();
bool mqtt_send_settings_equip();
bool mqtt_send_settings_client();


bool mqtt_send_datas_readings();



#endif // TOPICOS_H_
