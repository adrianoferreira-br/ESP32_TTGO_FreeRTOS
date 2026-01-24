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
    #ifdef LILYGO_T_DISPLAY_S3
        // DEBUG: Inicialização com logs detalhados
        Serial.begin(115200);
        delay(100);
        Serial.println("\n=== INIT DISPLAY LILYGO S3 (PARALLEL 8-BIT) ===");
        
        // Liga backlight
        pinMode(38, OUTPUT);
        digitalWrite(38, HIGH);
        Serial.println("Backlight ON (GPIO 38)");
        delay(100);
        
        // Inicializa display (interface paralela não precisa de SPI.begin())
        Serial.println("Chamando tft.init()...");
        tft.init();
        delay(200);
        Serial.println("tft.init() OK");
        
        tft.setRotation(1);
        Serial.print("Rotacao 1 - Resolucao: ");
        Serial.print(tft.width());
        Serial.print(" x ");
        Serial.println(tft.height());
        
        tft.setTextFont(4);
        tft.setTextSize(1);
        
        // Teste visual RGB        
        tft.fillScreen(TFT_RED);
        delay(500);
        tft.fillScreen(TFT_GREEN);
        delay(500);
        tft.fillScreen(TFT_BLUE);
        delay(500);
        tft.fillScreen(TFT_WHITE);
        delay(500);
        tft.fillScreen(TFT_BLACK);
        
        /*
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("LilyGo T-Display S3", 10, 10, 4);
        */
        
        
        Serial.println("=== INIT COMPLETO ===");
        delay(3000);
    #else
        // TTGO T-Display original
        pinMode(TFT_BL, OUTPUT);
        digitalWrite(TFT_BL, HIGH);
        
        tft.init();
        tft.setRotation(1);
        tft.setTextFont(4);
        tft.setTextSize(1);
        tft.fillScreen(TFT_BLACK);         
        tft.setTextDatum(TL_DATUM);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
    #endif
    
    tft.fillScreen(TFT_BLACK);
    
    #ifdef LILYGO_T_DISPLAY_S3
        int logoX = (tft.width() - 192) / 2;
        int logoY = (tft.height() - 51) / 2;
        tft.pushImage(logoX, logoY, 192, 51, (uint16_t *)INDX4_240x135);
    #else
        tft.pushImage(30, 40, 192, 51, (uint16_t *)INDX4_240x135);
    #endif
    
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
    
    #ifdef LILYGO_T_DISPLAY_S3
        // Layout para LilyGo S3 (320x170)
        tft.drawString("Versao: " + String(VERSION), tft.width() / 2, 40, 4);
        tft.drawString("LilyGo T-Display S3", tft.width() / 2, 85, 4);
        tft.drawString("Equip: " + String(DISPOSITIVO_ID), tft.width() / 2, tft.height() - 40, 2);
    #else
        // Layout para TTGO (240x135)
        tft.drawString("Versao: " + String(VERSION), tft.width() / 2, 30, 4);
        tft.drawString("Equip: " + String(DISPOSITIVO_ID), tft.width() / 2, tft.height() - 30, 2);
    #endif
    
    delay(2000);
    tft.setTextDatum(TL_DATUM); // Retorna para o padrão (top-left)
    tft.fillScreen(TFT_BLACK);
}




/*****************************************************************************************
*  Description: Show distance on the display
*/
void show_distancia(float dist) {
    #ifdef LILYGO_T_DISPLAY_S3
        // Layout para LilyGo S3 (320x170) - ajustado proporcionalmente
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("   " + String(dist, 1), 160, 25, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("cm", 260, 25, 4);    
        graficoBarra(5, 20, 80, 140, level_min - dist, level_min - level_max, TFT_BLUE, false);
        tft.drawString(String(level_max), 90, 20, 2);
        tft.drawString(String(level_min), 90, 145, 2);
    #else
        // Layout para TTGO (240x135)
        tft.setTextColor(TFT_YELLOW, TFT_BLACK);
        tft.drawString("   " + String(dist, 1), 120, 25, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("cm", 200, 25, 4);    
        graficoBarra(3, 20, 60, 113, level_min - dist, level_min - level_max, TFT_BLUE, false);
        tft.drawString(String(level_max), 70, 20, 2);
        tft.drawString(String(level_min), 70, 120, 2);
    #endif
}



/*
*  Description: Show percentual disponível no reservatório
*/
void show_percentual_reservatorio(float percentual) {
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%.1f%%", percentual);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    
    #ifdef LILYGO_T_DISPLAY_S3
        tft.drawString(buffer, 160, 140, 4);
    #else
        tft.drawString(buffer, 145, 115, 4);
    #endif
}



/*
 *  Description: Show temperature on the display
*/
void show_temperature(float temp) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    
    #ifdef LILYGO_T_DISPLAY_S3
        tft.drawString(" t: " + String(temp, 1), 160, 60, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("C", 260, 60, 4);
    #else
        tft.drawString(" t: " + String(temp, 1), 120, 55, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("C", 200, 55, 4);
    #endif
}

/*
 *  Description: Show humidity on the display
*/
void show_humidity(float hum) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    
    #ifdef LILYGO_T_DISPLAY_S3
        tft.drawString(" h:" + String(hum, 1), 160, 95, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("%", 260, 95, 4);
    #else
        tft.drawString(" h:" + String(hum, 1), 120, 85, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("%", 200, 85, 4);
    #endif
}

/*
 *  Description: Show battery voltage on the display
*/
void show_battery_voltage(float voltage) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    
    #ifdef LILYGO_T_DISPLAY_S3
        tft.drawString("   " + String(voltage, 2), 160, 130, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("V", 260, 130, 4);
    #else
        tft.drawString("   " + String(voltage, 2), 120, 115, 4);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.drawString("V", 200, 115, 4);
    #endif
}

/*
 *  Description: Show the number of presses on the display - Sensor 1
*/
void show_batidas(int batidas) {
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    #ifdef LILYGO_T_DISPLAY_S3
        tft.drawString(" S1:" + String(batidas) + "  ", 2, 50, 6);
    #else
        tft.drawString(" S1:" + String(batidas) + "  ", 2, 30, 4);
    #endif
}

/*
 *  Description: Show the number of presses on the display - Sensor 2
*/
void show_batidas_sensor2(int batidas) {
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    #ifdef LILYGO_T_DISPLAY_S3
        tft.drawString(" S2:" + String(batidas) + "  ", 2, 100, 6);
    #else
        tft.drawString(" S2:" + String(batidas) + "  ", 2, 60, 4);
    #endif
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
  String connectedSSID = WiFi.SSID();

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

/**********************************************************************************************
 *     MOSTRA O STATUS DO SENSOR DE PORTA NO DISPLAY
 *     
 *     @param sensor_id: ID do sensor (1 ou 2)
 *     @param value: valor do sensor (0 = fechado/azul, 1 = aberto/vermelho)
 *     
 *     Desenha um quadrado colorido com o texto D1 ou D2
 *     - Posição D1: canto superior direito (200, 20)
 *     - Posição D2: canto superior direito (200, 60)
 */
void show_door_sensor(int sensor_id, int value) {
    // Define posições baseadas no sensor_id
    int x = 280;  // Posição X (próximo ao canto direito)
    int y = (sensor_id == 1) ? 20 : 90;  // D1 no topo, D2 abaixo
    
    int box_width = 40;
    int box_height = 70;
    
    // Escolhe cor baseada no valor
    // 0 (LOW) = Fechado = Azul
    // 1 (HIGH) = Aberto = Vermelho
    uint16_t box_color = (value == 0) ? TFT_BLUE : TFT_RED;
    uint16_t text_color = TFT_WHITE;
    
    // Desenha o quadrado preenchido
    tft.fillRect(x, y, box_width, box_height, box_color);
    
    // Desenha a borda (opcional, para destaque)
    tft.drawRect(x, y, box_width, box_height, TFT_WHITE);
    
    // Desenha o texto "D1" ou "D2" centralizado
    tft.setTextColor(text_color, box_color);
    tft.setTextDatum(MC_DATUM); // Middle Center
    
    String label = "D" + String(sensor_id);
    tft.drawString(label, x + box_width/2, y + box_height/2, 4);
    
    // Restaura configurações padrão
    tft.setTextDatum(TL_DATUM); // Top Left
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}

/*
 *  Description: Show DS18B20 sensor temperature reading on display
 *  @param sensor_id: Index of sensor on OneWire bus (0 for first sensor)
 *  @param value: Temperature in Celsius
 */
void show_sensor_ds18b20(int sensor_id, float value) {
    // Posição no display (ajustada conforme sensor_id)
    int x = 20;
    int y = 40 + (sensor_id * 60);  // Espaço de 60 pixels entre sensores
    
    // Cor: Verde se temperatura válida (> -100°C), Vermelho se erro
    uint16_t text_color = (value > -100.0) ? TFT_GREEN : TFT_RED;
    
    tft.setTextColor(text_color, TFT_BLACK);
    
    // Formata string: "T0: 25.50 C" (sensor_id 0)
    String display_text = "T" + String(sensor_id+1) + ": ";
    
    if (value > -100.0) {
        display_text += String(value, 2) + " C";  
    } else {
        display_text += "ERROR  ";
    }
    
    tft.drawString(display_text, x, y, 6);
    
    // Restaura cor padrão
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
}