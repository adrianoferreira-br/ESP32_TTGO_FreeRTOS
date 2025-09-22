#include "voltage_bat.h"

void setup_tensao_bat() {
  analogReadResolution(12); // 12 bits (0-4095)
  // Se quiser, configure o atenuador para ler tensões maiores:
  analogSetAttenuation(ADC_11db); // até ~3.3V
}


void loop_tensao_bat() {
    float vbat = read_tensao();
    Serial.print("Tensão da placa: ");
    Serial.print(vbat, 2);
    Serial.println(" V");
    delay(2000);
}



float read_tensao() {
  int raw = analogRead(VBAT_PIN);
  // O divisor de tensão da placa TTGO T-Display é geralmente 2:1
  // 3.3V -> 4095 (12 bits)
  float tensao = (raw / 4095.0) * 3.3; // Multiplica por 2 por causa do divisor
  return tensao;
}