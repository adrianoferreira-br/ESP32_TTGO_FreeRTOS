#ifndef DS18B20_H
#define DS18B20_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Definição do pino para o sensor DS18B20
#define DS18B20_PIN 22

// Variáveis globais para armazenar a temperatura
extern float temperatura_ds18b20;
extern int num_sensors_ds18b20;

// Funções do sensor DS18B20
void ds18b20_setup();
void ds18b20_loop();
float ds18b20_read_temperature();
float ds18b20_read_temperature_by_index(int index);
int ds18b20_get_device_count();

#endif // DS18B20_H
