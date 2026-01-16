#include "voltage_bat.h"

void setup_tensao_bat() {
  #ifdef LILYGO_T_DISPLAY_S3
    Serial.println("Leitura de bateria no S3: usando GPIO 4");
  #endif
  analogReadResolution(12); // 12 bits (0-4095)
  // Se quiser, configure o atenuador para ler tensões maiores:
  analogSetAttenuation(ADC_11db); // até ~3.3V
}


void loop_tensao_bat() {
    #ifdef LILYGO_T_DISPLAY_S3
      // GPIO 4 no S3 pode não ter divisor de tensão - verificar hardware
      Serial.println("Leitura de bateria desabilitada temporariamente no S3");  //ToDo
      return;
    #endif
    
    float vbat = read_tensao();
    Serial.print("Tensão da placa: ");
    Serial.print(vbat, 2);
    Serial.println(" V");
    show_battery_voltage(vbat);    
}



float read_tensao() {
  #ifdef LILYGO_T_DISPLAY_S3
    // Retorna valor dummy para evitar leitura de pino inválido
    return 3.3; //ToDo: Implementar leitura correta no S3
  #else
    int raw = analogRead(VBAT_ADC_PIN);
    // O divisor de tensão da placa TTGO T-Display é geralmente 2:1
    // 3.3V -> 4095 (12 bits)
    float tensao = (raw / 4095.0) * 3.3; // Multiplica por 2 por causa do divisor
    return tensao;
  #endif
}