//https://github.com/Xinyuan-LilyGO/TTGO-T-Display


#include "main.h"


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

  // Interruptions
  //pinMode(GPIO_BOTAO, INPUT);
  //attachInterrupt(GPIO_BOTAO, funcao_ISR, RISING);  


}

void loop() 
{
  // Monitoramento de msg do broker mqtt
  //loop_mqqt();
  // Execução principal da aplicação  
  loop_state();
}




void vTask2(void *pvParameters) 
{  
  while (1)
  {    
    //displayPrint("123",3, 50, 1);      
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}


