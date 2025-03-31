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
#include "wifi_mqtt.h"
//#include "sqldatabase.h"


// Pino do sensor YF-201
#define FLOW_SENSOR_PIN 37 // Conecte o fio de saída do sensor no GPIO 4

// Pino do sensor reflexivo
#define REFLEX_SENSOR_PIN 38 

// Pino do sensor reflexivo
#define BATIDA_PIN 12 
int qnt_batidas_prensa = 0;
volatile bool batida_prensa = false;


// Flow - Variáveis para contagem de pulsos
volatile uint16_t pulseCount = 0; // Contador de pulsos
unsigned long lastTime = 0;       // Marca de tempo para cálculo
float flowRate = 0.0;             // Vazão em L/min (ou outra unidade, depende da calibração)
float totalLiters = 0.0;          // Total de litros ou volume acumulado
const float calibrationfLOWFactor = 4.5; // Fator de calibração (varia de acordo com o sensor e meio)


//Medição de corrente e tensão
EnergyMonitor monitorEletricity;
int StateMachine = 0;
typedef enum {INIT_ST, LOW_CURRENT_ST, HIGH_CURRENT_ST} Estado;
Estado estadoAtual = INIT_ST;
int Qnt = 0;  // Quantidade de vezes que a corrente passou de 1.5A
int i = 0;    // Contador de leituras eliminadas na inicialização

//Verificação de reflexo
volatile bool reflexSensorTriggered = false;

// botão da placa
const int BOTAO_35 = 35;


time_t before = 0;

//Potência
int tipoPotencia = 0;

// Variável global volátil para sinalizar a interrupção
volatile bool buttonPressed = false;

/**********************************************************************************************
 *     FUNÇÃO DE SETUP E CONFIGURAÇÃO INICIAL DA APLICAÇÃO
 */
void init_state() {

  // Corrente
  monitorEletricity.current(36, CALIBRATION_CURRENT_FACTOR);    //2.72         // Current: input pin, calibration.  

  // Tensão  
  monitorEletricity.voltage(39, CALIBRATION_VOLTAGE_FACTOR, 1); //PIN, 173, phase em relação a corrente(ex.1,7)

  // Sensor reflexivo
  pinMode(REFLEX_SENSOR_PIN, INPUT_PULLUP);  
  attachInterrupt(digitalPinToInterrupt(REFLEX_SENSOR_PIN), InterruptionPino38, FALLING);
    
  // fluxo
  pinMode(FLOW_SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_PIN), pulseCounter, RISING);

  // Inicializa o tempo
  lastTime = millis(); 

  // Firebase
  firebase_setup();


   // Inicializa horario do ntp com fuso -3
   configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
}


/**********************************************************************************************
 *     FUNÇÃO DE SETUP E CONFIGURAÇÃO INICIAL DA APLICAÇÃO
 */
void setup_batidas_prensa() {

  // Configura a interrupção para o botão
  attachInterrupt(digitalPinToInterrupt(BATIDA_PIN), InterruptionPino12, RISING);

  //Defini GPIO
  pinMode(BATIDA_PIN, INPUT_PULLUP); // Configura o pino como entrada com pull-up interno
  
  // Inicializa horario do ntp com fuso -3
  configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

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
    send_data_firestore();
  }

  // Calcula a tensão e mostra no display
  calcula_tensao();

  // Calcula a tensão e mostra no display
  //verifica_batida_prensa();

  // Verifica fluxo com sensor YF-S201
  //calcula_fluxo();

  //Responsável por verificar o sensor reflexivo
  if (reflexSensorTriggered){
    reflexSensorTriggered = false;
    Serial.println("Sensor reflexivo ativado!");
    Qnt++;
  }

 
}


/**********************************************************************************************
 *     INTERRUPÇÃO PINO 35 PARA ATUALIZAR PARAMETROS DO FIREBASE
 */
void IRAM_ATTR InterruptionPino35() {  
  buttonPressed = true;  // Sinaliza que o botão foi pressionado  
}

/**********************************************************************************************
 *     INTERRUPÇÃO PINO 38 PARA DO SENSOR REFLEXIVO
 */
void IRAM_ATTR InterruptionPino38(){
    reflexSensorTriggered = true;
}


/**********************************************************************************************
 *     INTERRUPÇÃO PINO 37 PARA CONTAR PULSO DO FLUXO DE AR/AGUA
 */
void IRAM_ATTR pulseCounter() {
    //pulseCount++;
}


/**********************************************************************************************
 *     INTERRUPÇÃO PINO 12 PARA CONTAR BATIDAS DA PRENSA
 */
void IRAM_ATTR InterruptionPino12() {
  batida_prensa = true;  // Sinaliza que o botão foi pressionado  
}

void verifica_interrupcao(){
  if (batida_prensa){
    verifica_batida_prensa();
    Serial.println(String(digitalRead(BATIDA_PIN)) + "- função verifica interrupção");
  }
  return;
}


/**********************************************************************************************
 *     VERIFICA AS BATIDAS DA PRENSA
 */
void verifica_batida_prensa(){
    char timeStr[20];  // armazena string do horário
    struct tm timeinfo;
    char nome_equipamento[10];    
    
    //atualiza horário    
    Serial.println(String(digitalRead(BATIDA_PIN)) + "- função verifica batida prensa");    
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Erro ao obter tempo!");
      return;
    }
    
    // Verifica se interrupção é falsa, batida de retorno
    delay(200);
    if (digitalRead(BATIDA_PIN) == HIGH){                 
      Serial.println("Batida falsa, retorno!");
      Serial.println(String(digitalRead(BATIDA_PIN)));        
      return;
    }

    // Confirma se nível continua 0
    if(digitalRead(BATIDA_PIN) == LOW){
        delay(250);        
        Serial.println(String(digitalRead(BATIDA_PIN)) + "atraso1");
    }else {
        Serial.println("falhou 1111");
        Serial.println(String(digitalRead(BATIDA_PIN)));
        return;
    }        

    if(digitalRead(BATIDA_PIN) == LOW){
        delay(250);        
        Serial.println(String(digitalRead(BATIDA_PIN)) + "atraso2");
    }else {
        Serial.println("falhou 22222");
        Serial.println(String(digitalRead(BATIDA_PIN)));
        return;
    }

    if(digitalRead(BATIDA_PIN) == LOW){
        delay(250);        
        Serial.println(String(digitalRead(BATIDA_PIN)) + "atraso3");
    }else {
        Serial.println("falhou 33333");
        Serial.println(String(digitalRead(BATIDA_PIN)));
        return;
    }
      
      Serial.println(String(digitalRead(BATIDA_PIN)) + "vai enviar dados");

      //envia mqtt
      strcpy(nome_equipamento, NOME_EQUIPAMENTO);      
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
      mqtt_send_data(nome_equipamento, timeStr);

      // Mostra quantidade de batida no display
      Serial.println(String(digitalRead(BATIDA_PIN)) + "Incrementa e mostra no display");
      qnt_batidas_prensa++;      
      tft.setTextColor(TFT_WHITE, TFT_BLACK);      
      tft.drawString((String)qnt_batidas_prensa,70, 50, 6);         

      // Mostra horário da ultima batida
      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);      
      tft.drawString("             ", 62, 105, 4);    
      tft.drawString(timeStr, 65, 105, 4);  
      
      Serial.println(String(digitalRead(BATIDA_PIN)) + "sainda da função");

  /*Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");   
  tft.drawString("             ", 62, 105, 4);    
  tft.drawString((String)timeinfo.tm_hour, 65, 105, 4);  
  tft.drawString(":", 94, 105, 4);
  tft.drawString((String)timeinfo.tm_min, 100, 105, 4);
  tft.drawString(":", 130, 105, 4);
  tft.drawString((String)timeinfo.tm_sec, 137, 105, 4); 
*/

  batida_prensa = false;  
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
  //apparentPower = Vrms * Irms;
  apparentPower = monitorEletricity.apparentPower;
  
powerFactor = abs(monitorEletricity.powerFactor);

  
  // mostra informação no Display e serial
  tft.setTextColor(TFT_WHITE, TFT_BLACK); 

  //Mostra Tensão
  tft.drawString("Tensao", 120, 18, 4);    
  tft.drawString((String)Vrms + " V   ", 135, 45, 4);  
  Serial.print("Vrms: ");       Serial.print(Vrms);  Serial.print(" v;   "); 
  
  //Mostra Corrente
  tft.drawString("Corrente", 5, 18, 4);
  tft.drawString((String)Irms + " A   ", 5, 45, 4);  
  Serial.print("Irms: ");     Serial.print(Irms);  Serial.print(" A;  ");

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
  Serial.print("P: ");      Serial.print(realPower);        Serial.print(" W;  ");    //Pot.Real
  Serial.print("S: ");      Serial.print(apparentPower);    Serial.print(" VA;  ");   //Pot.Aparente
  Serial.print("FP: ");     Serial.print(powerFactor);      Serial.print(";  ");      //Fator de Potência

// envia via mqtt
  //mqtt_send_data(Vrms, Irms, realPower, apparentPower, powerFactor, Qnt, potencia);
  

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
       // Qnt++;

        //show_diferença de tempo entre batida 
        time_t now = time(nullptr);   
        Serial.print("At(Batida): "); Serial.print(now - before);   
        before = now;

        
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


//show_diferença de tempo entre batida 
  time_t now = time(nullptr);   
  Serial.print("At(sample): "); Serial.print(now - before);   
  before = now;

  Serial.print("Qnt: ");       Serial.print((String)Qnt);   Serial.println("; ");

  // mostra quantidade de batida        
  tft.drawString((String)Qnt,190, 110, 4);                

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