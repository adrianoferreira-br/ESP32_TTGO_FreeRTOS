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

// Variáveis do Sensor 1 (definido em hardware_config.h como SENSOR_BATIDA_1)
int qnt_batidas_prensa = 0;
volatile bool batida_prensa = false;
long idBatida = 0; // Variável global para armazenar o ID da batida
int qtd_batidas_intervalo = 0;

// Variáveis do Sensor 2 (definido em hardware_config.h como SENSOR_BATIDA_2)
int qnt_batidas_prensa_sensor2 = 0;
volatile bool batida_prensa_sensor2 = false;
long idBatida_sensor2 = 0;
int qtd_batidas_intervalo_sensor2 = 0;
float distance_max = 100; // distância máxima do sensor ultrassônico em cm (padrão 400cm para o JSN-SR04T)
float percentual_reservatorio = 0.0; // percentual do reservatório
float altura_reservatorio = 100.0; // distância máxima do sensor ultrassônico em cm (padrão max. 400cm para o JSN-SR04T)

int message_error_code = 0;


//Timer do ESP32
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
bool timerToSendReadings = false;
bool timerToSendDataReadings = false;
time_t timestamp_global = 0; // Variável global para armazenar o timestamp

//Verificação de reflexo
volatile bool reflexSensorTriggered = false;

// botão da placa (definido em hardware_config.h como BTN_USER)
float level_max = 20;   //cx d´agua
float level_min = 100;  //cx d´agua
float filter_threshold = 10.0; // Threshold do filtro em % (padrão 10%)


// Variáveis para sincronização NTP
unsigned long lastNtpSync = 0;
const unsigned long ntpSyncInterval = 60UL * 60UL * 1000UL; // 1 hora

time_t before = 0;


// Variável global volátil para sinalizar a interrupção
volatile bool buttonPressed = false;



// VARIÁVEIS DE ACUMULAÇÃO DE BATIDAS - SENSOR 1
int accumulated_batidas = 0;           // Contador acumulado de batidas durante desconexão
unsigned long disconnection_start_time = 0; // Timestamp da primeira batida durante desconexão
bool is_accumulating = false;              // Flag indicando se está acumulando batidas

// VARIÁVEIS DE ACUMULAÇÃO DE BATIDAS - SENSOR 2
int accumulated_batidas_sensor2 = 0;
unsigned long disconnection_start_time_sensor2 = 0;
bool is_accumulating_sensor2 = false;

/**********************************************************************************************
 *     FUNÇÃO DE SETUP E CONFIGURAÇÃO INICIAL DA APLICAÇÃO
 */
void setup_batidas_prensa() {

  // Configura a interrupção para o Sensor 1
  attachInterrupt(digitalPinToInterrupt(SENSOR_BATIDA_1), InterruptionPino12, FALLING);
  pinMode(SENSOR_BATIDA_1, INPUT_PULLUP);
  
  // Configura a interrupção para o Sensor 2
  attachInterrupt(digitalPinToInterrupt(SENSOR_BATIDA_2), InterruptionPino13, FALLING);
  pinMode(SENSOR_BATIDA_2, INPUT_PULLUP);
  
  Serial.println("====================================");
  Serial.println("Sensores de batida configurados:");
  Serial.printf("  Sensor 1: GPIO %d\n", SENSOR_BATIDA_1);
  Serial.printf("  Sensor 2: GPIO %d\n", SENSOR_BATIDA_2);
  Serial.println("====================================");   

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
 *     INTERRUPÇÃO PINO 12 PARA CONTAR BATIDAS DA PRENSA - SENSOR 1
 */
void IRAM_ATTR InterruptionPino12() {
  batida_prensa = true;
}

/**********************************************************************************************
 *     INTERRUPÇÃO PINO 13 PARA CONTAR BATIDAS DA PRENSA - SENSOR 2
 */
void IRAM_ATTR InterruptionPino13() {
  batida_prensa_sensor2 = true;
}

void verifica_interrupcao(){
  if (batida_prensa){
    batida_prensa = false; // Reset flag ANTES de processar para evitar múltiplas execuções
    verifica_batida_prensa();      
  }
  if (batida_prensa_sensor2){
    batida_prensa_sensor2 = false;
    verifica_batida_prensa_sensor2();
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
    
    // ✅ SINCRONIZA NTP APENAS SE CONECTADO (evita bloqueio quando desconectado)
    if (WiFi.status() == WL_CONNECTED) {
        if (now - lastNtpSync > ntpSyncInterval || lastNtpSync == 0) {
            configTime(-3 * 3600, 0, "a.st1.ntp.br", "ntp.br", "time.nist.gov");
            lastNtpSync = now;
            Serial.println("NTP sincronizado");        
        }
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
      
      tft.drawString(" ",2, 50, 6); //simula um led no display a cada batida (apaga o "led")
}


/**********************************************************************************************
 *     VERIFICA AS BATIDAS DA PRENSA - SENSOR 2 (GPIO 13)
 */
void verifica_batida_prensa_sensor2(){
    char timeStr[20];
    struct tm timeinfo;
    char nome_equipamento[10];    
    unsigned long now = millis();
    
    tft.drawString("*",20, 50, 6); // LED virtual no display para sensor 2
    
    // Sincroniza NTP se conectado
    if (WiFi.status() == WL_CONNECTED) {
        if (now - lastNtpSync > ntpSyncInterval || lastNtpSync == 0) {
            configTime(-3 * 3600, 0, "a.st1.ntp.br", "ntp.br", "time.nist.gov");
            lastNtpSync = now;
            Serial.println("NTP sincronizado");        
        }
    }

    if (getLocalTime(&timeinfo)) {
        timestamp_global = mktime(&timeinfo);
    }
    else {
      Serial.println("Erro ao obter tempo!");    
    }
    
    // Verifica se interrupção é falsa (debounce)
    delay(100);
    if (digitalRead(SENSOR_BATIDA_2) == HIGH){
      return;
    }

    // Confirmação de nível LOW por 300ms
    if(digitalRead(SENSOR_BATIDA_2) == LOW){
        delay(100);              
    }else {
        Serial.println("Sensor2: falhou <100ms");        
        return;
    }        

    if(digitalRead(SENSOR_BATIDA_2) == LOW){
        delay(100);                
    }else {
        Serial.println("Sensor2: falhou <200ms");        
        return;
    }

    if(digitalRead(SENSOR_BATIDA_2) == LOW){
        delay(100);                
    }else {
        Serial.println("Sensor2: falhou <300ms");        
        return;
    }
     
    id_message_batch2++;
    qtd_batidas_intervalo_sensor2++;
    Serial.println("Sensor2 - batida: " + String(id_message_batch));

    strcpy(nome_equipamento, NOME_EQUIPAMENTO);      
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    // Mostra quantidade de batida no display (sensor 2)
    qnt_batidas_prensa_sensor2++;      
    show_batidas_sensor2(qnt_batidas_prensa_sensor2);

    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    show_time(timeStr);
      
    batida_prensa_sensor2 = false;   
      
    tft.drawString(" ",20, 50, 6); // Apaga LED virtual do sensor 2
}


// Envia via MQTT quando chegar o timer correto
void check_timer_interrupt_tosend_MqttDataReadings() {
    bool wifi_connected = (WiFi.status() == WL_CONNECTED);
    bool mqtt_connected = client.connected();
    
    // ✅ PRIORIDADE: Verifica se há dados acumulados E reconectou (independente do timer)
    if (is_accumulating && wifi_connected && mqtt_connected) {
        unsigned long accumulated_time = (millis() - disconnection_start_time) / 1000;
        
        Serial.println("✅ Reconectado! Enviando dados acumulados...");
        Serial.printf("📊 Total acumulado: %d batidas em %lu segundos\n", 
                      accumulated_batidas, accumulated_time);
        
        // Temporariamente substitui valores para envio
        int qtd_backup = qtd_batidas_intervalo;
        int interval_backup = sample_interval_batch;
        int error_backup = message_error_code;
        
        qtd_batidas_intervalo = accumulated_batidas;
        sample_interval_batch = accumulated_time;
        // message_error_code já está setado (1=WiFi ou 2=MQTT)
        
        enabled_send_batch_readings = true;
        bool enviado = mqtt_send_datas_readings();
        
        // Restaura valores originais
        qtd_batidas_intervalo = qtd_backup;
        sample_interval_batch = interval_backup;
        
        if (enviado) {
            Serial.println("✅ Dados acumulados enviados com sucesso!");
            // RESETA ACUMULAÇÃO
            is_accumulating = false;
            accumulated_batidas = 0;
            disconnection_start_time = 0;
            message_error_code = 0;
            
            // 📺 LIMPA DISPLAY (volta para zero)
            tft.drawString("0    ", 10, 105, 4);
        } else {
            Serial.println("❌ Falha ao enviar dados acumulados, continuando acumulação");
            message_error_code = error_backup; // Mantém código de erro
            
            // 📺 MANTÉM DISPLAY COM VALOR ACUMULADO
            tft.drawString(String(accumulated_batidas) + "  ", 10, 105, 4);
        }
        
        // Sai da função após tentar enviar dados acumulados
        return;
    }
    
    // ✅ PRIORIDADE: Verifica se há dados acumulados do SENSOR 2 E reconectou
    if (is_accumulating_sensor2 && wifi_connected && mqtt_connected) {
        unsigned long accumulated_time_s2 = (millis() - disconnection_start_time_sensor2) / 1000;
        
        Serial.println("✅ Sensor2: Reconectado! Enviando dados acumulados...");
        Serial.printf("📊 Sensor2: Total acumulado: %d batidas em %lu segundos\n", 
                      accumulated_batidas_sensor2, accumulated_time_s2);
        
        // Temporariamente substitui valores para envio
        int qtd_backup_s2 = qtd_batidas_intervalo_sensor2;
        int interval_backup_s2 = sample_interval_batch;
        int error_backup_s2 = message_error_code;
        
        qtd_batidas_intervalo_sensor2 = accumulated_batidas_sensor2;
        sample_interval_batch = accumulated_time_s2;
        
        enabled_send_batch_readings_sensor2 = true;
        bool enviado_s2 = mqtt_send_datas_readings();
        
        // Restaura valores originais
        qtd_batidas_intervalo_sensor2 = qtd_backup_s2;
        sample_interval_batch = interval_backup_s2;
        
        if (enviado_s2) {
            Serial.println("✅ Sensor2: Dados acumulados enviados com sucesso!");
            // RESETA ACUMULAÇÃO
            is_accumulating_sensor2 = false;
            accumulated_batidas_sensor2 = 0;
            disconnection_start_time_sensor2 = 0;
            message_error_code = 0;
        } else {
            Serial.println("❌ Sensor2: Falha ao enviar dados acumulados, continuando acumulação");
            message_error_code = error_backup_s2; // Mantém código de erro
        }
        
        // Sai da função após tentar enviar dados acumulados do sensor 2
        return;
    }
    
    // ====== PROCESSAMENTO DO TIMER PARA AMBOS OS SENSORES ======
    if (timerToSendDataReadings == true) {
        
        // ====== PROCESSA SENSOR 1 (GPIO 12) ======
        if (qtd_batidas_intervalo > 0) {
            if (!wifi_connected || !mqtt_connected) {
                if (!is_accumulating) {
                    // INICIA ACUMULAÇÃO
                    is_accumulating = true;
                    accumulated_batidas = qtd_batidas_intervalo;
                    disconnection_start_time = millis();
                    message_error_code = !wifi_connected ? 1 : 2;
                    Serial.printf("⚠️ Sensor1: Iniciando acumulação: %d batidas\n", accumulated_batidas);
                    tft.drawString(String(accumulated_batidas) + "  ", 10, 105, 4);
                } else {
                    // CONTINUA ACUMULANDO
                    accumulated_batidas += qtd_batidas_intervalo;
                    Serial.printf("📊 Sensor1: Acumulando: +%d (Total: %d)\n", 
                                  qtd_batidas_intervalo, accumulated_batidas);
                    tft.drawString(String(accumulated_batidas) + "  ", 10, 105, 4);
                }
            } else {
                // Habilita flag para envio posterior
                message_error_code = 0;
                enabled_send_batch_readings = true;
                Serial.printf("✅ Sensor1: Flag habilitada para envio: %d batidas\n", qtd_batidas_intervalo);
            }
        }
        
        // ====== PROCESSA SENSOR 2 (GPIO 13) ======
        if (qtd_batidas_intervalo_sensor2 > 0) {
            if (!wifi_connected || !mqtt_connected) {
                if (!is_accumulating_sensor2) {
                    is_accumulating_sensor2 = true;
                    accumulated_batidas_sensor2 = qtd_batidas_intervalo_sensor2;
                    disconnection_start_time_sensor2 = millis();
                    message_error_code = !wifi_connected ? 1 : 2;
                    Serial.printf("⚠️ Sensor2: Iniciando acumulação: %d batidas\n", accumulated_batidas_sensor2);
                } else {
                    accumulated_batidas_sensor2 += qtd_batidas_intervalo_sensor2;
                    Serial.printf("📊 Sensor2: Acumulando: +%d (Total: %d)\n", 
                                  qtd_batidas_intervalo_sensor2, accumulated_batidas_sensor2);
                }
            } else {
                // Habilita flag para envio posterior
                message_error_code = 0;
                enabled_send_batch_readings_sensor2 = true;
                Serial.printf("✅ Sensor2: Flag habilitada para envio: %d batidas\n", qtd_batidas_intervalo_sensor2);
            }
        }
        
        // ====== ENVIO ÚNICO PARA AMBOS OS SENSORES ======
        // Só envia se pelo menos um dos sensores tiver dados E estiver conectado
        if ((enabled_send_batch_readings || enabled_send_batch_readings_sensor2) && wifi_connected && mqtt_connected) {
            bool enviado = mqtt_send_datas_readings();
            
            if (enviado) {
                Serial.println("✅ Dados enviados via MQTT com sucesso!");
                if (enabled_send_batch_readings) {
                    Serial.printf("  - Sensor1: %d batidas\n", qtd_batidas_intervalo);
                }
                if (enabled_send_batch_readings_sensor2) {
                    Serial.printf("  - Sensor2: %d batidas\n", qtd_batidas_intervalo_sensor2);
                }
            } else {
                Serial.println("❌ Falha no envio MQTT");
                // Inicia acumulação para os sensores que falharam
                if (enabled_send_batch_readings && !is_accumulating) {
                    is_accumulating = true;
                    accumulated_batidas = qtd_batidas_intervalo;
                    disconnection_start_time = millis();
                    Serial.println("❌ Sensor1: Iniciando acumulação após falha");
                    tft.drawString(String(accumulated_batidas) + "  ", 10, 105, 4);
                }
                if (enabled_send_batch_readings_sensor2 && !is_accumulating_sensor2) {
                    is_accumulating_sensor2 = true;
                    accumulated_batidas_sensor2 = qtd_batidas_intervalo_sensor2;
                    disconnection_start_time_sensor2 = millis();
                    Serial.println("❌ Sensor2: Iniciando acumulação após falha");
                }
                message_error_code = mqtt_connected ? 1 : 2;
            }
        }
        
        // Reseta a flag do timer e os contadores
        timerToSendDataReadings = false;
        Serial.printf("Timer MQTT verificado. Sensor1: %d | Sensor2: %d\n", 
                      qtd_batidas_intervalo, qtd_batidas_intervalo_sensor2);
        qtd_batidas_intervalo = 0;
        qtd_batidas_intervalo_sensor2 = 0;
    }        
}



/**********************************************************************************************
 *     MOSTRA INFORMAÇÃO DE TEMPO DIA/MES/ANO  HORA:MINUTO:SEGUNDO
 */
void show_time() {

  char timeStr[20];  // Used to store time string
  struct tm timeinfo;
  unsigned long now = millis();


  // ✅ SINCRONIZA NTP APENAS SE CONECTADO (evita bloqueio quando desconectado)
  if (WiFi.status() == WL_CONNECTED) {
      if (now - lastNtpSync > ntpSyncInterval || lastNtpSync == 0) {
            configTime(-3 * 3600, 0, "a.st1.ntp.br", "ntp.br", "time.nist.gov");
            lastNtpSync = now;
            Serial.println("NTP sincronizado");
      }
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

  if (digitalRead(BTN_USER) == LOW){  // botão pressionado (GND){
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
  if (digitalRead(BTN_USER) == LOW){  // botão pressionado (GND){
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