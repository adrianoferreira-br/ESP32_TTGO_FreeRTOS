/*
 * DOOR_SENSOR.CPP
 * 
 * Implementação do monitoramento de sensores de porta com envio MQTT.
 * 
 * Funcionalidade:
 *   - Monitora sensores de porta a cada 1 segundo
 *   - Detecta mudanças de estado (aberto/fechado)
 *   - Envia dados via MQTT no formato JSON padronizado
 * 
 * Data: 2026-01-15
 */

#include "door_sensor.h"
#include "wifi_mqtt.h"
#include "topicos.h"
#include "constants.h"
#include "display.h"
#include <ArduinoJson.h>
#include <WiFi.h>

// Variáveis globais dos sensores
DoorSensorData door_sensor_1 = {1, 1, 1, 0, 0, false};
DoorSensorData door_sensor_2 = {2, 1, 1, 0, 0, false};

// Timer para leitura periódica
unsigned long last_door_check_time = 0;
const unsigned long DOOR_CHECK_INTERVAL = 1000; // 1 segundo

/**
 * Inicializa os sensores de porta
 * Configura os pinos como entrada com pull-up interno
 */
void door_sensor_setup() {
  Serial.println("\n====================================");
  Serial.println("Inicializando sensores de porta...");
  Serial.println("====================================");
  
  // Configurar pinos como entrada com pull-up
  pinMode(DOOR_SENSOR_1, INPUT_PULLUP);
  pinMode(DOOR_SENSOR_2, INPUT_PULLUP);
  
  // Leitura inicial
  door_sensor_1.value = digitalRead(DOOR_SENSOR_1);
  door_sensor_1.previous_value = door_sensor_1.value;
  
  door_sensor_2.value = digitalRead(DOOR_SENSOR_2);
  door_sensor_2.previous_value = door_sensor_2.value;
  
  Serial.printf("  Door Sensor 1: GPIO %d - Estado inicial: %s\n", 
                DOOR_SENSOR_1, door_sensor_1.value ? "ABERTO" : "FECHADO");
  Serial.printf("  Door Sensor 2: GPIO %d - Estado inicial: %s\n", 
                DOOR_SENSOR_2, door_sensor_2.value ? "ABERTO" : "FECHADO");
  Serial.println("  Intervalo de leitura: 1 segundo");
  Serial.println("====================================\n");
  
  // Atualiza display com estado inicial
  show_door_sensor(1, door_sensor_1.value);
  show_door_sensor(2, door_sensor_2.value);
  
  last_door_check_time = millis();
}

/**
 * Loop principal dos sensores de porta
 * Chama a cada ciclo do loop, mas só executa a cada 1 segundo
 */
void door_sensor_loop() {
  unsigned long current_time = millis();
  
  // Verifica se passou 1 segundo desde a última leitura
  if (current_time - last_door_check_time >= DOOR_CHECK_INTERVAL) {
    last_door_check_time = current_time;
    
    // Ler e processar ambos os sensores
    door_sensor_read(door_sensor_1, DOOR_SENSOR_1);
    door_sensor_read(door_sensor_2, DOOR_SENSOR_2);
    
    // Enviar dados via MQTT apenas quando houver mudança de estado
    if (door_sensor_1.changed) {
      door_sensor_send_mqtt(door_sensor_1);
      door_sensor_1.changed = false;
      door_sensor_1.last_send_time = current_time;
    }
    
    if (door_sensor_2.changed) {
      door_sensor_send_mqtt(door_sensor_2);
      door_sensor_2.changed = false;
      door_sensor_2.last_send_time = current_time;
    }
  }
}

/**
 * Lê o estado do sensor de porta
 * 
 * @param sensor Referência para a estrutura do sensor
 * @param pin Pino GPIO a ser lido
 */
void door_sensor_read(DoorSensorData &sensor, int pin) {
  // Ler valor atual do pino
  int current_value = digitalRead(pin);
  
  // Atualizar estrutura do sensor
  sensor.previous_value = sensor.value;
  sensor.value = current_value;
  
  // Detectar mudança de estado
  if (sensor.value != sensor.previous_value) {
    sensor.changed = true;
    sensor.message_id++;
    
    Serial.printf("🚪 Door Sensor %d: Estado mudou para %s (GPIO %d)\n", 
                  sensor.sensor_id, 
                  sensor.value ? "ABERTO" : "FECHADO",
                  pin);
    
    // Atualiza display quando há mudança
    show_door_sensor(sensor.sensor_id, sensor.value);
  }
}

/**
 * Envia dados do sensor via MQTT
 * 
 * @param sensor Referência para a estrutura do sensor
 */
void door_sensor_send_mqtt(DoorSensorData &sensor) {
  // Verificar conexão WiFi e MQTT
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado - não enviando door sensor");
    return;
  }
  
  if (!client.connected()) {
    Serial.println("⚠️ MQTT desconectado - não enviando door sensor");
    return;
  }
  
  // Criar JSON
  String json_payload = door_sensor_create_json(sensor);
  
  // Publicar no tópico MQTT
  char topic[128];
  snprintf(topic, sizeof(topic), "%s", topico);
  
  bool success = client.publish(topic, json_payload.c_str());
  
  if (success) {
    Serial.printf("✅ Door Sensor %d enviado via MQTT [%s]\n", sensor.sensor_id, topic);
    Serial.printf("   Estado: %s | Message ID: %lu\n", 
                  sensor.value ? "ABERTO" : "FECHADO", 
                  sensor.message_id);
  } else {
    Serial.printf("❌ Falha ao enviar Door Sensor %d via MQTT\n", sensor.sensor_id);
  }
}

/**
 * Cria o JSON no formato especificado
 * 
 * @param sensor Referência para a estrutura do sensor
 * @return String com o JSON formatado
 */
String door_sensor_create_json(DoorSensorData &sensor) {
  StaticJsonDocument<512> doc;
  
  // Campos principais
  doc["table"] = "device_readings";
  doc["device_id"] = DISPOSITIVO_ID;
  doc["timestamp"] = time(nullptr); // Unix timestamp
  doc["wifi_rssi_dbm"] = WiFi.RSSI();
  
  // Array de readings
  JsonArray readings = doc.createNestedArray("readings");
  JsonObject reading = readings.createNestedObject();
  
  reading["metric_name"] = "door";
  reading["value"] = sensor.value;
  reading["sensor_id"] = sensor.sensor_id; // Adicionar sensor_id para diferenciar
  reading["interval"] = 1; // Intervalo em segundos
  reading["message_id"] = sensor.message_id;
  
  // Serializar para string
  String output;
  serializeJson(doc, output);
  
  return output;
}
