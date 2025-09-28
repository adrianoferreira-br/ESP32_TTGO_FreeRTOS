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


#endif // JSN_SR04T_H_