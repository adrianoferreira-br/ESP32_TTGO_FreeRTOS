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

extern TFT_eSPI tft;

#endif