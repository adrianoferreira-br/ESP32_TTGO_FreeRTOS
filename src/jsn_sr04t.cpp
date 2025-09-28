/*  https://makerhero.com/img/files/download/RCWL-1655-Datasheet.pdf
*   https://www.makerhero.com/produto/modulo-sensor-ultrassonico-impermeavel-jsn-sr04t/
*
*
*/

#include "jsn_sr04t.h"
#include "display.h"


struct UltrasonicResult resultado;


/*  SETUP */
void setup_ultrasonic() {  
  
  digitalWrite(ULTRASONIC_TRIG, LOW);  
  Serial.println("Ultrassônico pronto!");

}


/* LOOP */
void loop_ultrasonic() {

   UltrasonicResult res = ultrasonic_read();
   if (res.valido) {  
       Serial.print("Distância: ");
       Serial.print(res.distance_cm);
       Serial.print(" cm | Percentual: ");
       Serial.print(res.percentual);
       Serial.println(" %");
       show_distancia(res.distance_cm);
       show_percentual_reservatorio(res.percentual);
   } else {
       Serial.println("Falha na leitura do ultrassônico!");
   }
}



UltrasonicResult ultrasonic_read() {
    UltrasonicResult result;
    result.distance_cm = -1;
    result.percentual = 0;
    result.valido = false;

    digitalWrite(ULTRASONIC_TRIG, LOW);
    delayMicroseconds(200);
    digitalWrite(ULTRASONIC_TRIG, HIGH);
    delayMicroseconds(15);
    digitalWrite(ULTRASONIC_TRIG, LOW);

    long duration = pulseIn(ULTRASONIC_ECHO, HIGH, 30000);
    if (duration == 0) return result;

    float distance_cm = (duration / 2.0) * 0.0343;
    float distance_max = length_max;

    if (distance_cm > distance_max) {
        Serial.println("Distância acima do máximo permitido!");
        return result;
    }
    // despreza 20cm de atuação mínima de leitura do sensor e realiza o percentual de liquido no reservatorio
    float percentual = (1-((distance_max - distance_cm - 20) / (distance_max - 20)) * 100.0) ;
    if (percentual < 0) percentual = 0;
    if (percentual > 100) percentual = 100;

    result.distance_cm = distance_cm;
    result.percentual = percentual;
    result.valido = true;

    percentual_reservatorio = percentual;


    return result;
}