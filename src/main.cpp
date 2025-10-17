//https://github.com/Xinyuan-LilyGO/TTGO-T-Display


#include "main.h"


#define BUTTON_35 35
#define PINO_12 12
#define WDT_TIMEOUT 5


TaskHandle_t task_handle_Menu = NULL;
TaskHandle_t task_handle2 = NULL;
bool initial_call = true;






/**************************************************************
 * SETUP PRINCIPAL
 */


void setup() {  
  Serial.begin(115200); 
  
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
  define_hardware();   
  setup_mem_flash(); 
  show_partitions();
  show_ota_info();      // Adicionar info sobre partições OTA
  setup_timer();

  /*    WIFI    */
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
  #endif

  //  DHT Sensor  
  #ifdef SENSOR_TEMPERATURE
    dht_loop();
  #endif

  // Ultrassônico 
  #ifdef SENSOR_WATER_LEVEL
    loop_ultrasonic();
    verifica_timer();
  #endif

  // Battery Voltage 
  #ifdef SENSOR_BATTERY_VOLTAGE
    //loop_tensao_bat();  //GPIO35 é compartilhado com sensor de tensão da bateria.
  #endif

  // delay 
  #ifdef SENSOR_WATER_LEVEL  
    delay(2000);
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

  // circuito prensa
  pinMode(PINO_12, INPUT_PULLUP);   

  // botão proximo ao reset  
  pinMode(BUTTON_35, INPUT);     
  
  // sensor ultrassônico
  pinMode(ULTRASONIC_TRIG, OUTPUT); //pino 26  
  pinMode(ULTRASONIC_ECHO, INPUT);  //pino 27

  // sensor de temperatura
  pinMode(DHTPIN, INPUT); //pino 21

}

