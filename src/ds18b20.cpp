#include "ds18b20.h"

// Configuração do barramento OneWire e sensor Dallas
OneWire oneWire(DS18B20_DATA_PIN);
DallasTemperature sensors(&oneWire);

// Variáveis globais
float temperatura_ds18b20 = 0.0;
int num_sensors_ds18b20 = 0;

/**
 * Inicializa o sensor DS18B20
 * Configura o pino GPIO 22 para comunicação 1-Wire
 */
void ds18b20_setup() {
  Serial.println("====================================");
  Serial.println("Inicializando sensor DS18B20...");
  Serial.printf("Pino: GPIO %d\n", DS18B20_DATA_PIN);
  
  // Inicializa o barramento OneWire
  sensors.begin();
  
  // Conta quantos sensores estão conectados no barramento
  num_sensors_ds18b20 = sensors.getDeviceCount();
  
  if (num_sensors_ds18b20 == 0) {
    Serial.println("❌ ERRO: Nenhum sensor DS18B20 encontrado!");
    Serial.println("   Verifique as conexões:");
    Serial.printf("   - DATA: GPIO %d (com resistor pull-up 4.7kΩ)\n", DS18B20_DATA_PIN);
    Serial.println("   - VCC: 3.3V");
    Serial.println("   - GND: GND");
    return;
  }
  
  Serial.printf("✅ DS18B20 inicializado com sucesso!\n");
  Serial.printf("   Número de sensores encontrados: %d\n", num_sensors_ds18b20);
  
  // Configura a resolução (9, 10, 11 ou 12 bits)
  // 12 bits: 0.0625°C, tempo de conversão ~750ms
  sensors.setResolution(12);
  Serial.printf("   Resolução: %d bits\n", sensors.getResolution());
  
  Serial.println("====================================");
}

/**
 * Lê a temperatura do primeiro sensor DS18B20
 * Atualiza a variável global temperatura_ds18b20
 */
void ds18b20_loop() {
  if (num_sensors_ds18b20 == 0) {
    Serial.println("⚠️ DS18B20: Nenhum sensor disponível para leitura");
    return;
  }
  
  // Solicita leitura de todos os sensores no barramento
  sensors.requestTemperatures();
  
  // Lê a temperatura do primeiro sensor (índice 0)
  temperatura_ds18b20 = sensors.getTempCByIndex(0);
  
  // Verifica se a leitura é válida
  if (temperatura_ds18b20 == DEVICE_DISCONNECTED_C) {
    Serial.println("❌ DS18B20: Erro na leitura - Sensor desconectado");
    temperatura_ds18b20 = -127.0; // Valor padrão de erro
    return;
  }
  
  Serial.println("------------------------------------");
  Serial.println("DS18B20 - Leitura:");
  Serial.print("  Temperatura: ");
  Serial.print(temperatura_ds18b20, 2);
  Serial.println(" °C");
  
  // Se houver múltiplos sensores, mostra todos
  if (num_sensors_ds18b20 > 1) {
    Serial.printf("  Total de sensores: %d\n", num_sensors_ds18b20);
    for (int i = 0; i < num_sensors_ds18b20; i++) {
      float temp = sensors.getTempCByIndex(i);
      Serial.printf("  Sensor %d: %.2f °C\n", i, temp);
    }
  }
  
  Serial.println("------------------------------------");
}

/**
 * Retorna a temperatura do primeiro sensor em °C
 * @return Temperatura em graus Celsius
 */
float ds18b20_read_temperature() {
  if (num_sensors_ds18b20 == 0) {
    return -127.0; // Código de erro
  }
  
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  
  if (temp == DEVICE_DISCONNECTED_C) {
    return -127.0; // Código de erro
  }
  
  return temp;
}

/**
 * Retorna a temperatura de um sensor específico pelo índice
 * @param index Índice do sensor (0 para o primeiro)
 * @return Temperatura em graus Celsius
 */
float ds18b20_read_temperature_by_index(int index) {
  if (num_sensors_ds18b20 == 0 || index >= num_sensors_ds18b20) {
    return -127.0; // Código de erro
  }
  
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(index);
  
  if (temp == DEVICE_DISCONNECTED_C) {
    return -127.0; // Código de erro
  }
  
  return temp;
}

/**
 * Retorna o número de sensores DS18B20 conectados
 * @return Número de sensores no barramento
 */
int ds18b20_get_device_count() {
  return num_sensors_ds18b20;
}
