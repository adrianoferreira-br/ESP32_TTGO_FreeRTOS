//https://github.com/Xinyuan-LilyGO/TTGO-T-Display


#include "main.h"



#define BUTTON_35 35
#define PINO_12 12
#define WDT_TIMEOUT 5


TaskHandle_t task_handle_Menu = NULL;
TaskHandle_t task_handle2 = NULL;



/**************************************************************
 * SETUP PRINCIPAL
 */


void setup() {  

  /*    HARDWARE   */   
  define_hardware();
  setup_mem_flash();

  /*    WIFI    */
  setup_wifi();  

  /*    PARTITIONS   */   
  show_partitions();

    /*    DISPLAY  */
  init_display();    
  Serial.println("Display inicializado");
  showBootInfo(); // Exibe informações de boot (versão e nome do equipamento)

  /*    MQTT    */
  setup_mqtt();
  Serial.println("Setup MQTT inicializado");

  /*    OTA   */   
  setup_ota();
  Serial.println("Setup OTA inicializado");


  /*    BATIDA   */
  if (SENSOR_BATIDA) {
    setup_batidas_prensa();      
    Serial.println("Setup Batida Prensa inicializado");
  }

  /*  DHT Sensor  */
  if (SENSOR_TEMPERATURE) {
    dht_setup();    
    Serial.println("Setup DHT inicializado");
  }
  
  /* Ultrassônico */
  if (SENSOR_WATER_LEVEL) {
    void setup_ultrasonic();
    Serial.println("Setup Ultrassônico inicializado");
  }

  /* Battery Voltage */
  if (SENSOR_BATTERY_VOLTAGE) {
    //setup_tensao_bat();
    //Serial.println("Setup Battery Voltage inicializado");
  }

  /* botão de configuração */
   define_length_max();
  

  tft.fillScreen(TFT_BLACK);      

}



/**************************************************************
 * LOOP PRINCIPAL
 */

void loop() 
{
  /*    WIFI    */
  loop_wifi();  

  /*    MQTT    */
  loop_mqqt();

  /*    OTA   */
  loop_ota();


  /*    WDT    */
  //esp_task_wdt_reset();

  /*    APPLICATION_1    */
  //loop_state();

  /*    APPLICATION_2    */

  if (SENSOR_BATIDA) {
      verifica_interrupcao();
  }

  /*  DHT Sensor  */
  if (SENSOR_TEMPERATURE){
    dht_loop();
  } 

  /* Ultrassônico */
  if (SENSOR_WATER_LEVEL){
    loop_ultrasonic();
  }

  /* Battery Voltage */
  if (SENSOR_BATTERY_VOLTAGE){
    //loop_tensao_bat();  //GPIO35 é compartilhado com sensor de tensão da bateria.
  }

  /* delay */
  if (!SENSOR_BATIDA){
    delay(2000);  // aguarda 2 segundos para próxima leitura do DHT
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
  
  pinMode(ULTRASONIC_TRIG, OUTPUT);  
  pinMode(ULTRASONIC_ECHO, INPUT); 

}

