/*
 * DOOR_SENSOR.H
 * 
 * Monitoramento de sensores de porta (door sensors) com envio MQTT.
 * 
 * Funcionalidade:
 *   - Monitora até 2 sensores de porta (DOOR_SENSOR_1 e DOOR_SENSOR_2)
 *   - Leitura a cada 1 segundo
 *   - Pull-up interno ativado (HIGH quando aberto, LOW quando fechado)
 *   - Envio via MQTT no formato JSON padronizado
 * 
 * Hardware:
 *   - Pinos configurados com INPUT_PULLUP
 *   - Sensor fechado: GND conectado ao pino (leitura LOW = 0)
 *   - Sensor aberto: pino flutuante com pull-up (leitura HIGH = 1)
 * 
 * Data: 2026-01-15
 */

#ifndef DOOR_SENSOR_H
#define DOOR_SENSOR_H

#include <Arduino.h>
#include "main.h"

// Estrutura para armazenar estado do sensor de porta
struct DoorSensorData {
    int sensor_id;           // ID do sensor (1 ou 2)
    int value;               // Valor atual (0 = fechado, 1 = aberto)
    int previous_value;      // Valor anterior para detectar mudanças
    unsigned long message_id; // ID da mensagem incremental
    unsigned long last_send_time; // Timestamp do último envio
    bool changed;            // Flag indicando mudança de estado
};

// Variáveis globais para os sensores
extern DoorSensorData door_sensor_1;
extern DoorSensorData door_sensor_2;

// Timer para leitura periódica
extern unsigned long last_door_check_time;
extern const unsigned long DOOR_CHECK_INTERVAL; // 1 segundo

// Funções principais
void door_sensor_setup();
void door_sensor_loop();

// Funções auxiliares
void door_sensor_read(DoorSensorData &sensor, int pin);
void door_sensor_send_mqtt(DoorSensorData &sensor);
String door_sensor_create_json(DoorSensorData &sensor);

#endif // DOOR_SENSOR_H
