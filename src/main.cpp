#include <Arduino.h>
#include "main.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


TaskHandle_t task_handle1 = NULL;
TaskHandle_t task_handle2 = NULL;


void setup() {
  // put your setup code here, to run once:
  
  init_display();

  xTaskCreate(vTask1, "Task1", 2048, NULL, 1, &task_handle1);
  xTaskCreate(vTask2, "Task2", 2048, NULL, 1, &task_handle2);
}

void loop() 
{
  // put your main code here, to run repeatedly:
  
 // vTaskDelay(1000);
}


void vTask1(void *pvParameters)
{   
  char str[10] = "ABC"; 
  char* p_str = str;
  while (1)
  { 
    vTaskDelay(pdMS_TO_TICKS(500));     
    displayPrint(p_str,sizeof(str)/sizeof(char),50,50);      
    vTaskDelay(pdMS_TO_TICKS(500));
  }
    
}


void vTask2(void *pvParameters) 
{  
  while (1)
  {    
    displayPrint("123",3, 50, 50);      
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}


//#define LED_PIN 2
//Serial.begin(9600);
  //pinMode(LED_PIN, OUTPUT);
//pinMode(LED_PIN, OUTPUT);
//Serial.println("Task2: " + String(cont++));
    //tft.drawString("1", 0, 0, 2);