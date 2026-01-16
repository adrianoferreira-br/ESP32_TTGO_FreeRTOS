/*
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include "constants.h"  // DEVE SER O PRIMEIRO INCLUDE para definições funcionarem
#include "hardware_config.h"  // Configuração centralizada de todos os pinos GPIO
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


void vTask1(void *pvParameters);  //
void vTask2(void *pvParameters);  //
void define_hardware();


extern String ip;



#endif  //MAIN_H_