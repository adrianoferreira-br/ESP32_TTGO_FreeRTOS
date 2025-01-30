/*/**********************************************************************************************
 *  File: state.cpp
 *  Description:  Algoritmo principal responsavel pela aplicação.
 *  date: 2025-01-14
/***********************************************************************************************/

#include "state.h"
#include "display.h"
#include <TFT_eSPI.h>
#include "Arduino.h"
#include "wifi_mqtt.h"
#include "main.h"
#include "EmonLib.h"                   // Include Emon Library
#include "time.h"
#include "constants.h"
//#include "extern_data.h"


// Pino do sensor YF-201
#define FLOW_SENSOR_PIN 37 // Conecte o fio de saída do sensor no GPIO 4


// Flow - Variáveis para contagem de pulsos
volatile uint16_t pulseCount = 0; // Contador de pulsos
unsigned long lastTime = 0;       // Marca de tempo para cálculo
float flowRate = 0.0;             // Vazão em L/min (ou outra unidade, depende da calibração)
float totalLiters = 0.0;          // Total de litros ou volume acumulado
const float calibrationfLOWFactor = 4.5; // Fator de calibração (varia de acordo com o sensor e meio)

//botão extra
const int buttom = 35;

//Medição de corrente
EnergyMonitor emon1;         // Instancia do sensor de corrente
int StateMachine = 0;
typedef enum {INIT_ST, LOW_CURRENT_ST, HIGH_CURRENT_ST} Estado;
Estado estadoAtual = INIT_ST;
const float calibrationCurrentFactor = CALIBRATION_CURRENT_FACTOR; // Fator de calibração (varia de acordo com o sensor e meio)
int Qnt = 0;  // Quantidade de vezes que a corrente passou de 1.5A
int i = 0;    // Contador de leituras eliminadas na inicialização
int NivelSinal = 0;




/**********************************************************************************************
 *     FUNÇÃO DE LOOP DA APLICAÇÃO
 */
void loop_state() {

  // Preenche informações referente a rede
  if (WiFi.status() == WL_CONNECTED) {      
      show_ip();         
  } else {
      tft.setTextColor(TFT_RED, TFT_BLACK);    
      tft.drawString("Disconnected     ", 0, 0, 2);  
      tft.drawString("                 ", 130, 0, 2);  
      
  }
  delay(1000);

  // Calcula a corrente e mostra no display
  calcula_corrente();
  
 
  // Verifica fluxo com sensor YF-S201
  //calcula_fluxo();
  // envia informação via protocolo mqtt
  //loop_mqqt();

   
 
}


/**********************************************************************************************
 *     FUNÇÃO DE SETUP E CONFIGURAÇÃO INICIAL DA APLICAÇÃO
 */
void init_state() {
  pinMode(buttom, INPUT);    
  // Corrente
  emon1.current(36, calibrationCurrentFactor);    //2.72         // Current: input pin, calibration.  
  
  // fluxo
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

    // Inicializa o tempo
  lastTime = millis(); 

}

// Interrupção para contar pulsos
void IRAM_ATTR pulseCounter() {
    pulseCount++;
}



/**********************************************************************************************
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

      if (Irms > 1.60) {        
        Qnt++;
        show_time(); 
        tft.setTextColor(TFT_WHITE, TFT_BLACK);        
        tft.drawString((String)Qnt,65, 85, 6);                
        estadoAtual = HIGH_CURRENT_ST;
      }      
    break;

    //Aguarda transição para baixa corrente
    case HIGH_CURRENT_ST:
    
      if (Irms < 1.10) {
        estadoAtual = LOW_CURRENT_ST;        
      }
    break;
  }

  
  // Mostra as corrente lida  
  Serial.print(Irms*230.0);	     // Apparent power
  Serial.print(" ");
  Serial.println(Irms);		       // Irms
  tft.drawString("Corrente", 0, 18, 4);  
  tft.drawString((String)Irms + " A", 10, 45, 4);  
  // emon1.serialprint();
  // Zera contador de leituras
  if (digitalRead(buttom) == LOW) {    
    Serial.println("Botão pressionado");   
    Qnt = 0;
  }

  tft.drawString("Tensao", 120, 18, 4);  
  tft.drawString((String)Irms + " V", 135, 45, 4);  
 

}




/**********************************************************************************************
 *     VERIFICA O FLUXO DO SENSOR YF-S201
 */
void calcula_fluxo(){

// Calcula a taxa de fluxo a cada segundo
unsigned long currentTime = millis();
    if (currentTime - lastTime >= 1000) { // A cada 1 segundo
        noInterrupts(); // Pausa a interrupção para calcular
        uint16_t currentPulseCount = pulseCount;
        pulseCount = 0; // Reseta o contador para o próximo intervalo
        interrupts(); // Retoma a interrupção

        // Taxa de fluxo em L/min
        flowRate = (currentPulseCount / calibrationfLOWFactor);

        // Volume total processado (em litros)
        totalLiters += (flowRate / 60.0);

        // Exibe as informações no monitor serial
        Serial.print("Taxa de fluxo: ");
        Serial.print(flowRate);
        Serial.println(" L/min");

        Serial.print("Volume total: ");
        Serial.print(totalLiters);
        Serial.println(" L");

        // Exibe as informações no display        
        tft.drawString("Fluxo(l/s):", 45, 75, 4);  
        tft.drawString((String)flowRate, 65, 100, 4); 

        // Atualiza o tempo
        lastTime = currentTime;
    }

// maquina de estado para detectar transição e realizar contagem
switch (estadoAtual) {

    // inicia a leitura, porém elimita 7 primeiras leituras que geralmente são erradas
    case INIT_ST:
        estadoAtual = LOW_CURRENT_ST;
    break;


    // Aguarda transição para alta corrente
    case LOW_CURRENT_ST:

      if (flowRate > 5) {        
        Qnt++;
        show_time(); 
        tft.drawString((String)Qnt,65, 130, 6);                
        estadoAtual = HIGH_CURRENT_ST;
      }      
    break;

    //Aguarda transição para baixa corrente
    case HIGH_CURRENT_ST:
    
      if (flowRate < 2) {
        estadoAtual = LOW_CURRENT_ST;        
      }
    break;
  }



}


/**********************************************************************************************
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

  tft.drawString(timeStr, 125, 173, 2);
 
}


/**********************************************************************************************
 *     MOSTRA O IP DA REDE NO DISPLAY
 */
void show_ip () {
  
  // Mostra o IP
  char ipStr[16];  
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

