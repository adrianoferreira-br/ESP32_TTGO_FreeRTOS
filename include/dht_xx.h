#ifndef DHT_XX_H
#define DHT_XX_H


#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include "display.h"
#include "main.h"


// Defina os pinos e tipo do sensor conforme seu hardware
#define DHTTYPE DHT22    // DHT11, DHT21, DHT22


void dht_setup();
void dht_loop();





#endif