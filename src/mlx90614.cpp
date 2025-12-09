#include "mlx90614.h"

// Cria o objeto do sensor MLX90614
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Variáveis globais para armazenar as leituras
float temperatura_ambiente_mlx = 0.0;
float temperatura_objeto_mlx = 0.0;

/**
 * Inicializa o sensor MLX90614
 * Configura o barramento I2C nos pinos 32 (SDA) e 33 (SCL)
 */
void mlx90614_setup() {
  Serial.println("====================================");
  Serial.println("Inicializando sensor MLX90614 (GY-906)...");
  
  // Configura o barramento I2C nos pinos customizados
  Wire.begin(MLX90614_SDA, MLX90614_SCL);
  
  // Inicializa o sensor
  if (!mlx.begin()) {
    Serial.println("❌ ERRO: Sensor MLX90614 não encontrado!");
    Serial.println("   Verifique as conexões:");
    Serial.println("   - SDA: GPIO 32");
    Serial.println("   - SCL: GPIO 33");
    Serial.println("   - VCC: 3.3V");
    Serial.println("   - GND: GND");
    return;
  }
  
  Serial.println("✅ Sensor MLX90614 inicializado com sucesso!");
  Serial.println("   SDA: GPIO 32");
  Serial.println("   SCL: GPIO 33");
  Serial.println("====================================");
}

/**
 * Lê os valores do sensor MLX90614
 * Atualiza as variáveis globais com temperatura ambiente e do objeto
 */
void mlx90614_loop() {
  temperatura_ambiente_mlx = mlx.readAmbientTempC();
  temperatura_objeto_mlx = mlx.readObjectTempC();
  
  Serial.println("------------------------------------");
  Serial.println("MLX90614 - Leitura:");
  Serial.print("  Temperatura Ambiente: ");
  Serial.print(temperatura_ambiente_mlx);
  Serial.println(" °C");
  Serial.print("  Temperatura Objeto:   ");
  Serial.print(temperatura_objeto_mlx);
  Serial.println(" °C");
  Serial.println("------------------------------------");
}

/**
 * Retorna a temperatura ambiente em °C
 */
float mlx90614_read_ambient() {
  return mlx.readAmbientTempC();
}

/**
 * Retorna a temperatura do objeto em °C
 */
float mlx90614_read_object() {
  return mlx.readObjectTempC();
}
