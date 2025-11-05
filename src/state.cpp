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
#include "topicos.h"
#include "extern_data.h"

// Pino do sensor reflexivo
#define BATIDA_PIN 12 
int qnt_batidas_prensa = 0;
volatile bool batida_prensa = false;
long idBatida = 0; // Variável global para armazenar o ID da batida
int qtd_batidas_intervalo = 0;
float distance_max = 100; // distância máxima do sensor ultrassônico em cm (padrão 400cm para o JSN-SR04T)
float percentual_reservatorio = 0.0; // percentual do reservatório
float altura_reservatorio = 100.0; // distância máxima do sensor ultrassônico em cm (padrão max. 400cm para o JSN-SR04T)




//Timer do ESP32
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
bool timerToSendReadings = false;
bool timerToSendDataReadings = false;
time_t timestamp_global = 0; // Variável global para armazenar o timestamp

//Verificação de reflexo
volatile bool reflexSensorTriggered = false;

// botão da placa
const int BUTTON_35 = 35;
float level_max = 20;   //cx d´agua
float level_min = 100;  //cx d´agua
float filter_threshold = 10.0; // Threshold do filtro em % (padrão 10%)


// Variáveis para sincronização NTP
unsigned long lastNtpSync = 0;
const unsigned long ntpSyncInterval = 60UL * 60UL * 1000UL; // 1 hora

time_t before = 0;


// Variável global volátil para sinalizar a interrupção
volatile bool buttonPressed = false;


// Defina o tamanho máximo do buffer
#define MAX_BUFFERED_MSGS 300

// Estrutura para armazenar os dados da batida
struct BatidaMsg {
    char nome_equipamento[64];  // Device ID
    char timeStr[24];           // Timestamp string
    long id_message_batch;      // ID da mensagem de batch
    int qtd_batidas;            // Quantidade de batidas no intervalo
    int interval;               // Intervalo em segundos
    int cod_erro;               // Código de erro (10=MQTT, 20=WiFi)
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
*   Função de setup para o timer que envia via MQTT a somatoria de batidas
*/

void setup_timer_send_takt_time() {
  
  timer = timerBegin(1, 80, true); // Timer 1, prescaler 80 (1us por tick)
  timerAttachInterrupt(timer, &onTimerSendMqtt, true);
  timerAlarmWrite(timer, 1000000 * sample_interval_batch, true); // 1.000.000us * sample_interval_batch = sample_interval_batch segundos
  timerAlarmEnable(timer);
  Serial.printf("✅ Timer batch configurado para %d segundos\n", sample_interval_batch);
  
}

/**********************************************************************************************
*   Função para reconfigurar o timer batch_time dinamicamente
*/
void reconfigure_batch_timer(int new_interval) {
    if (new_interval <= 0 || new_interval > 3600) {
        Serial.println("⚠️ Intervalo inválido para timer batch. Ignorando...");
        return;
    }
    
    // Desabilita o timer atual
    timerAlarmDisable(timer);
    
    // Reconfigura com o novo intervalo
    timerAlarmWrite(timer, 1000000ULL * new_interval, true);
    
    // Reabilita o timer
    timerAlarmEnable(timer);
    
    Serial.printf("🔄 Timer batch reconfigurado para %d segundos\n", new_interval);
}

// Função de interrupção do timer para envio via MQTT
void IRAM_ATTR onTimerSendMqtt() {
  portENTER_CRITICAL_ISR(&timerMux);
  timerToSendDataReadings = true; // Sinaliza para enviar ping MQTT
  portEXIT_CRITICAL_ISR(&timerMux);
}



/**********************************************************************************************
 *     SETUP DO TIMER PARA TRIGGER DO ENVIO DO ULTRASSON E TEMPERATURA
 */
void setup_timer() {
  
  timer = timerBegin(0, 80, true); // Timer 0, prescaler 80 (1us por tick)
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 1000000 * SAMPLE_INTERVAL, true); // 10.000.000us = 10s
  timerAlarmEnable(timer);
  
}

/*     FUNÇÃO DE INTERRUPÇÃO DO TIMER DO ESP32
  *    Executa a cada 5 segundos  *   
  *    Exemplo: leitura de sensores críticos, atualização de variáveis de estado, etc.     
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
        enabled_send_temperature_readings = true;
        enabled_send_humidity_readings = true;
        enabled_send_level_readings = true;
        bool enviado = mqtt_send_datas_readings();
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
     

      id_message_batch++;  // Incrementa o ID da leitura
      qtd_batidas_intervalo ++; // Incrementa batida no intervalo
      Serial.println(String(digitalRead(BATIDA_PIN)) + "batida: " + String(id_message_batch));  // Mostra a leitura do pino 12

      //prepara dados para enviar mqtt
      strcpy(nome_equipamento, NOME_EQUIPAMENTO);      
      strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
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



// Envia via MQTT quando chegar o timer correto
  void check_timer_interrupt_tosend_MqttDataReadings() {
      if (timerToSendDataReadings == true) {
          if (qtd_batidas_intervalo > 0) {
                char timeStr[20];
                struct tm timeinfo;
                
                // Obtém timestamp atual
                if (getLocalTime(&timeinfo)) {
                    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
                } else {
                    strcpy(timeStr, "1970-01-01 00:00:00");
                }
                
                if (WiFi.status() == WL_CONNECTED) {
                      enabled_send_batch_readings = true; // Habilita o envio da leitura do ticket
                      bool enviado = mqtt_send_datas_readings();

                    // Se falha do MQTT, armazena no buffer
                    if (!enviado) {            
                        buffer_batida(DISPOSITIVO_ID, timeStr, id_message_batch, qtd_batidas_intervalo, 60 /*interval*/, 10/*cod erro MQTT*/);  
                        Serial.println("❌ Falha MQTT - Dados armazenados no buffer");
                    } else {
                        Serial.println("✅ Dados batch_time enviados via MQTT com sucesso!");
                    }              
                    
                } else {
                    // Se falha do WiFi, armazena no buffer
                    buffer_batida(DISPOSITIVO_ID, timeStr, id_message_batch, qtd_batidas_intervalo, 60 /*interval*/, 20/*cod erro WiFi*/);
                    Serial.println("❌ Falha WiFi - Dados armazenados no buffer");
                }
          }
          
          timerToSendDataReadings = false; // Reseta a flag após envio
          Serial.printf("Timer MQTT batidas verificado. qntd: %d\n", qtd_batidas_intervalo);
          qtd_batidas_intervalo = 0;
          
          // Tenta enviar mensagens pendentes do buffer
          try_send_buffered_batidas();
      }        
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
/**********************************************************************************************
 *     ADICIONA MENSAGEM AO BUFFER
 */
void buffer_batida(const char* nome, const char* timeStr, long id, int qtd_batidas, int interval, int cod_erro) {
    if (bufferCount < MAX_BUFFERED_MSGS) {
        strncpy(batidaBuffer[bufferTail].nome_equipamento, nome, sizeof(batidaBuffer[bufferTail].nome_equipamento)-1);
        batidaBuffer[bufferTail].nome_equipamento[sizeof(batidaBuffer[bufferTail].nome_equipamento)-1] = '\0';
        
        strncpy(batidaBuffer[bufferTail].timeStr, timeStr, sizeof(batidaBuffer[bufferTail].timeStr)-1);
        batidaBuffer[bufferTail].timeStr[sizeof(batidaBuffer[bufferTail].timeStr)-1] = '\0';
        
        batidaBuffer[bufferTail].id_message_batch = id;
        batidaBuffer[bufferTail].qtd_batidas = qtd_batidas;
        batidaBuffer[bufferTail].interval = interval;
        batidaBuffer[bufferTail].cod_erro = cod_erro;
        
        bufferTail = (bufferTail + 1) % MAX_BUFFERED_MSGS;
        bufferCount++;
        
        Serial.printf("📦 Buffer armazenado: [%d/%d] Device: %s, Batidas: %d, Erro: %d\n", 
                      bufferCount, MAX_BUFFERED_MSGS, nome, qtd_batidas, cod_erro);
    } else {
        // Buffer cheio, pode descartar ou sobrescrever o mais antigo
        Serial.println("⚠️ Buffer de batidas CHEIO! Mensagem descartada.");
    }
    tft.drawString(String(bufferCount), 10, 105, 4);
}

/**********************************************************************************************
 *     TENTA ENVIAR TODAS AS MENSAGENS DO BUFFER
 */
void try_send_buffered_batidas() {
    while (bufferCount > 0) {
        BatidaMsg& msg = batidaBuffer[bufferHead];
        
        // Habilita apenas o envio de ticket com os dados do buffer
        enabled_send_batch_readings = true;
        
        // Temporariamente armazena qtd_batidas_intervalo e restaura após envio
        int qtd_batidas_backup = qtd_batidas_intervalo;
        qtd_batidas_intervalo = msg.qtd_batidas;
        
        bool enviado = mqtt_send_datas_readings();
        
        // Restaura o valor original
        qtd_batidas_intervalo = qtd_batidas_backup;
        
        if (enviado) {
            bufferHead = (bufferHead + 1) % MAX_BUFFERED_MSGS;
            bufferCount--;            
            Serial.println("✅ Mensagem do buffer enviada com sucesso!");
        } else {
            // Se falhar, pare para tentar novamente depois
            Serial.println("⚠️ Falha ao enviar mensagem do buffer, tentando novamente mais tarde...");
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
      // As configurações já foram carregadas em load_all_settings_from_flash()
      // chamada no main.cpp durante o setup
      Serial.println("📋 Usando configurações já carregadas da flash:");
      Serial.println("   Level_max: " + String(level_max) + " cm");
      Serial.println("   Level_min: " + String(level_min) + " cm");
      
      altura_reservatorio = level_min;      
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