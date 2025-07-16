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


// Variáveis para sincronização NTP
unsigned long lastNtpSync = 0;
const unsigned long ntpSyncInterval = 60UL * 60UL * 1000UL; // 1 hora

time_t before = 0;

//Potência
int tipoPotencia = 0;

// Variável global volátil para sinalizar a interrupção
volatile bool buttonPressed = false;

// Variável global para armazenar o ID da leitura
long id_leitura = 0;

// Defina o tamanho máximo do buffer
#define MAX_BUFFERED_MSGS 300

// Estrutura para armazenar os dados da batida
struct BatidaMsg {
    char nome_equipamento[16];
    char timeStr[24];
    long id_leitura;
    char observacao[32];
};

// Buffer circular
BatidaMsg batidaBuffer[MAX_BUFFERED_MSGS];
int bufferHead = 0;
int bufferTail = 0;
int bufferCount = 0;

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
  attachInterrupt(digitalPinToInterrupt(BATIDA_PIN), InterruptionPino12, FALLING); // Configura a interrupção para o botão (pino 35) na borda de descida (pressionado)  

  //Defini GPIO
  pinMode(BATIDA_PIN, INPUT_PULLUP); // Configura o pino como entrada com pull-up interno
  
  // Inicializa horario do ntp com fuso -3
  //configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

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
  
  // Verifica fluxo com sensor YF-S201
  //calcula_fluxo();

  //Responsável por verificar o sensor reflexivo
  if (reflexSensorTriggered){
    reflexSensorTriggered = false;
    Serial.println("Sensor reflexivo ativado!");
    Qnt++;
  }
  //  try_send_buffered_batidas();

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
    batida_prensa = false;  
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
     unsigned long now = millis();
    
    //informação para log
    //Serial.println(String(digitalRead(BATIDA_PIN)) + "- verifica_batida_prensa");    
    tft.drawString("*",2, 50, 6); //simula um led no display a cada batida (acende o "led")
    
     //atualiza horário   
    

    
  if (now - lastNtpSync > ntpSyncInterval || lastNtpSync == 0) {
        configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        lastNtpSync = now;
        Serial.println("NTP sincronizado");        
  }    

  if (!getLocalTime(&timeinfo)) {
      Serial.println("Erro ao obter tempo!");
    //  return;
  }
    
    // Verifica se interrupção é falsa, batida de retorno
    delay(100);
    if (digitalRead(BATIDA_PIN) == HIGH){                 
     // Serial.println("Batida falsa, retorno!");     
      return;
    }

    // Confirma se nível continua 0
    if(digitalRead(BATIDA_PIN) == LOW){
        delay(100);              
    }else {
        Serial.println("falhou <100ms");        
        return;
    }        

    if(digitalRead(BATIDA_PIN) == LOW){
        delay(100);                
    }else {
        Serial.println("falhou <200ms");        
        return;
    }

    if(digitalRead(BATIDA_PIN) == LOW){
        delay(100);                
    }else {
        Serial.println("falhou <300ms");        
        return;
    }
     

      id_leitura++;  // Incrementa o ID da leitura
      Serial.println(String(digitalRead(BATIDA_PIN)) + "batida: " + String(id_leitura));  // Mostra a leitura do pino 12

      //prepara dados para enviar mqtt
      strcpy(nome_equipamento, NOME_EQUIPAMENTO);      
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);

    // Tenta enviar imediatamente
    if (WiFi.status() == WL_CONNECTED) {
        bool enviado = mqtt_send_data(nome_equipamento, timeStr, id_leitura, " ");
        // Se falha do MQTT, armazena no buffer
        if (!enviado) {            
            buffer_batida(nome_equipamento, timeStr, id_leitura, "Retransmitido - falha MQTT");
        }
    } else {
        // Se falha do WiFi, armazena no buffer        
        buffer_batida(nome_equipamento, timeStr, id_leitura, "Retransmitido - Falha WiFi");
    }

      // Mostra quantidade de batida no display
      Serial.println(String(digitalRead(BATIDA_PIN)) + "Incrementa e mostra no display");
      qnt_batidas_prensa++;      
      tft.setTextColor(TFT_WHITE, TFT_BLACK);      
      tft.drawString((String)qnt_batidas_prensa,70, 50, 6);         

      // Mostra horário da ultima batida
      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);      
      tft.drawString("             ", 62, 105, 4);    
      tft.drawString(timeStr, 130, 105, 4);  
      
      batida_prensa = false;   
      
      try_send_buffered_batidas();
      tft.drawString(" ",2, 50, 6); //simula um led no display a cada batida (apaga o "led")

 
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
  unsigned long now = millis();


  if (now - lastNtpSync > ntpSyncInterval || lastNtpSync == 0) {
        configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        lastNtpSync = now;
        Serial.println("NTP sincronizado");
  }
  //syncNtpIfNeeded(); // Synchronize NTP time if needed  
  
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Erro ao obter tempo2!");
 //   return;
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

/**********************************************************************************************
 *     SINCRONIZAÇÃO NTP SE NECESSÁRIO
 */
void syncNtpIfNeeded() {
    unsigned long now = millis();
    if (now - lastNtpSync > ntpSyncInterval || lastNtpSync == 0) {
        configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
        lastNtpSync = now;
        Serial.println("NTP sincronizado");
    }
}

/**********************************************************************************************
 *     ADICIONA MENSAGEM AO BUFFER
 */
void buffer_batida(const char* nome, const char* timeStr, long id, const char* obs) {
    if (bufferCount < MAX_BUFFERED_MSGS) {
        strncpy(batidaBuffer[bufferTail].nome_equipamento, nome, sizeof(batidaBuffer[bufferTail].nome_equipamento)-1);
        strncpy(batidaBuffer[bufferTail].timeStr, timeStr, sizeof(batidaBuffer[bufferTail].timeStr)-1);
        batidaBuffer[bufferTail].id_leitura = id;
        strncpy(batidaBuffer[bufferTail].observacao, obs, sizeof(batidaBuffer[bufferTail].observacao)-1);
        bufferTail = (bufferTail + 1) % MAX_BUFFERED_MSGS;
        bufferCount++;        
    } else {
        // Buffer cheio, pode descartar ou sobrescrever o mais antigo
        Serial.println("Buffer de batidas cheio! Mensagem descartada.");
    }
    tft.drawString(String(bufferCount), 10, 105, 4);
    Serial.println("buffer_inc: " + String(bufferCount));
}

/**********************************************************************************************
 *     TENTA ENVIAR TODAS AS MENSAGENS DO BUFFER
 */
void try_send_buffered_batidas() {
    while (bufferCount > 0) {
        BatidaMsg& msg = batidaBuffer[bufferHead];
        bool enviado = mqtt_send_data(msg.nome_equipamento, msg.timeStr, msg.id_leitura, msg.observacao);
        if (enviado) {
            bufferHead = (bufferHead + 1) % MAX_BUFFERED_MSGS;
            bufferCount--;            
        } else {
            // Se falhar, pare para tentar novamente depois
            break;
        }
        tft.drawString(String(bufferCount) + "  ", 10, 105, 4);
        Serial.println("buffer_dec: " + String(bufferCount));
    } 
}