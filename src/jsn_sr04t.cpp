/*  https://makerhero.com/img/files/download/RCWL-1655-Datasheet.pdf
*   https://www.makerhero.com/produto/modulo-sensor-ultrassonico-impermeavel-jsn-sr04t/
*
*
*/

#include "jsn_sr04t.h"
#include "display.h"


/*  SETUP */
void setup_ultrasonic() {  
  
  digitalWrite(ULTRASONIC_TRIG, LOW);  
  Serial.println("Ultrassônico pronto!");

}


/* LOOP */
void loop_ultrasonic() {

   float dist = ultrasonic_read_cm();
   if (dist >= 0) {
       Serial.print("Distância: ");
       Serial.print(dist);
       Serial.println(" cm");
       show_distancia(dist);
   } else {
       Serial.println("Falha na leitura do ultrassônico!");
   }      
}



float ultrasonic_read_cm() {
  // Gera pulso de trigger
  digitalWrite(ULTRASONIC_TRIG, LOW);
  delayMicroseconds(200);
  digitalWrite(ULTRASONIC_TRIG, HIGH);
  delayMicroseconds(15);
  digitalWrite(ULTRASONIC_TRIG, LOW);

  // Mede o tempo do pulso no pino ECHO
  long duration = pulseIn(ULTRASONIC_ECHO, HIGH, 30000); // timeout 30ms (~5m)
  if (duration == 0) return -1; // Falha na leitura

  // Calcula a distância em centímetros
  float distance_cm = (duration / 2.0) * 0.0343;
  
  return distance_cm;
}