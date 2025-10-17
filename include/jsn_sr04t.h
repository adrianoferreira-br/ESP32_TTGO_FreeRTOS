#ifndef JSN_SR04T_H_
#define JSN_SR04T_H_

#include <Arduino.h>
#include "main.h"


struct UltrasonicResult {
    float distance_cm;
    float percentual;
    bool valido;
};


void setup_ultrasonic();
void loop_ultrasonic();
UltrasonicResult ultrasonic_read();
void reset_percentual_filter(); // Reset do filtro inteligente


#endif // JSN_SR04T_H_