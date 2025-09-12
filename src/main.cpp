//https://github.com/Xinyuan-LilyGO/TTGO-T-Display


#include "main.h"



#define BOTAO_35 35
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

  /*    WIFI    */
  setup_wifi();  

  /*    PARTITIONS   */   
  show_partitions();

  /*    OTA   */   
  setup_ota();

  /*    DISPLAY  */
  init_display();  
  
  /*    MQTT    */
  setup_mqtt();

  /*    APPLICATION   */
  setup_batidas_prensa();

  /*  DHT Sensor  */
  dht_setup();


}



/**************************************************************
 * LOOP PRINCIPAL
 */

void loop() 
{
  /*    WIFI    */
  loop_wifi();  

  /*    OTA   */
  loop_ota();

  /*    MQTT    */
  loop_mqqt();

  /*    WDT    */
  //esp_task_wdt_reset();

  /*    APPLICATION_1    */
  //loop_state();

  /*    APPLICATION_2    */
  //verifica_batida_prensa();
  verifica_interrupcao();

  /*  DHT Sensor  */
  //dht_loop();

  

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
  //pinMode(PINO_12, INPUT_PULLUP);   

  // botão proximo ao reset  
  //pinMode(BOTAO_35, INPUT);    

}

