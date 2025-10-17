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
#include "time.h"
#include "constants.h"
#include "wifi_mqtt.h"
#include "mem_flash.h"



// Pino do sensor reflexivo
#define BATIDA_PIN 12 
int qnt_batidas_prensa = 0;
volatile bool batida_prensa = false;
long idBatida = 0; // Variável global para armazenar o ID da batida
float distance_max = 100; // distância máxima do sensor ultrassônico em cm (padrão 400cm para o JSN-SR04T)
float percentual_reservatorio = 0.0; // percentual do reservatório
float altura_reservatorio = 100.0; // distância máxima do sensor ultrassônico em cm (padrão max. 400cm para o JSN-SR04T)




//Timer do ESP32
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
bool timerToSendReadings = false;
time_t timestamp_global = 0; // Variável global para armazenar o timestamp

//Verificação de reflexo
volatile bool reflexSensorTriggered = false;

// botão da placa
const int BUTTON_35 = 35;
float level_max = 20;   //cx d´agua
float level_min = 100;  //cx d´agua


// Variáveis para sincronização NTP
unsigned long lastNtpSync = 0;
const unsigned long ntpSyncInterval = 60UL * 60UL * 1000UL; // 1 hora

time_t before = 0;


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
void setup_batidas_prensa() {

  // Configura a interrupção para o botão
  attachInterrupt(digitalPinToInterrupt(BATIDA_PIN), InterruptionPino12, FALLING); // Configura a interrupção para o botão (pino 35) na borda de descida (pressionado)  

  //Defini GPIO
  pinMode(BATIDA_PIN, INPUT_PULLUP); // Configura o pino como entrada com pull-up interno
  
  // Inicializa horario do ntp com fuso -3
  //configTime(-3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.println("Sensor de batida da prensa configurado.");   

}



/**********************************************************************************************
 *     SETUP DO TIMER DO ESP32
 */
void setup_timer() {
  
  timer = timerBegin(0, 80, true); // Timer 0, prescaler 80 (1us por tick)
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000 * SAMPLE_INTERVAL, true); // 10.000.000us = 10s
  timerAlarmEnable(timer);
  
}

/**
  *    FUNÇÃO DE INTERRUPÇÃO DO TIMER DO ESP32
  *    Executa a cada 5 segundos
  *    Use para tarefas periódicas que não podem esperar o loop principal
  *    Exemplo: leitura de sensores críticos, atualização de variáveis de estado, etc.
  *    Lembre-se de manter o código da ISR o mais curto possível para evitar atrasos.
  *    Use portENTER_CRITICAL_ISR e portEXIT_CRITICAL_ISR para proteger variáveis compartilhadas.
  *    Evite chamadas de funções demoradas ou bloqueantes dentro da ISR.
  *    Se precisar fazer algo complexo, defina uma flag e trate no loop principal.
  */
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  timerToSendReadings = true; // Sinaliza para enviar ping MQTT
  portEXIT_CRITICAL_ISR(&timerMux);
}


void verifica_timer(){  
  if (timerToSendReadings){
    // Executa a ação desejada
    // Exemplo: Enviar dados de sensores via MQTT
    if (WiFi.status() == WL_CONNECTED) {
        bool enviado = mqtt_send_readings();                 
        // Se falha do MQTT, armazena no buffer
        if (!enviado) {            
            Serial.println("Falha MQTT ao enviar leituras");
        } else {
            Serial.println("Leituras enviadas via MQTT");
        }
    } else {
        Serial.println("WiFi desconectado - Nao foi possivel enviar leituras");
    }

    portENTER_CRITICAL_ISR(&timerMux);
    timerToSendReadings = false; // Reseta a flag
    portEXIT_CRITICAL_ISR(&timerMux);
  }
  //return;
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
        configTime(-3 * 3600, 0, "a.st1.ntp.br", "ntp.br", "time.nist.gov");
        lastNtpSync = now;
        Serial.println("NTP sincronizado");        
    }    

    if (getLocalTime(&timeinfo)) {
        timestamp_global = mktime(&timeinfo);
    }
    else {
      Serial.println("Erro ao obter tempo!");    
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
        bool enviado = mqtt_send_data(nome_equipamento, timeStr, id_leitura, "");
        // Se falha do MQTT, armazena no buffer
        if (!enviado) {            
            buffer_batida(nome_equipamento, timeStr, id_leitura, "Retransmitido - falha MQTT");
            Serial.println("Falha MQTT - Add info in buffer");
        }
    } else {
        // Se falha do WiFi, armazena no buffer        
        buffer_batida(nome_equipamento, timeStr, id_leitura, "Retransmitido - Falha WiFi");
        Serial.println("Falha WiFi - Add info in buffer");
    }

      // Mostra quantidade de batida no display
      Serial.println(String(digitalRead(BATIDA_PIN)) + "Incrementa e mostra no display");
      qnt_batidas_prensa++;      
      show_batidas(qnt_batidas_prensa);

      strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
      show_time(timeStr);
      
      batida_prensa = false;   
      
      try_send_buffered_batidas();
      tft.drawString(" ",2, 50, 6); //simula um led no display a cada batida (apaga o "led")

 
}


/**********************************************************************************************
 *     MOSTRA INFORMAÇÃO DE TEMPO DIA/MES/ANO  HORA:MINUTO:SEGUNDO
 */
void show_time() {

  char timeStr[20];  // Used to store time string
  struct tm timeinfo;
  unsigned long now = millis();


  if (now - lastNtpSync > ntpSyncInterval || lastNtpSync == 0) {
        configTime(-3 * 3600, 0, "a.st1.ntp.br", "ntp.br", "time.nist.gov");
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
 *     função que retorna o timestamp
 */

char* get_time_str(char* buffer, size_t bufferSize) {
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        time_t timestamp = mktime(&timeinfo);
        snprintf(buffer, bufferSize, "%ld", (long)timestamp);
    } else {
        strncpy(buffer, "0", bufferSize);
    }
    return buffer;
}



/**********************************************************************************************
 *     SINCRONIZAÇÃO NTP SE NECESSÁRIO
 */
void syncNtpIfNeeded() {
    unsigned long now = millis();
    if (now - lastNtpSync > ntpSyncInterval || lastNtpSync == 0) {
        configTime(-3 * 3600, 0, "a.st1.ntp.br", "ntp.br", "time.nist.gov");
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


/**********************************************************************************************
*     DEFINE A MINIMO E MAXIMO DO RESERVATÓRIO DE ÁGUA
*/
void set_reservatorio(){  
  //Configura o botão para definir mínimo do reservatório (Altura máxima do sensor)
  Serial.println("Pressione o botão para definir Minimo do reservatório");
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Pressione botao para", 3, 20, 4);
  tft.drawString("definir nivel min.", 10, 50, 4);
  tft.drawString(" ou aguarde inciar", 10, 80, 4);
  delay(4000); // Aguarda 4 segundos para o usuário se preparar

  if (digitalRead(BUTTON_35) == LOW){  // botão pressionado (GND){
      Serial.println("Botão pressionado - config caixa de água");
      UltrasonicResult res = ultrasonic_read();
      Serial.println("Altura mínima do reservatório: " + String(res.distance_cm) + " cm");
      //grava variavel em memoria não volátil                   
      save_flash_float(KEY_LEVEL_MIN, res.distance_cm);
      Serial.println("Altura mínima do reservatório gravada na EEPROM");
      tft.fillScreen(TFT_BLACK);
      tft.drawString("Altura minima: " + String(res.distance_cm) + " cm", 3, 20, 4);
      delay(3000);
  }
  else {
      //lê variavel em memoria não volátil          
      level_max = read_flash_float(KEY_LEVEL_MAX);
      if (level_max < 20 || level_max > 400 || isnan(level_max)){
          level_max = 20; //padrão
          Serial.println("Altura máxima do reservatório inválida, usando padrão: " + String(level_max) + " cm");
      }

      level_min = read_flash_float(KEY_LEVEL_MIN);
      if (level_min < 20 || level_min > 400 || isnan(level_min)){
          level_min = 400; //padrão
          Serial.println("Altura mínima do reservatório inválida, usando padrão: " + String(level_min) + " cm");
      }

      Serial.println("Altura mínima do reservatório (lido da EEPROM): " + String(level_min) + " cm");
      altura_reservatorio = level_min;
      Serial.println("Altura máxima do reservatório (lido da EEPROM): " + String(level_max) + " cm");
        if (level_max < 20 || level_max > 400){
            level_max = 100; //padrão
            Serial.println("Altura máxima do reservatório inválida, usando padrão: " + String(level_max) + " cm");
        }    
      delay(4000);  
      return;
  }  
  Serial.println("Pressione o botão para definir Máximo do reservatório");
  tft.fillScreen(TFT_BLACK);
  tft.drawString("Pressione o botão", 10, 30, 4);
  tft.drawString(" definir maximo", 10, 60, 4);
  delay(4000); // Aguarda 4 segundos para o usuário se preparar
  if (digitalRead(BUTTON_35) == LOW){  // botão pressionado (GND){
      Serial.println("Botão pressionado - config caixa de água");
      UltrasonicResult res = ultrasonic_read();
      Serial.println("Altura máxima do reservatório: " + String(res.distance_cm) + " cm");
      //grava variavel em memoria não volátil                   
      save_flash_float(KEY_LEVEL_MAX, res.distance_cm);
      Serial.println("Altura máxima do reservatório gravada na EEPROM");
      tft.fillScreen(TFT_BLACK);
      tft.drawString("Altura minima: " + String(res.distance_cm) + " cm", 3, 20, 4);
  }

}

/**************************************************************
 * LOOP PRINCIPAL DO STATE MACHINE
 */
void loop_state(void) {
  // Implementação básica do state machine
  // Adicione aqui a lógica específica da aplicação conforme necessário
}

/**********************************************************************************************/