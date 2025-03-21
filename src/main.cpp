//https://github.com/Xinyuan-LilyGO/TTGO-T-Display


#include "main.h"


#define BOTAO_35 35
#define PINO_12 12
#define WDT_TIMEOUT 5


TaskHandle_t task_handle_Menu = NULL;
TaskHandle_t task_handle2 = NULL;




void setup() {

  // Display
  init_display();
  
  // WIFI
  setup_wifi(); 

  // MQTT
  setup_mqtt();

  // FreeRTOS
  //xTaskCreate(vTask_Menu, "TaskMenu", 2048, NULL, 1, &task_handle_Menu);
  //xTaskCreate(vTask2, "Task2", 2048, NULL, 1, &task_handle2);

  //state
  init_state();

  // Batidas prensa
  pinMode(PINO_12, INPUT);
  attachInterrupt(PINO_12, InterruptionPino12, RISING);

  // Interruptions
  pinMode(BOTAO_35, INPUT);
  attachInterrupt(BOTAO_35, InterruptionPino35, RISING);  

  //WDT  
  //esp_task_wdt_init(WDT_TIMEOUT, true);
  //esp_task_wdt_add(NULL);



}

void loop() 
{
  // verifica conexão e sinal do wifi
  loop_wifi();

  // Execução principal da aplicação  
  //loop_state();

  // Monitoramento de msg do broker mqtt
  loop_mqqt();

  //WDT
  //esp_task_wdt_reset();

  //Processa os batimentos da prensa
  verifica_batida_prensa();

}




void vTask2(void *pvParameters) 
{  
  while (1)
  {    
    //displayPrint("123",3, 50, 1);      
    //vTaskDelay(pdMS_TO_TICKS(1000));
  }

}


