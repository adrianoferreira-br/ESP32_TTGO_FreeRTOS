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

  /*    HARDWARE   */     
  define_hardware();   
  setup_mem_flash(); 
  show_partitions();
  setup_timer();

  /*    WIFI    */
  setup_wifi();       
  setup_webserver();
  setup_ntp();
  
  /*    DISPLAY  */
  init_display();      
  showBootInfo(); // Exibe informações de boot (versão e nome do equipamento)

  /*    MQTT    */
  setup_mqtt();
  

  /*    OTA   */   
  setup_ota();

  /***** DEFINE APLICAÇÃO ******/
  if (SENSOR_BATIDA) {
    setup_batidas_prensa();
  }

  /*    BATIDA   */
  if (SENSOR_BATIDA) {
    setup_batidas_prensa();          
  }

  /*  DHT Sensor  */
  if (SENSOR_TEMPERATURE) {
    dht_setup();        
  }
  
  /* Ultrassônico */
  if (SENSOR_WATER_LEVEL) {
    void setup_ultrasonic();
    define_length_max();    
  }

  /* Battery Voltage */
  if (SENSOR_BATTERY_VOLTAGE) {
    //setup_tensao_bat();
    //Serial.println("Setup Battery Voltage inicializado");
  }

  /* botão de configuração */   
 
  tft.fillScreen(TFT_BLACK);      

}



/**************************************************************
 * LOOP PRINCIPAL
 */

void loop() 
{
  /*    WIFI    */
  loop_wifi();    
  loop_webserver();  /*    WEB SERVER    */

  

  /*    OTA   */
  loop_ota();


  /*    MQTT    */
  loop_mqqt();


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
 

  verifica_timer();

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
  
  pinMode(ULTRASONIC_TRIG, OUTPUT);  
  pinMode(ULTRASONIC_ECHO, INPUT); 

}

