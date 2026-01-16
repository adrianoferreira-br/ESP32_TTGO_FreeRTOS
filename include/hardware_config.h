/*
 * HARDWARE_CONFIG.H
 * 
 * Configuração centralizada de todos os pinos GPIO para diferentes placas ESP32.
 * Este é o ÚNICO lugar onde os pinos devem ser definidos.
 * 
 * Placas suportadas:
 *   - LILYGO_T_DISPLAY_S3 (ESP32-S3)
 *   - TTGO T-Display (ESP32 original)
 * 
 * Autor: Sistema de configuração centralizada
 * Data: 2026-01-15
 */

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// ============================================================================
// CONFIGURAÇÃO POR PLACA
// ============================================================================

#ifdef LILYGO_T_DISPLAY_S3
  // ==========================================================================
  // LILYGO T-DISPLAY S3 (ESP32-S3)
  // ==========================================================================
  
  // --------------------------------------------------------------------------
  // BOTÕES
  // --------------------------------------------------------------------------
  #define BTN_BOOT          14    // GPIO 14 - Botão BOOT no ESP32-S3
  #define BTN_USER          14    // GPIO 14 - Usar o mesmo que BOOT (GPIO 35 não existe no S3)
  
  // --------------------------------------------------------------------------
  // DISPLAY (PINOS RESERVADOS - NÃO USAR PARA OUTROS PROPÓSITOS)
  // --------------------------------------------------------------------------
  // GPIO 12: SCLK (Clock do display)
  // GPIO 13: DC (Data/Command do display)
  // GPIO 38: Backlight
  // Estes pinos são controlados internamente pela biblioteca TFT_eSPI
  
  // --------------------------------------------------------------------------
  // SENSORES DE BATIDA (PRENSA)
  // --------------------------------------------------------------------------
  #define SENSOR_BATIDA_1    3    // GPIO 3 (GPIO 12 conflita com display SCLK)
  #define SENSOR_BATIDA_2   16    // GPIO 16 (GPIO 13 conflita com display DC)
  
  // --------------------------------------------------------------------------
  // SENSOR ULTRASSÔNICO (JSN-SR04T ou HC-SR04)
  // --------------------------------------------------------------------------
  #define ULTRASONIC_TRIG    1    // GPIO 1 (TX) - Disponível no S3
  #define ULTRASONIC_ECHO    2    // GPIO 2 - Disponível no S3
  
  // --------------------------------------------------------------------------
  // SENSOR DE TEMPERATURA E UMIDADE (DHT22/DHT11)
  // --------------------------------------------------------------------------
  #define DHT_DATA_PIN      21    // GPIO 21 - Compatível com S3
  
  // --------------------------------------------------------------------------
  // SENSOR DE TEMPERATURA 1-WIRE (DS18B20)
  // --------------------------------------------------------------------------
  #define DS18B20_DATA_PIN  22    // GPIO 22 - Comunicação 1-Wire
  
  // --------------------------------------------------------------------------
  // LEITURA DE TENSÃO DA BATERIA (ADC)
  // --------------------------------------------------------------------------
  #define VBAT_ADC_PIN       4    // GPIO 4 - ADC para leitura de bateria
  
  // --------------------------------------------------------------------------
  // I2C (MLX90614 e outros sensores I2C)
  // --------------------------------------------------------------------------
  #define I2C_SDA_PIN       21    // GPIO 21 - Compatível (pode compartilhar com DHT se não usar simultaneamente)
  #define I2C_SCL_PIN       22    // GPIO 22 - Compatível (pode compartilhar com DS18B20 se não usar simultaneamente)
  
  // Informações da placa
  #define BOARD_NAME        "LilyGo T-Display S3 (ESP32-S3)"
  
#else
  // ==========================================================================
  // TTGO T-DISPLAY (ESP32 ORIGINAL)
  // ==========================================================================
  
  // --------------------------------------------------------------------------
  // BOTÕES
  // --------------------------------------------------------------------------
  #define BTN_BOOT           0    // GPIO 0 - Botão BOOT
  #define BTN_USER          35    // GPIO 35 - Botão lateral (somente entrada)
  
  // --------------------------------------------------------------------------
  // DISPLAY (PINOS RESERVADOS)
  // --------------------------------------------------------------------------
  // Controlado pela biblioteca TFT_eSPI
  
  // --------------------------------------------------------------------------
  // SENSORES DE BATIDA (PRENSA)
  // --------------------------------------------------------------------------
  #define SENSOR_BATIDA_1   12    // GPIO 12 - Sensor de batida 1
  #define SENSOR_BATIDA_2   13    // GPIO 13 - Sensor de batida 2
  
  // --------------------------------------------------------------------------
  // SENSOR ULTRASSÔNICO (JSN-SR04T ou HC-SR04)
  // --------------------------------------------------------------------------
  #define ULTRASONIC_TRIG   26    // GPIO 26 - Trigger
  #define ULTRASONIC_ECHO   27    // GPIO 27 - Echo
  
  // --------------------------------------------------------------------------
  // SENSOR DE TEMPERATURA E UMIDADE (DHT22/DHT11)
  // --------------------------------------------------------------------------
  #define DHT_DATA_PIN      21    // GPIO 21 - Data do DHT
  
  // --------------------------------------------------------------------------
  // SENSOR DE TEMPERATURA 1-WIRE (DS18B20)
  // --------------------------------------------------------------------------
  #define DS18B20_DATA_PIN  22    // GPIO 22 - Comunicação 1-Wire
  
  // --------------------------------------------------------------------------
  // LEITURA DE TENSÃO DA BATERIA (ADC)
  // --------------------------------------------------------------------------
  #define VBAT_ADC_PIN      35    // GPIO 35 - ADC (compartilhado com BTN_USER)
  
  // --------------------------------------------------------------------------
  // I2C (MLX90614 e outros sensores I2C)
  // --------------------------------------------------------------------------
  #define I2C_SDA_PIN       21    // GPIO 21 - SDA (padrão ESP32)
  #define I2C_SCL_PIN       22    // GPIO 22 - SCL (padrão ESP32)
  
  // Informações da placa
  #define BOARD_NAME        "TTGO T-Display (ESP32)"
  
#endif

// ============================================================================
// ALIASES PARA COMPATIBILIDADE COM CÓDIGO LEGADO
// ============================================================================
// Mantém nomes antigos funcionando durante a transição

#define BUTTON_35         BTN_USER
#define PINO_12           SENSOR_BATIDA_1
#define BATIDA_PIN        SENSOR_BATIDA_1
#define BATIDA_PIN_SENSOR2 SENSOR_BATIDA_2
#define DHTPIN            DHT_DATA_PIN
#define VBAT_PIN          VBAT_ADC_PIN

// ============================================================================
// NOTAS E AVISOS
// ============================================================================

#ifdef LILYGO_T_DISPLAY_S3
  #warning "Configuração de hardware: LilyGo T-Display S3 (ESP32-S3)"
  #warning "ATENÇÃO: GPIO 12 e 13 são usados pelo display - sensores remapeados para GPIO 3 e 16"
#else
  #warning "Configuração de hardware: TTGO T-Display (ESP32 original)"
#endif

// ============================================================================
// FUNÇÕES AUXILIARES
// ============================================================================

// Retorna o nome da placa em tempo de execução
inline const char* get_board_name() {
  return BOARD_NAME;
}

// Imprime configuração de pinos no Serial
inline void print_pin_configuration() {
  Serial.println("\n========================================");
  Serial.println("CONFIGURAÇÃO DE HARDWARE");
  Serial.println("========================================");
  Serial.printf("Placa: %s\n", BOARD_NAME);
  Serial.println("----------------------------------------");
  Serial.printf("Botão BOOT:          GPIO %d\n", BTN_BOOT);
  Serial.printf("Botão USER:          GPIO %d\n", BTN_USER);
  Serial.printf("Sensor Batida 1:     GPIO %d\n", SENSOR_BATIDA_1);
  Serial.printf("Sensor Batida 2:     GPIO %d\n", SENSOR_BATIDA_2);
  Serial.printf("Ultrassônico TRIG:   GPIO %d\n", ULTRASONIC_TRIG);
  Serial.printf("Ultrassônico ECHO:   GPIO %d\n", ULTRASONIC_ECHO);
  Serial.printf("DHT Data:            GPIO %d\n", DHT_DATA_PIN);
  Serial.printf("DS18B20 Data:        GPIO %d\n", DS18B20_DATA_PIN);
  Serial.printf("VBAT ADC:            GPIO %d\n", VBAT_ADC_PIN);
  Serial.printf("I2C SDA:             GPIO %d\n", I2C_SDA_PIN);
  Serial.printf("I2C SCL:             GPIO %d\n", I2C_SCL_PIN);
  Serial.println("========================================\n");
}

#endif // HARDWARE_CONFIG_H
