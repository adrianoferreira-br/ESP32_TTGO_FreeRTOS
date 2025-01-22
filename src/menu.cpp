/*

*/

#include "menu.h"
#include "main.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "display.h"
#include <TFT_eSPI.h>

/*
*/
void vTask_Menu(void *pvParameters)
{   
  char str[10] = "Rafaelaaa"; 
  char* p_str = str;
  while (1)
  { 
    vTaskDelay(pdMS_TO_TICKS(500));     
    //displayPrint(p_str,sizeof(str)/sizeof(char),50,50);          
    //CreateMenu();
    vTaskDelay(pdMS_TO_TICKS(500));
  }
    
}

/*
*/
void CreateMenu(){
   // displayPrint("Escolha uma opção:",4,50,50);
}

