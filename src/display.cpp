#include <TFT_eSPI.h>
#include "display.h"


TFT_eSPI tft = TFT_eSPI();


void init_display()
{

    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.drawString("Hello Worldd!", 0, 0, 4);  
}



void displayPrint(char* str, int qnt, int x, int y)
{
     tft.drawString(str, x, y, 4);  //display 0-135 x 0-239px OLED color
}
