/*
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include "constants.h"  // DEVE SER O PRIMEIRO INCLUDE para definições funcionarem
#include "wifi_mqtt.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "state.h"
#include "esp_task_wdt.h"
#include "esp_ota_ops.h"
#include "dht_xx.h"
#include "jsn_sr04t.h"
#include "voltage_bat.h"
#include "mlx90614.h"
#include "ds18b20.h"
#include "mem_flash.h"
#include "web_server.h"
#include <Preferences.h>
#include "wifi_mqtt.h"


#ifdef LILYGO_T_DISPLAY_S3
  // ESP32-S3: GPIO 26/27 podem ter problemas, usar pinos seguros      ToDo: Testar outros pinos
  #define ULTRASONIC_TRIG 1  // GPIO 1 (TX) - disponível no S3
  #define ULTRASONIC_ECHO 2  // GPIO 2 - disponível no S3
#else
  #define ULTRASONIC_TRIG 26
  #define ULTRASONIC_ECHO 27
#endif

#ifdef LILYGO_T_DISPLAY_S3
  // ESP32-S3: Usar GPIO seguro (21 é válido no S3)
  #define DHTPIN 43
#else
  #define DHTPIN 21
#endif

#ifdef LILYGO_T_DISPLAY_S3
  // GPIO 35 não existe no ESP32-S3! Usar GPIO alternativo
  #define VBAT_PIN 4  // GPIO4 para leitura de bateria no S3  ToDo: Verificar se esse pino é adequado
#else
  #define VBAT_PIN 35  // GPIO35 para ESP32 original
#endif


void vTask1(void *pvParameters);  //
void vTask2(void *pvParameters);  //
void define_hardware();


extern String ip;



#endif  //MAIN_H_