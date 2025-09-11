/*  File: display.cpp
 *  Description: Controller Display and extra functions
 *  date: 2025-01-14
 */

#include <display.h>
#include <WiFi.h>
#include <Arduino.h>
#include "constants.h"




/* Informações úteirs (Display TFT, ST7789 (ou ST7735?)):
 * Resolução: 240x135px   1.14"
 * Fontes: 2, 4, 6, 7
 * Cores: TFT_BLACK, TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_CYAN, TFT_MAGENTA, TFT_YELLOW, TFT_ORANGE, TFT_PINK, TFT_GREY, TFT_DARKGREY, TFT_LIGHTGREY, TFT_BROWN, TFT_DARKGREEN, TFT_DARKCYAN, TFT_PURPLE, TFT_NAVY, TFT_DARKRED, TFT_MAROON, TFT_OLIVE, TFT_LIGHTGREY, TFT_DARKGREY, TFT_BLUE, TFT_GREEN, TFT_RED, TFT_WHITE, TFT_BLACK
 * Rotação: 0, 1, 2, 3
 * Tipo de fonte: A4, A5, A6, A7
 * 
 *  tft.drawString(voltage,  tft.width() / 2, tft.height() / 2 );
 */


TFT_eSPI tft = TFT_eSPI();



/*  init_display
 *  Description: Initialize the display, set basic items like: rotation, font, background and text color.
*/
void init_display()
{ 
    tft.init();
    tft.setRotation(3);
    tft.setTextFont(4);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);         
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);        
    int origemX = tft.width();    
    int origemY = tft.height();    
    tft.drawString(((String)origemX) + " x " + ((String)origemY),90,50,2);     //string, x, y, font  : Fonte 2, 4, 6, 7
    delay(500);  
    //showBootInfo(); // Exibe informações de boot (versão e nome do equipamento)    
    tft.fillScreen(TFT_RED);         
    delay(500);  
    tft.fillScreen(TFT_GREEN);         
    delay(500);  
    tft.fillScreen(TFT_BLUE);         
    delay(500);  
    tft.fillScreen(TFT_ORANGE);     
    delay(500);        
    tft.fillScreen(TFT_BLACK);    
    tft.drawString("  Procurando rede...", 2, 10, 4); //string, x, y, font
    tft.drawString("Versao:   " + String(VERSION), 2, 55, 4); //string, x, y, font
    tft.drawString("Equip:  " + String(NOME_EQUIPAMENTO), 2, 100, 4); //string, x, y, font
    //graficoBarra(1,105,180,132,50,132,TFT_BLUE);    // x, y, largura, altura, valor, valorMaximo, cor)
   
 
  // tft.fillCircle(120, 120, 10, TFT_YELLOW);
  // drawGauge(35); // Exemplo com um valor de 50
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



void graficoBarra(int x, int y, int largura, int altura, int valor, int valorMaximo, int cor) {
  
    // Verificação de divisão por zero
    if (valorMaximo == 0) {
        Serial.println("Erro: valorMaximo é zero.");
        return;
    }    

    tft.drawRect(x, y, largura, altura, TFT_WHITE);
    tft.fillRect(x, y, largura, altura, TFT_WHITE);
    tft.fillRect(x, y, (valor * largura) / valorMaximo, altura, cor);
}

  

/* drawGauge
 *  Description: Draw a gauge on the display
 *  @param value: value to be displayed on the gauge
*/
void drawGauge(int value) {
  int centerX = 45;//tft.width() / 2;
  int centerY = 60;//tft.height() / 2;
  int radius = 30; // Raio do mostrador

  // Desenhe o mostrador circular
  tft.drawCircle(centerX, centerY, radius, TFT_WHITE);

  // Desenhe as marcas no mostrador
  for (int i = 0; i <= 100; i += 10) {
    float angle = (i - 50) * 2.7; // Mapeie os valores de 0 a 100 para ângulos de -135 a 135 graus
    int x0 = centerX + radius * cos(angle * PI / 180);
    int y0 = centerY + radius * sin(angle * PI / 180);
    int x1 = centerX + (radius - 10) * cos(angle * PI / 180);
    int y1 = centerY + (radius - 10) * sin(angle * PI / 180);
    tft.drawLine(x0, y0, x1, y1, TFT_WHITE);
  }

  // Desenhe o ponteiro
  float angle = (value - 50) * 2.7;
  int pointerLength = radius - 15;
  int pointerX = centerX + pointerLength * cos(angle * PI / 180);
  int pointerY = centerY + pointerLength * sin(angle * PI / 180);
  tft.drawLine(centerX, centerY, pointerX, pointerY, TFT_RED);
}


void showBootInfo() {        
    
    tft.setTextColor(TFT_WHITE, TFT_BLACK);   

    // Exibe VERSION na parte superior
    int16_t topY = 10;
    tft.drawString(VERSION, (tft.width() / 2)-4, topY);

    // Exibe NOME_EQUIPAMENTO na parte inferior
    int16_t bottomY = tft.height() - 20;
    tft.drawString(NOME_EQUIPAMENTO, (tft.width() / 2)-4, bottomY-2);

    delay(3000); // Aguarda 3 segundos
    
}


void show_temperature(float temp) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("   " + String(temp, 1), 10, 50, 7);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("C", 170, 75, 4);
}