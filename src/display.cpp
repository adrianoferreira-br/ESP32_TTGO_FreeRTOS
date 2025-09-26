/*  File: display.cpp
 *  Description: Controller Display and extra functions
 *  date: 2025-01-14
 */

#include <display.h>
#include <WiFi.h>
#include <Arduino.h>
#include "constants.h"
#include "main.h"

TFT_eSPI tft = TFT_eSPI();



/*  init_display
 *  Description: Initialize the display, set basic items like: rotation, font, background and text color.
*/
void init_display()
{ 
    tft.init();
    tft.setRotation(1);  // 3- landpage com conector a esquerda
    tft.setTextFont(4);
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);         
    tft.setTextDatum(TL_DATUM);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);        
    // Mostra resolução para debug (opcional)
    /*tft.drawString(String(tft.width()) + " x " + String(tft.height()), 90, 50, 2);
    delay(500);
    // Animação de boot (opcional)
    uint16_t bootColors[] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_ORANGE};
    for (uint8_t i = 0; i < 4; i++) {
        tft.fillScreen(bootColors[i]);
        delay(300);
    }*/
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(30, 40, 192, 51, (uint16_t *)INDX4_240x135); // Logo Presto
    delay(3000);
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




/*
 *  Description: Draw a bar graph on the display
 *  @param x, y: posição inicial
 *  @param largura, altura: dimensões do gráfico
 *  @param valor: valor atual
 *  @param valorMaximo: valor máximo
 *  @param cor: cor da barra
 *  @param horizontal: true = horizontal, false = vertical
 */
void graficoBarra(int x, int y, int largura, int altura, int valor, int valorMaximo, int cor, bool horizontal = true) {
    if (valorMaximo == 0) {
        Serial.println("Erro: valorMaximo é zero.");
        return;
    }
    tft.drawRect(x, y, largura, altura, TFT_WHITE);
    tft.fillRect(x, y, largura, altura, TFT_WHITE); // Fundo branco

    if (horizontal) {
        int barra = (valor * largura) / valorMaximo;
        tft.fillRect(x, y, barra, altura, cor);
    } else {
        int barra = (valor * altura) / valorMaximo;
        tft.fillRect(x, y + (altura - barra), largura, barra, cor);
    }
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



/*
 *  Description: Show boot information on the display
 */
void showBootInfo() {        
    
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM); // Centraliza texto
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    // Exibe versão na parte superior centralizada
    tft.drawString("Versao: " + String(VERSION), tft.width() / 2, 30, 4);
    // Exibe nome do equipamento na parte inferior centralizada
    tft.drawString("Equip: " + String(NOME_EQUIPAMENTO), tft.width() / 2, tft.height() - 30, 4);
    delay(2000); // Aguarda 2 segundos
    tft.setTextDatum(TL_DATUM); // Retorna para o padrão (top-left)
}




/*
*  Description: Show distance on the display
*/
void show_distancia(float dist) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("   " + String(dist, 1), 120, 25, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("cm", 200, 25, 4);    
    graficoBarra(3, 20, 60, 113,  length_max - dist, length_max, TFT_BLUE, false); // Exemplo de uso do gráfico de barras
    tft.drawString("0", 70, 20, 2); // Exibe medida inicial do reservatório
    tft.drawString(String(length_max), 70, 120, 2); // Exibe altura máxima do ponto inicial até o modo sem agua
    tft.setTextColor(TFT_BLACK, TFT_WHITE);
    tft.drawString(String(100-(dist/length_max * 100))+"%", 12, 70, 2);
}


/*
 *  Description: Show temperature on the display
*/
void show_temperature(float temp) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("   " + String(temp, 1), 120, 55, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("C", 200, 55, 4);
}

/*
 *  Description: Show humidity on the display
*/
void show_humidity(float hum) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("   " + String(hum, 1), 120, 85, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("%", 200, 85, 4);
}

/*
 *  Description: Show battery voltage on the display
*/
void show_battery_voltage(float voltage) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("   " + String(voltage, 2), 120, 115, 4);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString("V", 200, 115, 4);
}

/*
 *  Description: Show the number of presses on the display
*/
void show_batidas(int batidas) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("   " + String(batidas), 55, 50, 6);
}


/*
 *  Description: Show the current time on the display
*/
void show_time(char* timeStr) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);        
   // tft.drawString("          ", 120, 105, 4);    
    tft.drawString("   " + String(timeStr), 127, 105, 4);    
}


/**********************************************************************************************
 *     MOSTRA O IP DA REDE NO DISPLAY
 */
void show_ip () {
  
  // Mostra o IP
  char ipStr[16];  
  int NivelSinal = 0;
  IPAddress ip = WiFi.localIP();


  sprintf(ipStr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);    
  tft.drawString(ipStr, 0, 0, 2); 

  // Mostra o nível do sinal
   NivelSinal = WiFi.RSSI();
      //Serial.print("Nível Sinal:" + (String)NivelSinal + "dBm");
      if (NivelSinal >= -50) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);    
        tft.drawString("Otimo   ", 150,0 , 2);
      } else if (NivelSinal >= -70) {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);    
        tft.drawString("Bom    ", 150, 0, 2);
      } else if (NivelSinal >= -80) {
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);    
        tft.drawString("Ruim   ", 150, 0, 2);
      } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);    
        tft.drawString("Pessimo", 150, 0, 2);
      }
      tft.drawString((String)NivelSinal, 205, 1, 4);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);    
}
