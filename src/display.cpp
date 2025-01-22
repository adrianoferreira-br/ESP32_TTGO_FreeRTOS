/*  File: display.cpp
 *  Description: Controller Display and extra functions
 *  date: 2025-01-14
 */

#include <display.h>
#include <WiFi.h>




TFT_eSPI tft = TFT_eSPI();


/*  init_display
 *  Description: Initialize the display, set basic items like: rotation, font, background and text color.
*/
void init_display()
{

    tft.init();
    tft.setRotation(0);
    tft.setTextFont(A4);
    tft.fillScreen(TFT_BLACK);         
    tft.drawString("ABCDE",50,50,2);     //string, x, y, font  : Fonte 2, 4, 6, 7
    delay(1000);  
}


/* displayPrint
 *  Description: Print a string on the display
 *  @param str: string to be printed
 *  @param qnt: quantity of characters
 *  @param x: x position
 *  @param y: y position
*/
void displayPrint(char* str, int qnt, int x, int y)
{
    // tft.drawString(str, x, y, 4);  //display 0-135 x 0-239px OLED color
}
