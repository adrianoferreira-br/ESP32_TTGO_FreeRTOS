#include <Arduino.h>
#include <TFT_eSPI.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"



TFT_eSPI tft = TFT_eSPI();
TaskHandle_t task_handle1 = NULL;
TaskHandle_t task_handle2 = NULL;


void vTask1(void *pvParameters);
void vTask2(void *pvParameters);

// put function declarations here:
//#define LED_PIN 2




void setup() {
  // put your setup code here, to run once:
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Hello World!", 0, 0, 2);  


  //Serial.begin(9600);
  //pinMode(LED_PIN, OUTPUT);
  xTaskCreate(vTask1, "Task1", 2048, NULL, 1, &task_handle1);
  xTaskCreate(vTask2, "Task2", 2048, NULL, 1, &task_handle2);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  vTaskDelay(1000);

}


void vTask1(void *pvParameters)
  {
    //pinMode(LED_PIN, OUTPUT);
    int x,y,textsize = 0;
    while (1)
    {
      //digitalWrite(LED_PIN, HIGH);
      //tft.drawString("0", 20, 5, 2 );      
      tft.drawChar(x,y,'A',2,2,textsize);
      x += textsize * 6;
      vTaskDelay(pdMS_TO_TICKS(2000));
    }
    
  }


void vTask2(void *pvParameters) {
  int cont = 0;
  while (1)
  {
    //Serial.println("Task2: " + String(cont++));
    //tft.drawString("1", 0, 0, 2);
    tft.drawString("1", 20, 5, 2 );      
    vTaskDelay(pdMS_TO_TICKS(1000));
  }

}