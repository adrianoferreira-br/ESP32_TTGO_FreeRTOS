/*
 *
 */

#ifndef MAIN_H_
#define MAIN_H_

#include <Arduino.h>
#include "wifi_mqtt.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "state.h"
#include "esp_task_wdt.h"
#include "dht_xx.h"


void vTask1(void *pvParameters);  //
void vTask2(void *pvParameters);  //
void define_hardware();


extern String ip;



#endif  //MAIN_H_