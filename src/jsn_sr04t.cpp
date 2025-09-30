/*  https://makerhero.com/img/files/download/RCWL-1655-Datasheet.pdf
*   https://www.makerhero.com/produto/modulo-sensor-ultrassonico-impermeavel-jsn-sr04t/
*
*
*/

#include "jsn_sr04t.h"
#include "display.h"


struct UltrasonicResult resultado;
int altura_despresada = 20.0; // cm, nível mínimo de leitura útil
float nivel_max = altura_reservatorio; // altura total do reservatório
float altura_medida = 0.0; // altura medida pelo sensor


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
       altura_medida = altura_reservatorio - res.distance_cm;
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
    // Considerando ponto inicial o topo do reservatório e desprezando os primeiros 20cm devido atuação mínima de leitura do sensor,
    // segue o cálculo do percentual de liquido no reservatorio, já descontado esses 20cm acima do nível da agua
    float percentual = ((distance_max - distance_cm) / (distance_max - altura_despresada)) * 100.0;
    // limitar o percentual entre 0 e 100
    if (percentual < 0) percentual = 0;
    if (percentual > 100) percentual = 100;

    result.distance_cm = distance_cm;
    result.percentual = percentual;
    result.valido = true;

    percentual_reservatorio = percentual;


    return result;
}