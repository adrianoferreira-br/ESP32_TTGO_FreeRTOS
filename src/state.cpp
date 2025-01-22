/*  File: state.cpp
 *  Description:  Algoritmo principal responsavel pela aplicação.
 *  date: 2025-01-14
 */
#include "state.h"
#include "display.h"
#include <TFT_eSPI.h>
#include "Arduino.h"
#include "wifi_mqtt.h"
#include "main.h"
#include "EmonLib.h"                   // Include Emon Library
#include "time.h"


const int buttom = 35;


TFT_eSPI tft2 = TFT_eSPI();  // Instancia do display
EnergyMonitor emon1;         // Instancia do sensor de corrente
int StateMachine = 0;
typedef enum {INIT_ST, LOW_CURRENT_ST, HIGH_CURRENT_ST} Estado;
Estado estadoAtual = INIT_ST;

int Qnt = 0;  // Quantidade de vezes que a corrente passou de 1.5A
int i = 0;    // Contador de leituras eliminadas na inicialização




/*
 *     FUNÇÃO DE LOOP DA APLICAÇÃO
 */
void loop_state() {

  // Preenche informações referente a rede
  if (WiFi.status() == WL_CONNECTED) {
      //tft2.drawString("Connected        ", 45, 54, 2);      
      show_ip();
  } else 
      tft2.drawString("Disconnected     ", 45, 54, 2);  
  delay(1000);

  // Calcula a corrente e mostra no display
  calcula_corrente();

  // Verifica fluxo com sensor YF-S201
  //calcula_fluxo();
 
}


/*
 *     FUNÇÃO DE SETUP E CONFIGURAÇÃO INICIAL DA APLICAÇÃO
 */
void init_state() {
  pinMode(buttom, INPUT);
  //tft2.drawString("state", 3, 50, 3);
  delay(5000);
  emon1.current(36, 2.72);             // Current: input pin, calibration.  
  
}



/*
 *     VERIFICA O FLUXO DO SENSOR YF-S201
 */
void calcula_fluxo(){

// Calcula a taxa de fluxo a cada segundo
}


/*
 *     MOSTRA INFORMAÇÃO DE TEMPO DIA/MES/ANO  HORA:MINUTO:SEGUNDO
 */
void show_time() {

  char timeStr[20];  // Used to store time string
  struct tm timeinfo;

  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Erro ao obter tempo!");
    return;
  }

  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");   
//  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
  strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);

  tft2.drawString(timeStr, 125, 173, 2);
 
}


/*
 *     MOSTRA O IP DA REDE NO DISPLAY
 */
void show_ip () {

  char ipStr[16];  
  IPAddress ip = WiFi.localIP();
  sprintf(ipStr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  tft2.drawString(ipStr, 45, 54, 2); 

}


/*
 *     CALCULA E MOSTRA O VALOR DE CARRENTE NO DISPLAY
 */
void calcula_corrente(){

 //Captura a corrente  
  double Irms = emon1.calcIrms(1480);  // Calculate Irms only

  // Maquina de estado da aplicação, implementa uma histerese
  switch (estadoAtual) {

    // inicia a leitura, porém elimita 7 primeiras leituras que geralmente são erradas
    case INIT_ST:

      if (i < 7)        
        i++;
      else       
        estadoAtual = LOW_CURRENT_ST;

    break;


    // Aguarda transição para alta corrente
    case LOW_CURRENT_ST:

      if (Irms > 1.50) {        
        Qnt++;
        show_time(); 
        tft2.drawString((String)Qnt,65, 130, 6);        
        estadoAtual = HIGH_CURRENT_ST;
      }      
    break;

    //Aguarda transição para baixa corrente
    case HIGH_CURRENT_ST:
    
      if (Irms < 1.20) {
        estadoAtual = LOW_CURRENT_ST;        
      }
    break;
  }

  
  // Mostra as corrente lida  
  Serial.print(Irms*230.0);	     // Apparent power
  Serial.print(" ");
  Serial.println(Irms);		       // Irms
  tft2.drawString("Corrente:", 45, 75, 4);  
  tft2.drawString((String)Irms, 65, 100, 4);  
  // emon1.serialprint();
  // Zera contador de leituras
  if (digitalRead(buttom) == LOW) {    
    Serial.println("Botão pressionado");   
    Qnt = 0;
  }


}