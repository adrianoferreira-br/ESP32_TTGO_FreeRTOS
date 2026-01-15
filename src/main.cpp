//https://github.com/Xinyuan-LilyGO/TTGO-T-Display


#include "main.h"

// Remapeamento de pinos para LilyGo T-Display S3
#ifdef LILYGO_T_DISPLAY_S3
  #define BUTTON_35 14   // GPIO 0 (botão BOOT) no ESP32-S3
  #define PINO_12 3    // GPIO 15 (GPIO 12 usado pelo display SCLK)
#else
  #define BUTTON_35 35  // GPIO 35 no ESP32 original
  #define PINO_12 12    // GPIO 12 no ESP32 original
#endif

#define WDT_TIMEOUT 5


TaskHandle_t task_handle_Menu = NULL;
TaskHandle_t task_handle2 = NULL;
bool initial_call = true;






/**************************************************************
 * SETUP PRINCIPAL
 */


void setup() {  
  Serial.begin(115200);
  delay(500); // Aguarda estabilização da serial
  
  Serial.println("\n\n========== BOOT INICIADO ==========");
  
  #ifdef LILYGO_T_DISPLAY_S3
    Serial.println("Placa: LilyGo T-Display S3 (ESP32-S3)");
    // Desabilita watchdog temporariamente para debug
    //esp_task_wdt_delete(NULL);
  #else
    Serial.println("Placa: TTGO T-Display (ESP32)");
  #endif
  
  // VERIFICAÇÃO CRÍTICA DE PARTIÇÕES - DEVE SER PRIMEIRO
  Serial.println("=== VERIFICAÇÃO CRÍTICA DE BOOT ===");
  Serial.printf("FIRMWARE VERSION: %s\n", VERSION);
  
  const esp_partition_t* running = esp_ota_get_running_partition();
  const esp_partition_t* boot_partition = esp_ota_get_boot_partition();
  
  if (running && boot_partition) {
    Serial.printf("Executando de: %s (0x%06x)\n", running->label, running->address);
    Serial.printf("Boot config: %s (0x%06x)\n", boot_partition->label, boot_partition->address);
    
    if (running->address != boot_partition->address) {
      Serial.println("❌ INCONSISTÊNCIA DETECTADA!");
      Serial.println("Sistema não está executando da partição configurada como boot");
      
      // Tentar forçar a ativação da partição correta
      Serial.println("Tentando corrigir...");
      esp_err_t err = esp_ota_set_boot_partition(boot_partition);
      if (err == ESP_OK) {
        Serial.println("✅ Correção aplicada - será ativada no próximo reboot");
      } else {
        Serial.printf("❌ Falha na correção: %s\n", esp_err_to_name(err));
      }
    } else {
      Serial.println("✅ Partições consistentes");
    }
  }
  Serial.println("=====================================");

  /*    HARDWARE   */
  Serial.println("\n[1/15] Configurando hardware...");     
  define_hardware();
  
  Serial.println("[2/15] Configurando memoria flash...");
  setup_mem_flash();
  
  Serial.println("[3/15] Mostrando particoes...");
  show_partitions();
  
  Serial.println("[4/15] Carregando configuracoes...");
  load_all_settings_from_flash(); // 📂 Carrega todas as configurações da flash
  
  Serial.println("[5/15] Info OTA...");
  show_ota_info();      // Adicionar info sobre partições OTA
  
  Serial.println("[6/15] Setup timer...");
  setup_timer();
  
  Serial.println("[7/15] Setup timer takt...");
  setup_timer_send_takt_time();

  /*    WIFI    */
  Serial.println("[8/15] Setup WiFi...");
  setup_wifi();       
  setup_ota();
  setup_webserver();
  setup_ntp();
  
  /*    DISPLAY  */
  init_display();      
  showBootInfo(); // Exibe informações de boot (versão e nome do equipamento)

  /*    MQTT    */
  setup_mqtt();
  
  /***** DEFINE APLICAÇÃO ******/ 

  /*    BATIDA   */
  #ifdef SENSOR_BATIDA
    setup_batidas_prensa();   
  #endif

  /*  DHT Sensor  */
  #ifdef SENSOR_TEMPERATURE
    dht_setup();
  #endif

  /* Ultrassônico */
  #ifdef SENSOR_WATER_LEVEL
    setup_ultrasonic();
    set_reservatorio();
  #endif

  /* Battery Voltage */
  #ifdef SENSOR_BATTERY_VOLTAGE
    setup_tensao_bat();
    Serial.println("Setup Battery Voltage inicializado");
  #endif

  /* MLX90614 Infrared Temperature Sensor */
  #ifdef SENSOR_MLX90614
    mlx90614_setup();
  #endif

  /* DS18B20 Temperature Sensor */
  #ifdef SENSOR_DS18B20
    ds18b20_setup();
  #endif

  /* botão de configuração */
 
  tft.fillScreen(TFT_BLACK);
}



/**************************************************************
 * LOOP PRINCIPAL
 */

void loop() 
{
  //    WIFI    
  loop_wifi();      

  //    OTA    
  loop_ota();

  //    MQTT    
  loop_mqqt();

  //    WEB SERVER    
  loop_webserver();

  //    WDT    
  esp_task_wdt_reset();

  //    APPLICATION_1    
  loop_state();

  //    APPLICATION_2    

  #ifdef SENSOR_BATIDA
      verifica_interrupcao();      
      check_timer_interrupt_tosend_MqttDataReadings();
  #endif

  //  DHT Sensor  
  #ifdef SENSOR_TEMPERATURE
    dht_loop();
  #endif

  // Ultrassônico - Executado apenas a cada 2 segundos (não-bloqueante)
  #ifdef SENSOR_WATER_LEVEL
    static unsigned long last_ultrasonic_time = 0;
    unsigned long current_time = millis();
    if (current_time - last_ultrasonic_time >= 2000) {  // 2000ms = 2 segundos
      loop_ultrasonic();
      last_ultrasonic_time = current_time;
    }
    verifica_timer();
  #endif

  // Battery Voltage 
  #ifdef SENSOR_BATTERY_VOLTAGE
    //loop_tensao_bat();  //GPIO35 é compartilhado com sensor de tensão da bateria.
  #endif

  // MLX90614 Infrared Temperature Sensor
  #ifdef SENSOR_MLX90614
    mlx90614_loop();
  #endif

  // DS18B20 Temperature Sensor
  #ifdef SENSOR_DS18B20
    ds18b20_loop();
  #endif
 
     

  if (initial_call) {   //envia uma mqtt de inicialização
    mqtt_send_info();
    initial_call = false;
  }
}



void vTask2(void *pvParameters) 
{  
  while (1)
  {    
    //displayPrint("123",3, 50, 1);      
    //vTaskDelay(pdMS_TO_TICKS(1000));
  }

}



void define_hardware(){

  Serial.begin(115200); 
  
  Serial.print("Configurando pinos - BUTTON_35: GPIO ");
  Serial.print(BUTTON_35);
  Serial.print(" | PINO_12: GPIO ");
  Serial.println(PINO_12);

  // circuito prensa
  Serial.println("  -> pinMode PINO_12...");
  pinMode(PINO_12, INPUT_PULLUP);
  Serial.println("  -> PINO_12 OK");

  // botão proximo ao reset
  Serial.println("  -> pinMode BUTTON_35...");  
  pinMode(BUTTON_35, INPUT);
  Serial.println("  -> BUTTON_35 OK");
  
  // sensor ultrassônico
  Serial.println("  -> pinMode ULTRASONIC_TRIG...");
  pinMode(ULTRASONIC_TRIG, OUTPUT); //pino 26
  Serial.println("  -> ULTRASONIC_TRIG OK");
  
  Serial.println("  -> pinMode ULTRASONIC_ECHO...");  
  pinMode(ULTRASONIC_ECHO, INPUT);  //pino 27
  Serial.println("  -> ULTRASONIC_ECHO OK");

  // sensor de temperatura
  Serial.println("  -> pinMode DHTPIN...");
  pinMode(DHTPIN, INPUT); //pino 21
  Serial.println("  -> DHTPIN OK");
  
  #ifdef LILYGO_T_DISPLAY_S3
    Serial.println("Hardware: LilyGo T-Display S3");
  #else
    Serial.println("Hardware: TTGO T-Display");
  #endif

}

