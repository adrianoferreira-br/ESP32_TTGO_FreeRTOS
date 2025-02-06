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


//Medição de corrente
//EnergyMonitor emonCurrent;         // Instancia do sensor de corrente
int StateMachine = 0;
typedef enum {INIT_ST, LOW_CURRENT_ST, HIGH_CURRENT_ST} Estado;
Estado estadoAtual = INIT_ST;
int Qnt = 0;  // Quantidade de vezes que a corrente passou de 1.5A
int i = 0;    // Contador de leituras eliminadas na inicialização

const int BOTAO_35 = 35;


//Medição de tensão
//EnergyMonitor emonVoltage;
EnergyMonitor monitorEletricity;

//Potência
int tipoPotencia = 0;

// Variável global volátil para sinalizar a interrupção
volatile bool buttonPressed = false;

/**********************************************************************************************
 *     FUNÇÃO DE SETUP E CONFIGURAÇÃO INICIAL DA APLICAÇÃO
 */
void init_state() {

  //Defini GPIO
  pinMode(BOTAO_35, INPUT);    

  // Corrente
  monitorEletricity.current(36, CALIBRATION_CURRENT_FACTOR);    //2.72         // Current: input pin, calibration.  

  // Tensão  
  monitorEletricity.voltage(39, CALIBRATION_VOLTAGE_FACTOR, 1); //PIN, 173, phase em relação a corrente(ex.1,7)
    
  // fluxo
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

  // Inicializa o tempo
  lastTime = millis(); 

  // Firebase
  firebase_setup();

  // Configura a interrupção para o botão
  attachInterrupt(digitalPinToInterrupt(BOTAO_35), InterruptionPino35, FALLING);

}



/**********************************************************************************************
 *     FUNÇÃO DE LOOP DA APLICAÇÃO
 */
void loop_state() {  

  // Verifica se o botão foi pressionado
  if (buttonPressed) {
    buttonPressed = false;  // Reseta a variável de estado do botão
    Serial.println("Botão pressionado");
    firebase_updateValues();
  }

  // Calcula a tensão e mostra no display
  calcula_tensao();

  // Verifica fluxo com sensor YF-S201
  //calcula_fluxo();

 
}


/**********************************************************************************************
 *     INTERRUPÇÃO PARA ATUALIZAR PARAMETROS DO FIREBASE
 */
void IRAM_ATTR InterruptionPino35() {  
  buttonPressed = true;  // Sinaliza que o botão foi pressionado
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

  double realPower;
  double apparentPower;  
  double powerFactor;
  double potencia;
  
  
  double Vrms = monitorEletricity.Vrms;  
  double Irms = monitorEletricity.Irms;

  
  //Verifica se a corrente está dentro dos limites aceitáveis
  if (Irms > 10/*MAX_CURRENT_LIMIT*/) {
    Serial.println("Erro: Corrente muito elevada!");
    return;
  }

  realPower = abs(monitorEletricity.realPower);
  apparentPower = Vrms * Irms;
  if (apparentPower != 0)
    powerFactor = realPower / apparentPower;

  
  // mostra informação no Display e serial
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 

  //Mostra Tensão
  tft.drawString("Tensao", 120, 18, 4);    
  tft.drawString((String)Vrms + " V   ", 135, 45, 4);  
  Serial.print("Tensão RMS: ");       Serial.print(Vrms);  Serial.print(" V;   "); 
  
  //Mostra Corrente
  tft.drawString("Corrente", 5, 18, 4);
  tft.drawString((String)Irms + " A   ", 5, 45, 4);  
  Serial.print("Corrente RMS: ");     Serial.print(Irms);  Serial.print(" A;  ");

  //Mostra Potencia
  switch (tipoPotencia) {
    case 0:  //realPower
      //tft.drawString("Pot. Real", 5, 70, 4);
      tft.drawString((String)realPower + " w      ", 55, 80, 4);
      potencia = realPower;
    break;

    case 1:  //apparentPower
      //tft.drawString("Pot. Aparente", 5, 70, 4);
      tft.drawString((String)apparentPower + " VA      ", 55, 80, 4);
      potencia = apparentPower;
    break;

    case 2:  // powerFactor
      //tft.drawString("Fator Pot.", 5, 70, 4);
      tft.drawString((String)powerFactor + "       ", 55, 80, 4);
      potencia = powerFactor;
    break; 

  }    

// mostra na serial  
  Serial.print("Potência Real: ");           Serial.print(realPower);       Serial.print(" W;  ");
  Serial.print("Potência Aparente: ");       Serial.print(apparentPower);   Serial.print(" VA;  ");
  Serial.print("Fator de Potência: ");       Serial.println(powerFactor);

// grafico de barra
  if (potencia < LIMIAR_INFERIOR) {
    bar_color = TFT_RED;
  } else if (potencia >= LIMIAR_INFERIOR && potencia < LIMIAR_SUPERIOR) {
    bar_color = TFT_ORANGE;
  } else if (potencia >= LIMIAR_SUPERIOR) {
    bar_color = TFT_BLUE;
  } 
  
  if (potencia >= LIMITE_MAX) {  //Evita ultrapassar barra grafica
    potencia = LIMITE_MAX;
  }
  
  graficoBarra(1,105,180,132,potencia,LIMITE_MAX,bar_color);    // x, y, largura, altura, valor, valorMaximo, cor)
  
  

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