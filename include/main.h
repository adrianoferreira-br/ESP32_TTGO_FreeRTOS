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
#include "mem_flash.h"
#include "web_server.h"
#include <Preferences.h>
#include "wifi_mqtt.h"


#define ULTRASONIC_TRIG 26
#define ULTRASONIC_ECHO 27

#define DHTPIN 21         // pino 21


#define VBAT_PIN 35  // GPIO35 geralmente é usado para ler a tensão


void vTask1(void *pvParameters);  //
void vTask2(void *pvParameters);  //
void define_hardware();


extern String ip;



#endif  //MAIN_H_