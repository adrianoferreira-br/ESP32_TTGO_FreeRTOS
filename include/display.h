#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <TFT_eSPI.h>
#include "display.h"
#include "Arduino.h"


void init_display();
void displayPrint(char*,int,int,int);
void graficoBarra(int, int, int, int, int, int, int);
void drawGauge(int);
void showBootInfo();
void show_temperature(float temp);
void show_humidity(float hum);
void show_distancia(float dist);
void show_battery_voltage(float voltage);
void show_batidas(int batidas);
void show_time(char* timeStr);


extern TFT_eSPI tft;

#endif