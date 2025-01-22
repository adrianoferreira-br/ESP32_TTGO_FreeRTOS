#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <TFT_eSPI.h>
#include "display.h"
#include "Arduino.h"

void init_display();
void displayPrint(char*,int,int,int);


#endif