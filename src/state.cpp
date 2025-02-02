/*/**********************************************************************************************
 *  File: state.cpp
 *  Description:  Algoritmo principal responsavel pela aplicação.
 *  date: 2025-01-14
/***********************************************************************************************/

#include "state.h"
#include "display.h"
#include <TFT_eSPI.h>
#include "Arduino.h"
#include "main.h"
#include "EmonLib.h"                 
#include "time.h"
#include "constants.h"
#include "extern_data.h"


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
//EnergyMonitor emonCurrent;         // Instancia do sensor de corrente
int StateMachine = 0;
typedef enum {INIT_ST, LOW_CURRENT_ST, HIGH_CURRENT_ST} Estado;
Estado estadoAtual = INIT_ST;
int Qnt = 0;  // Quantidade de vezes que a corrente passou de 1.5A
int i = 0;    // Contador de leituras eliminadas na inicialização


//Medição de tensão
//EnergyMonitor emonVoltage;
EnergyMonitor monitorEletricity;


/**********************************************************************************************
 *     FUNÇÃO DE SETUP E CONFIGURAÇÃO INICIAL DA APLICAÇÃO
 */
void init_state() {

  //Defini GPIO
  pinMode(buttom, INPUT);    

  // Corrente
  monitorEletricity.current(36,CALIBRATION_CURRENT_FACTOR);    //2.72         // Current: input pin, calibration.  

  // Tensão  
  monitorEletricity.voltage(39, CALIBRATION_VOLTAGE_FACTOR, 1); //PIN, 173, phase em relação a corrente(ex.1,7)
    
  // fluxo
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

  // Inicializa o tempo
  lastTime = millis(); 

  // Firebase
  firebase_setup();

}



/**********************************************************************************************
 *     FUNÇÃO DE LOOP DA APLICAÇÃO
 */
void loop_state() {  

  // Calcula a tensão e mostra no display
  calcula_tensao();

  // Verifica fluxo com sensor YF-S201
  //calcula_fluxo();

  //Firebase 
  // firebase_updateValues();   
  // Zera contador de leituras
  if (digitalRead(buttom) == LOW) {    
    Serial.println("Botão pressionado");   
    firebase_updateValues();
    Qnt = 0;
  } 

}




/**********************************************************************************************
 *     INTERRUPÇÃO PARA CONTAR PULSO DO FLUXO DE AR/AGUA
 */
void IRAM_ATTR pulseCounter() {
    pulseCount++;
}




/**********************************************************************************************
 *     VERIFICA O FLUXO DO SENSOR YF-S201
 */
void calcula_tensao(){

   int bar_color;
  // Atualize a leitura 
  monitorEletricity.calcVI(20, 2000);  //

  double Vrms = monitorEletricity.Vrms;  
  double Irms = monitorEletricity.Irms;
  double realPower = monitorEletricity.realPower;
  double apparentPower = Vrms * Irms;
  double powerFactor = realPower / apparentPower;

  // mostra a informação serial
  Serial.print("Energia-> Tensão RMS: ");    Serial.print(Vrms);  Serial.print(" V;   "); 
  Serial.print("Corrente RMS: ");            Serial.print(Irms);  Serial.print(" A;  ");
  Serial.print("Potência Real: ");           Serial.print(realPower);   Serial.print(" W;  ");
  Serial.print("Potência Aparente: ");       Serial.print(apparentPower);   Serial.print(" VA;  ");
  Serial.print("Fator de Potência: ");       Serial.println(powerFactor);

  // mostra informação no Display
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 
  tft.drawString("Tensao", 120, 18, 4);    
  tft.drawString((String)Vrms + " V   ", 135, 45, 4);  

  tft.drawString("Corrente", 5, 18, 4);
  tft.drawString((String)Irms + " A   ", 5, 45, 4);

  //tft.drawString("Pot. Real", 5, 70, 4);
  //tft.drawString((String)realPower + " W   ", 5, 95, 4);

  //tft.drawString("Pot. Aparente", 5, 120, 4);
  tft.drawString((String)apparentPower + " va   ", 60, 77, 4);

  //tft.drawString("Fator Pot.", 5, 170, 4);
  //tft.drawString((String)powerFactor, 5, 195, 4);

  if (realPower < LIMIAR_INFERIOR) {
    bar_color = TFT_RED;
  } else if (realPower >= LIMIAR_INFERIOR && realPower < LIMIAR_SUPERIOR) {
    bar_color = TFT_ORANGE;
  } else if (realPower >= LIMIAR_SUPERIOR) {
    bar_color = TFT_BLUE;
  } 
  
  if (realPower >= LIMITE_MAX) {  //Evita ultrapassar barra grafica
    realPower = LIMITE_MAX;
  }
  
  graficoBarra(1,105,180,132,realPower,LIMITE_MAX,bar_color);    // x, y, largura, altura, valor, valorMaximo, cor)
  

// maquina de estado para detectar transição e realizar contagem
  switch (estadoAtual) {

    // inicia a leitura, porém elimita 7 primeiras leituras que geralmente são erradas
    case INIT_ST:
        estadoAtual = HIGH_CURRENT_ST;
    break;

    // Aguarda transição para alta corrente
    case LOW_CURRENT_ST:

      if (realPower > LIMIAR_SUPERIOR) {        
        Qnt++;
        //show_time(); 
        tft.drawString((String)Qnt,190, 110, 4);                
        estadoAtual = HIGH_CURRENT_ST;
      }      
    break;

    //Aguarda transição para baixa corrente
    case HIGH_CURRENT_ST:
    
      if (realPower < LIMIAR_INFERIOR) {
        estadoAtual = LOW_CURRENT_ST;        
      }
    break;
  }
  tft.setTextColor(TFT_RED, TFT_BLACK);        
  tft.drawString((String)LIMIAR_INFERIOR, 1, 87, 2);
  tft.drawString((String)LIMIAR_SUPERIOR, 180, 87, 2);
  tft.drawString("|", (LIMIAR_INFERIOR * 172) / LIMITE_MAX, 100, 2);
  tft.drawString("|", (LIMIAR_SUPERIOR * 172) / LIMITE_MAX, 100, 2);

  tft.drawString((String)CALIBRATION_CURRENT_FACTOR, 1, 67, 2);
  tft.drawString((String)CALIBRATION_VOLTAGE_FACTOR, 180, 67, 2);


  tft.setTextColor(TFT_WHITE, TFT_BLACK);        



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
  // Mostra o horário
  Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");   
  strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

  tft.drawString(timeStr, 125, 173, 2);
 
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
      Serial.print("Taxa de fluxo: ");    Serial.print(flowRate);    Serial.println(" L/min");
      Serial.print("Volume total: ");     Serial.print(totalLiters); Serial.println(" L");

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