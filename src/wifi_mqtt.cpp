/************************************************************
 *  File: wifi_mqtt.cpp
 *  Description:  WIFI and MQTT
 *  date: 2025-01-14
 ***********************************************************/

#include "wifi_mqtt.h"
#include "constants.h"
#include "main.h"
#include <ArduinoJson.h>
//partions
#include "esp_partition.h"
#include <esp_ota_ops.h>
// WebServer
#include <WebServer.h>
#include "web_server.h"
//OTA
#include <ArduinoOTA.h>
#include <Update.h>
//mDNS
#include <ESPmDNS.h>
//display
#include <display.h>
// flash
#include "mem_flash.h"

#define MQTT_MAX_PACKET_SIZE 1024

WiFiClient espClient;
PubSubClient client(espClient);
//ip_addr_t new_dns;



const char* ssid = SSID;                //"STARLINK";//"PhoneAdr"; // Substitua pelo seu SSID 
const char* password = PASSWORD;        //"11121314";//"UDJ1-ddsp";// "SUA_SENHA"; // Substitua pela sua senha 




/**************************************************************
 * INICIALIZAÇÃO DO WIFI
 */
void setup_wifi(){

   int i = 0;
   char ssid_tmp[32];
   char password_tmp[64];

   // Lê ssid e password na memoria NVS, se não existir usa os definidos em constants.cpp
   read_flash_string(KEY_WIFI_SSID, ssid_tmp, 32);
   read_flash_string(KEY_WIFI_PASS, password_tmp, 64);
   if (strlen(ssid_tmp) > 0) {
       ssid = ssid_tmp;       
   }

   if (strlen(password_tmp) > 0) {
       password = password_tmp;
   }

   Serial.println("ssid lindo em NVS: " + String(ssid_tmp) + " Usando: " + String(ssid));
   Serial.print("Conectando a ");
   Serial.println(ssid);


   WiFi.begin(ssid, password); 
   do  
   { 
      delay(1000); 
      Serial.print("."); 
      i++;
   } while (((WiFi.status() != WL_CONNECTED) && (i<360)));

   if (WiFi.status() != WL_CONNECTED)
   {
      Serial.println("Falha ao conectar na rede");
      return;
   } 
   else 
   {
     Serial.println(""); 
     // tft.fillScreen(TFT_BLACK);  // Comentado para teste OTA sem display
     Serial.println("WiFi conectado"); 
     Serial.print("Endereço IP: "); 
     Serial.println(WiFi.localIP());     
   }       
  }

  

/**************************************************************
 * LOOP DO WIFI 
 */
void loop_wifi(){
  // Preenche informações referente a rede
  if (WiFi.status() == WL_CONNECTED) {       
      show_ip();              
  } else {
     // tft.setTextColor(TFT_RED, TFT_BLACK);    
     // tft.drawString("Disconnected     ", 0, 0, 2);  
     // tft.drawString("                 ", 130, 0, 2);        
  }   
}


/**************************************************************
 * SETUP NTP
 */

void setup_ntp() {    
    configTime(-3 * 3600, 0, "a.st1.ntp.br", "ntp.br", "time.nist.gov");
    struct tm timeinfo;
    int tentativas = 0;
    while (!getLocalTime(&timeinfo) && tentativas < 10) {
        Serial.println("Aguardando sincronização NTP...");
        delay(1000);
        tentativas++;
    }
    if (tentativas < 10) {
        Serial.println("NTP sincronizado!");
    } else {
        Serial.println("Falha ao sincronizar NTP.");
    }    
}



/**************************************************************
 * MOSTRA INFO DAS PARTIÇÕES 
 */
void show_partitions() 
{
  Serial.println("=== ANÁLISE COMPLETA DAS PARTIÇÕES NO BOOT ===");
  
  // Motivo do último reset/boot
  esp_reset_reason_t reset_reason = esp_reset_reason();
  Serial.printf("Motivo do boot: ");
  switch(reset_reason) {
    case ESP_RST_POWERON: Serial.println("Power-on reset"); break;
    case ESP_RST_EXT: Serial.println("Reset externo"); break;
    case ESP_RST_SW: Serial.println("Reset por software (ESP.restart()) ← POSSÍVEL OTA"); break;
    case ESP_RST_PANIC: Serial.println("Reset por panic/exception"); break;
    case ESP_RST_INT_WDT: Serial.println("Reset por watchdog interno"); break;
    case ESP_RST_TASK_WDT: Serial.println("Reset por task watchdog"); break;
    case ESP_RST_WDT: Serial.println("Reset por watchdog"); break;
    case ESP_RST_DEEPSLEEP: Serial.println("Wake up do deep sleep"); break;
    case ESP_RST_BROWNOUT: Serial.println("Reset por brownout"); break;
    case ESP_RST_SDIO: Serial.println("Reset por SDIO"); break;
    default: Serial.printf("Motivo desconhecido (%d)\n", reset_reason); break;
  }
  
  // Informações críticas das partições OTA
  const esp_partition_t* running_partition = esp_ota_get_running_partition();
  const esp_partition_t* boot_partition = esp_ota_get_boot_partition();
  
  Serial.println("\n=== DIAGNÓSTICO CRÍTICO DE PARTIÇÕES ===");
  if (running_partition) {
    Serial.printf("🟢 EXECUTANDO DA PARTIÇÃO: %s (0x%06x)\n", 
                  running_partition->label, running_partition->address);
  }
  
  if (boot_partition) {
    Serial.printf("🔵 PARTIÇÃO DE BOOT CONFIGURADA: %s (0x%06x)\n", 
                  boot_partition->label, boot_partition->address);
  }
  
  // ANÁLISE CRÍTICA: Verificar se as partições coincidem
  if (running_partition && boot_partition) {
    if (running_partition->address == boot_partition->address) {
      Serial.println("✅ NORMAL: Sistema está executando da partição de boot correta");
    } else {
      Serial.println("❌ PROBLEMA DETECTADO!");
      Serial.println("   A partição em execução é DIFERENTE da partição de boot!");
      Serial.println("   POSSÍVEIS CAUSAS:");
      Serial.println("   1. OTA falhou em ativar a nova partição");
      Serial.println("   2. Nova partição tem firmware inválido, sistema reverteu");
      Serial.println("   3. Problema na gravação do OTA data");
      Serial.println("   4. Corrupção na partição OTA");
    }
  }
  
  Serial.println("\n=== TODAS AS PARTIÇÕES APP DISPONÍVEIS ===");
  const esp_partition_t* part = NULL;
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
  int app_count = 0;
  while (it != NULL) 
  {
    part = esp_partition_get(it);
    app_count++;
    
    // Indicar status detalhado de cada partição
    String status = "";
    if (running_partition && part->address == running_partition->address) {
      status = " ← EXECUTANDO AGORA";
    }
    if (boot_partition && part->address == boot_partition->address) {
      status += " [BOOT]";
    }
    
    Serial.printf("APP%d: %s, Offset: 0x%06x, Size: %.2f MB%s\n", 
                  app_count, part->label, part->address, 
                  part->size / 1024.0 / 1024.0, status.c_str());
    it = esp_partition_next(it);
  }
  esp_partition_iterator_release(it);
  
  Serial.println("\n=== PARTIÇÕES DE DADOS ===");
  it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
  while (it != NULL) 
  {
    part = esp_partition_get(it);
    Serial.printf("DATA: %s, Offset: 0x%06x, Size: %.2f MB\n", 
                  part->label, part->address, part->size / 1024.0 / 1024.0);
    it = esp_partition_next(it);
  }
  esp_partition_iterator_release(it);
  
  // Análise de OTA
  Serial.println("\n=== STATUS DO SISTEMA OTA ===");
  if (app_count >= 2) {
    Serial.printf("✅ Sistema OTA configurado (%d partições APP encontradas)\n", app_count);
    Serial.println("   O sistema pode alternar entre partições para atualizações");
  } else {
    Serial.printf("⚠️ Sistema OTA limitado (apenas %d partição APP)\n", app_count);
    Serial.println("   Atualizações OTA podem não funcionar corretamente");
  }
  
  Serial.println("==============================================="); 
}



/**************************************************************
 * INICIALIZAÇÃO DO OTA 
 */
void setup_ota(void){

  // Inicializar mDNS primeiro
  if (!MDNS.begin(DISPOSITIVO_ID)) {
    Serial.println("OTA: Erro ao inicializar mDNS!");
  } else {
    Serial.println("OTA: mDNS inicializado com sucesso!");
    Serial.print("OTA: mDNS hostname: ");
    Serial.print(DISPOSITIVO_ID);
    Serial.println(".local");
  }

  // ArduinoOTA setup
  ArduinoOTA.setPort(3232); 
  ArduinoOTA.setHostname(DISPOSITIVO_ID); 
  ArduinoOTA.onStart([]() {
    Serial.println("Iniciando ArduinoOTA...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nArduinoOTA finalizada!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso ArduinoOTA: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro ArduinoOTA[%u]: ", error);
    if (error == OTA_AUTH_ERROR) 
      Serial.println("Falha de autenticação");
    else if (error == OTA_BEGIN_ERROR) 
      Serial.println("Falha ao iniciar");
    else if (error == OTA_CONNECT_ERROR) 
      Serial.println("Falha de conexão");
    else if (error == OTA_RECEIVE_ERROR) 
      Serial.println("Falha ao receber");
    else if (error == OTA_END_ERROR) 
      Serial.println("Falha ao finalizar");
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA: ArduinoOTA inicializado!");
  Serial.print("OTA: ArduinoOTA porta 3232 - IP: ");
  Serial.println(WiFi.localIP());
  
}


/**************************************************************
 * LOOP DO OTA
 */
void loop_ota() {  
  ArduinoOTA.handle();
}


/**************************************************************
 * INICIALIZAÇÃO DO MQTT
 */
void setup_mqtt()
{
   char mqtt_server_tmp[32];

   // Lê mqtt_server e port_mqtt na memoria NVS, se não existir usa os definidos em constants.cpp
   read_flash_string(KEY_MQTT_SERVER, mqtt_server_tmp, sizeof(mqtt_server_tmp));
   int port_mqtt = read_flash_int(KEY_MQTT_PORT);

   if (strlen(mqtt_server_tmp) > 0) {
       strncpy(MQTT_SERVER, mqtt_server_tmp, sizeof(MQTT_SERVER) - 1);
       MQTT_SERVER[sizeof(MQTT_SERVER) - 1] = '\0'; // Garantir terminação nula
   }

   if (port_mqtt > 0) {
       PORT_MQTT = port_mqtt;
   }
   
   client.setServer(MQTT_SERVER, PORT_MQTT);
   client.setCallback(callback); 
   Serial.println("MQTT: Serviço MQTT inicializado!    Servidor: " + String(MQTT_SERVER) + " Porta: " + String(PORT_MQTT));
   snprintf(topico, sizeof(topico), "%s/%s/%s/%s", CLIENTE, LOCAL, TIPO_EQUIPAMENTO, ID_EQUIPAMENTO);//  "presto/palhoca/prensa/001";
   Serial.println("Tópico MQTT: " + String(topico));
}
 

/**************************************************************
 * LOOP DO MQTT
 */
void loop_mqqt() {
  // put your main code here, to run repeatedly:  
  if (!client.connected()) {             
    reconnect(); 
  } 
  client.loop();  
  
}


/**************************************************************
 *  Callback do MQTT  
 */
void callback(char* topic, byte* payload, unsigned int length) 
{
   Serial.print("Mensagem recebida. Tópico: "); 
   Serial.print(topic); 
   Serial.print(".  : "); 
   String message; 
   for (unsigned int i = 0; i < length; i++) 
   { 
    message += (char)payload[i]; 
   } 
   Serial.println(message); 

   //Reiniciar dispositivo
  if (String(topic) == String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO) + "/" + String(ID_EQUIPAMENTO) + "/reboot_") {  
    Serial.println("Reiniciando o sistema Tópico MQTT Reboot_...");
    delay(1000);
    ESP.restart(); // Reinicia o ESP32
  }

  //Forçar reconexão WiFi
  if (String(topic) == String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO) + "/" + String(ID_EQUIPAMENTO) + "/reconnect_wifi_") {  
    Serial.println("Forçando reconexão WiFi... Tópico MQTT Reconnect_WiFi_");
    WiFi.disconnect();
    delay(1000);
    setup_wifi(); // Reconecta com as novas configurações
  }

  //Forçar reconexão MQTT
  if (String(topic) == String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO) + "/" + String(ID_EQUIPAMENTO) + "/reconnect_mqtt_") {
    Serial.println("Forçando reconexão MQTT... Tópico MQTT Reconnect_MQTT_");
    client.disconnect();
    delay(1000);
    setup_mqtt(); // Reconecta com as novas configurações
  }

  // Envia leitura do sistema
  if (String(topic) == String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO) + "/" + String(ID_EQUIPAMENTO) + "/info") {
    Serial.println("Enviando informações do sistema conforme... Resposta topico INFO");
    bool result = mqtt_send_info();  
  }

  // Envia informações do dispositivo
  if (String(topic) == String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO) + "/" + String(ID_EQUIPAMENTO) + "/settings_device") {
    Serial.println("Enviando informações do dispositivo... Resposta tópico settings_device");
    bool result = mqtt_send_settings_device();    
  }

  // Envia configurações do equipamento
  if (String(topic) == String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO) + "/" + String(ID_EQUIPAMENTO) + "/settings_equip") {
    Serial.println("Enviando configurações do equipamento... Resposta tópico settings_equip");
    bool result = mqtt_send_settings_equip();    
  }

  // Envia configurações do cliente
  if (String(topic) == String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO) + "/" + String(ID_EQUIPAMENTO) + "/settings_client") {
    Serial.println("Enviando configurações do cliente... Resposta tópico settings_client");
    bool result = mqtt_send_settings_client();    
  }

  // Atualiza configurações via MQTT
  if (String(topic) == String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO) + "/" + String(ID_EQUIPAMENTO) + "/settings") {
        DynamicJsonDocument doc(1024); // Aumentei o tamanho para comportar mais campos
        DeserializationError error = deserializeJson(doc, message);
        if (!error) {
            bool mqtt_changed = false;
            bool wifi_changed = false;
              
         /*   { "level_max", 20
                "level_min", 10
                "sample_time_s", 30
                "wifi_ssid", "meu_wifi"
                "wifi_password", "minha_senha"
                "mqtt_server", "mqtt.exemplo.com",
                "mqtt_port", 1883,
                "mqtt_user", "usuario",
                "mqtt_password", "senha"
              }

            adriano/floripa/reservatorio/002/settings
              {
                "cliente":"adrianoo",
                "local":"floripaa",
                "tipo_equip":"reservatorioo",
                "id_equip":"0022",
                "nome_equip":"cx_aguaa"
              }

          */

            // =============================== CONFIGURAÇÕES DE NÍVEL DO RESERVATÓRIO ===
            if (doc.containsKey("level_max")) {
                float level_max_tmp = doc["level_max"];
                save_flash_float(KEY_LEVEL_MAX, level_max_tmp);  // Usa função centralizada
                level_max = level_max_tmp; // Atualiza a variável global imediatamente
                Serial.println("✅ Salvo level_max: " + String(level_max_tmp) + " cm");
            }
            if (doc.containsKey("level_min")) {
                float level_min_tmp = doc["level_min"];
                save_flash_float(KEY_LEVEL_MIN, level_min_tmp);  // Usa função centralizada
                level_min = level_min_tmp; // Atualiza a variável global imediatamente
                Serial.println("✅ Salvo level_min: " + String(level_min_tmp) + " cm");
            }
            if (doc.containsKey("sample_time_s")) {
                int sample_time_tmp = doc["sample_time_s"];
                save_flash_int(KEY_SAMPLE_TIME_S, sample_time_tmp);  // Usa função centralizada
                SAMPLE_INTERVAL = sample_time_tmp; // Atualiza a variável global imediatamente
                Serial.println("✅ Salvo sample_time_s: " + String(sample_time_tmp) + " segundos");
            }
            if (doc.containsKey("filter_threshold")) {
                float filter_threshold_tmp = doc["filter_threshold"];
                save_flash_float(KEY_FILTER_THRESHOLD, filter_threshold_tmp);  // Usa função centralizada
                filter_threshold = filter_threshold_tmp; // Atualiza a variável global imediatamente
                Serial.println("✅ Salvo filter_threshold: " + String(filter_threshold_tmp) + "%");
            }

            // =============================== CONFIGURAÇÕES DE DISPOSITIVO ===
            
            //CLIENTE
            if (doc.containsKey("cliente")) {
                String cliente_tmp = doc["cliente"];
                save_flash_string(KEY_CLIENTE, cliente_tmp.c_str());  // Salva na flash
                // ✅ Não podemos alterar char* diretamente - valor será carregado no próximo boot
                Serial.println("✅ Salvo CLIENTE: " + cliente_tmp + " (ativo no próximo boot)");
            }
            // LOCAL
            if (doc.containsKey("local")) {
                String local_tmp = doc["local"];
                save_flash_string(KEY_LOCAL, local_tmp.c_str());  // Salva na flash
                // ✅ Não podemos alterar char* diretamente - valor será carregado no próximo boot
                Serial.println("✅ Salvo LOCAL: " + local_tmp + " (ativo no próximo boot)");
            }
            // TIPO EQUIPAMENTO
            if (doc.containsKey("tipo_equip")) {
                String tipo_equip_tmp = doc["tipo_equip"];
                save_flash_string(KEY_TIPO_EQUIP, tipo_equip_tmp.c_str());  // Salva na flash
                // ✅ Não podemos alterar char* diretamente - valor será carregado no próximo boot
                Serial.println("✅ Salvo TIPO_EQUIPAMENTO: " + tipo_equip_tmp + " (ativo no próximo boot)");
            }
            // ID EQUIPAMENTO
            if (doc.containsKey("id_equip")) {
                String id_equip_tmp = doc["id_equip"];
                save_flash_string(KEY_ID_EQUIP, id_equip_tmp.c_str());  // Salva na flash
                // ✅ Não podemos alterar char* diretamente - valor será carregado no próximo boot
                Serial.println("✅ Salvo ID_EQUIPAMENTO: " + id_equip_tmp + " (ativo no próximo boot)");
            }
            // NOME EQUIPAMENTO
            if (doc.containsKey("nome_equip")) {
                String nome_equip_tmp = doc["nome_equip"];
                save_flash_string(KEY_NOME_EQUIP, nome_equip_tmp.c_str());  // Usa função centralizada
                strcpy(NOME_EQUIPAMENTO, nome_equip_tmp.c_str()); // Atualiza variável global
                Serial.println("✅ Salvo nome_equip: " + nome_equip_tmp);
            }

            // DISPOSITIVO_ID
            if (doc.containsKey("dispositivo_id")) {
                String dispositivo_id_tmp = doc["dispositivo_id"];
                save_flash_string(KEY_DISPOSITIVO_ID, dispositivo_id_tmp.c_str());
                strcpy(DISPOSITIVO_ID, dispositivo_id_tmp.c_str());
                Serial.println("✅ Salvo dispositivo_id: " + dispositivo_id_tmp);
            }

            // FABRICANTE MAQUINA
            if (doc.containsKey("fabricante_maquina")) {
                String fabricante_maquina_tmp = doc["fabricante_maquina"];
                save_flash_string(KEY_FABRICANTE_MAQUINA, fabricante_maquina_tmp.c_str());
                strcpy(FABRICANTE_MAQUINA, fabricante_maquina_tmp.c_str());
                Serial.println("✅ Salvo fabricante_maquina: " + fabricante_maquina_tmp);
            }

            // MODELO MAQUINA
            if (doc.containsKey("modelo_maquina")) {
                String modelo_maquina_tmp = doc["modelo_maquina"];
                save_flash_string(KEY_MODELO_MAQUINA, modelo_maquina_tmp.c_str());
                strcpy(MODELO_MAQUINA, modelo_maquina_tmp.c_str());
                Serial.println("✅ Salvo modelo_maquina: " + modelo_maquina_tmp);
            }

            // TIPO SENSOR
            if (doc.containsKey("tipo_sensor")) {
                String tipo_sensor_tmp = doc["tipo_sensor"];
                save_flash_string(KEY_TIPO_SENSOR, tipo_sensor_tmp.c_str());
                strcpy(TIPO_SENSOR, tipo_sensor_tmp.c_str());
                Serial.println("✅ Salvo tipo_sensor: " + tipo_sensor_tmp);
            }

            // OBSERVACAO DEVICE INFO
            if (doc.containsKey("observacao_device_info")) {
                String observacao_device_info_tmp = doc["observacao_device_info"];
                save_flash_string(KEY_OBSERVACAO_DEVICE_INFO, observacao_device_info_tmp.c_str());
                strcpy(OBSERVACAO_DEVICE_INFO, observacao_device_info_tmp.c_str());
                Serial.println("✅ Salvo observacao_device_info: " + observacao_device_info_tmp);
            }

            // OBSERVACAO SETTINGS
            if (doc.containsKey("observacao_settings")) {
                String observacao_settings_tmp = doc["observacao_settings"];
                save_flash_string(KEY_OBSERVACAO_SETTINGS, observacao_settings_tmp.c_str());
                strcpy(OBSERVACAO_SETTINGS, observacao_settings_tmp.c_str());
                Serial.println("✅ Salvo observacao_settings: " + observacao_settings_tmp);
            }

            // OBSERVACAO READINGS
            if (doc.containsKey("observacao_readings")) {
                String observacao_readings_tmp = doc["observacao_readings"];
                save_flash_string(KEY_OBSERVACAO_READINGS, observacao_readings_tmp.c_str());
                strcpy(OBSERVACAO_READINGS, observacao_readings_tmp.c_str());
                Serial.println("✅ Salvo observacao_readings: " + observacao_readings_tmp);
            }

            // LINHA
            if (doc.containsKey("linha")) {
                String linha_tmp = doc["linha"];
                save_flash_string(KEY_LINHA, linha_tmp.c_str());
                strcpy(LINHA, linha_tmp.c_str());
                Serial.println("✅ Salvo linha: " + linha_tmp);
            }

            // PLACA_SOC
            if (doc.containsKey("placa_soc")) {
                String placa_soc_tmp = doc["placa_soc"];
                save_flash_string(KEY_PLACA_SOC, placa_soc_tmp.c_str());
                strcpy(PLACA_SOC, placa_soc_tmp.c_str());
                Serial.println("✅ Salvo placa_soc: " + placa_soc_tmp);
            }

            // FABRICANTE_SENSOR
            if (doc.containsKey("fabricante_sensor")) {
                String fabricante_sensor_tmp = doc["fabricante_sensor"];
                save_flash_string(KEY_FABRICANTE_SENSOR, fabricante_sensor_tmp.c_str());
                strcpy(FABRICANTE_SENSOR, fabricante_sensor_tmp.c_str());
                Serial.println("✅ Salvo fabricante_sensor: " + fabricante_sensor_tmp);
            }

            // MODELO_SENSOR
            if (doc.containsKey("modelo_sensor")) {
                String modelo_sensor_tmp = doc["modelo_sensor"];
                save_flash_string(KEY_MODELO_SENSOR, modelo_sensor_tmp.c_str());
                strcpy(MODELO_SENSOR, modelo_sensor_tmp.c_str());
                Serial.println("✅ Salvo modelo_sensor: " + modelo_sensor_tmp);
            }

            // VERSAO_HARDWARE
            if (doc.containsKey("versao_hardware")) {
                String versao_hardware_tmp = doc["versao_hardware"];
                save_flash_string(KEY_VERSAO_HARDWARE, versao_hardware_tmp.c_str());
                strcpy(VERSAO_HARDWARE, versao_hardware_tmp.c_str());
                Serial.println("✅ Salvo versao_hardware: " + versao_hardware_tmp);
            }

            // DATA_INSTALACAO
            if (doc.containsKey("data_instalacao")) {
                String data_instalacao_tmp = doc["data_instalacao"];
                save_flash_string(KEY_DATA_INSTALACAO, data_instalacao_tmp.c_str());
                strcpy(DATA_INSTALACAO, data_instalacao_tmp.c_str());
                Serial.println("✅ Salvo data_instalacao: " + data_instalacao_tmp);
            }


            // ==================================== CONFIGURAÇÕES DE WIFI ===
            if (doc.containsKey("wifi_ssid")) {
                String wifi_ssid_tmp = doc["wifi_ssid"];
                save_flash_string(KEY_WIFI_SSID, wifi_ssid_tmp.c_str());  // Usa função centralizada
                Serial.println("✅ Salvo WiFi SSID: " + wifi_ssid_tmp);
                wifi_changed = true;
            }
            if (doc.containsKey("wifi_password")) {
                String wifi_pass_tmp = doc["wifi_password"];
                save_flash_string(KEY_WIFI_PASS, wifi_pass_tmp.c_str());  // Usa função centralizada
                Serial.println("✅ Salvo WiFi Password: [HIDDEN]");
                wifi_changed = true;
            }

            // ===================================== CONFIGURAÇÕES DE MQTT ===
            if (doc.containsKey("mqtt_server")) {
                String mqtt_server_tmp = doc["mqtt_server"];
                save_flash_string(KEY_MQTT_SERVER, mqtt_server_tmp.c_str());  // Usa função centralizada
                strncpy(MQTT_SERVER, mqtt_server_tmp.c_str(), sizeof(MQTT_SERVER) - 1);
                MQTT_SERVER[sizeof(MQTT_SERVER) - 1] = '\0'; // Garantir terminação nula
                Serial.println("✅ Salvo MQTT Server: " + mqtt_server_tmp);
                mqtt_changed = true;
            }
            if (doc.containsKey("mqtt_port")) {
                int mqtt_port_tmp = doc["mqtt_port"];
                save_flash_int(KEY_MQTT_PORT, mqtt_port_tmp);  // Usa função centralizada
                PORT_MQTT = mqtt_port_tmp;
                Serial.println("✅ Salvo MQTT Port: " + String(mqtt_port_tmp));
                mqtt_changed = true;
            }
            if (doc.containsKey("mqtt_user")) {
                String mqtt_user_tmp = doc["mqtt_user"];
                save_flash_string(KEY_MQTT_USER, mqtt_user_tmp.c_str());  // Usa função centralizada
                strncpy(MQTT_USERNAME, mqtt_user_tmp.c_str(), sizeof(MQTT_USERNAME) - 1);
                MQTT_USERNAME[sizeof(MQTT_USERNAME) - 1] = '\0';
                Serial.println("✅ Salvo MQTT User: " + mqtt_user_tmp);
                mqtt_changed = true;
            }
            if (doc.containsKey("mqtt_password")) {
                String mqtt_pass_tmp = doc["mqtt_password"];
                save_flash_string(KEY_MQTT_PASS, mqtt_pass_tmp.c_str());  // Usa função centralizada
                strncpy(MQTT_PASSWORD, mqtt_pass_tmp.c_str(), sizeof(MQTT_PASSWORD) - 1);
                MQTT_PASSWORD[sizeof(MQTT_PASSWORD) - 1] = '\0';
                Serial.println("✅ Salvo MQTT Password: [HIDDEN]");
                mqtt_changed = true;
            }
            
            // === APLICAÇÃO DAS MUDANÇAS ===
            
            // Recarrega configurações e reseta o filtro percentual se necessário
            #ifdef SENSOR_WATER_LEVEL
                if (doc.containsKey("level_max") || doc.containsKey("level_min")) {
                    reset_percentual_filter(); // Reseta filtro para aplicar novos limites
                    Serial.println("🔄 Filtro percentual resetado para aplicar novos limites");
                }
            #endif
            
            // Envia confirmação via MQTT
            mqtt_send_settings_confirmation();
            
            // Informa sobre necessidade de reconexão
            if (mqtt_changed) {
                Serial.println("⚠️  MQTT configurações alteradas - Reconexão necessária");
                Serial.println("   Use o comando 'reconnect_mqtt' ou reinicie o dispositivo");
            }
            if (wifi_changed) {
                Serial.println("⚠️  WiFi configurações alteradas - Reconexão necessária");
                Serial.println("   Use o comando 'reconnect_wifi' ou reinicie o dispositivo");
            }
            
        } else {
            Serial.println("❌ Erro ao analisar a mensagem de configurações JSON.");
        }
    }


 /* Exemplo de mensagem esperada: 
        {"server":"192.168.100.4",
         "port":1883,
         "username":"Adriano",
         "password":"Rafa1404"}
*/
  if (String(topic) == "config_mqtt") {
    Serial.println("Reconfigurando o MQTT conforme comando recebido...");
    // Analisa a mensagem JSON
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, message);
    // Verifica se houve erro na análise
    if (!error) {
        strncpy(MQTT_SERVER, doc["server"] | "", sizeof(MQTT_SERVER));
        PORT_MQTT = doc["port"] | 1883;
        strncpy(MQTT_USERNAME, doc["username"] | "", sizeof(MQTT_USERNAME));
        strncpy(MQTT_PASSWORD, doc["password"] | "", sizeof(MQTT_PASSWORD));
        Serial.println("Configurações do MQTT atualizadas:");
        Serial.println("Servidor: " + String(MQTT_SERVER));
        Serial.println("Porta: " + String(PORT_MQTT));
        Serial.println("Usuário: " + String(MQTT_USERNAME));
        Serial.println("Senha: " + String(MQTT_PASSWORD));
    // Salve na NVS usando funções centralizadas
    save_flash_string(KEY_MQTT_SERVER, MQTT_SERVER);
    save_flash_int(KEY_MQTT_PORT, PORT_MQTT);
    save_flash_string(KEY_MQTT_USER, MQTT_USERNAME);
    save_flash_string(KEY_MQTT_PASS, MQTT_PASSWORD);
    } else {
        Serial.println("Erro ao analisar a mensagem de configuração do MQTT.");
    }
  }

  /* Envio de informações do MQTT */
  if (String(topic) == "info_mqtt") {
    Serial.println("Enviando informações do MQTT conforme comando recebido...");
    Serial.println("Servidor: " + String(MQTT_SERVER));
    Serial.println("Porta: " + String(PORT_MQTT));
    Serial.println("Usuário: " + String(MQTT_USERNAME));
    Serial.println("Senha: " + String(MQTT_PASSWORD));
  }


  /* Grava o IP recebido na NVS */ 
  if (String(topic) == "config_ip") {    
    Serial.println("Atualizando configurações de rede: " + message);
    save_flash_string(KEY_IP, message.c_str());
  }

  if (String(topic) == "info_ip") {        
      char ip_salvo[32];
      read_flash_string(KEY_IP, ip_salvo, sizeof(ip_salvo));      
      Serial.println("IP salvo na NVS: " + String(ip_salvo));      
  } 

}


/**************************************************************
 *  RECONEXÃO DO MQTT COM SEUS RESPECTIVOS TÓPICOS
 */
void reconnect() 
{ 
  int qnt = 3;
  while (!client.connected() && qnt > 1) 
  { 
    qnt--;
    Serial.print("Tentando conectar ao MQTT..."); 
    if (client.connect(DISPOSITIVO_ID)) //Nome do MQTT na rede
    {       
      Serial.println("Conectado no MQTT com nome: " + String(DISPOSITIVO_ID));   
      client.subscribe("info",1);
      client.subscribe("settings",1);
      client.subscribe("config_mqtt",1);
      client.subscribe("config_ip",1);
      
      // Construir tópicos específicos hierárquicos      
      String topico_dispositivo = String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO) + "/" + String(ID_EQUIPAMENTO);
      String topico_settings = topico_dispositivo + "/settings";
      String topico_reboot = topico_dispositivo + "/reboot_";
      String topico_info = topico_dispositivo + "/info";      
      String topico_settings_device = topico_dispositivo + "/settings_device";
      String topico_settings_client = topico_dispositivo + "/settings_client";
      String topico_settings_equip = topico_dispositivo + "/settings_equip";
      String topico_local = String(CLIENTE) + "/" + String(LOCAL);
      String topico_tipo = String(CLIENTE) + "/" + String(LOCAL) + "/" + String(TIPO_EQUIPAMENTO);
      
      
      //client.subscribe("presto/floripa/forno/001")      
      client.subscribe(topico,1); // Inscreve-se no tópico geral do equipamento
      client.subscribe(CLIENTE,1); // Inscreve-se no tópico do cliente específico      
      client.subscribe(topico_local.c_str(), 1); // Inscreve-se no tópico do local específico
      client.subscribe(topico_tipo.c_str(), 1); // Inscreve-se no tópico do tipo de equipamento específico
      client.subscribe(topico_settings.c_str(), 1); // ✅ TÓPICO SETTINGS ESPECÍFICO
      client.subscribe(topico_reboot.c_str(), 1); // ✅ TÓPICO REBOOT ESPECÍFICO
      client.subscribe(topico_info.c_str(), 1); // ✅ TÓPICO INFO
      client.subscribe(topico_settings_device.c_str(), 1); // ✅ TÓPICO SETTINGS DEVICE
      client.subscribe(topico_settings_client.c_str(), 1); // ✅ TÓPICO
      client.subscribe(topico_settings_equip.c_str(), 1); // ✅ TÓPICO SETTINGS EQUIP
      
      Serial.println("Inscrito nos tópicos com sucesso!");      
      Serial.println("Tópico geral: " + String(topico));
      Serial.println("Tópico settings: " + topico_settings);
      Serial.println("Tópico reboot: " + topico_reboot);
      Serial.println("Tópico info: " + topico_info);
      Serial.println("Tópico settings device: " + topico_settings_device);
      Serial.println("Tópico settings client: " + topico_settings_client);
      Serial.println("Tópico settings equip: " + topico_settings_equip);

    } 
    else 
    { 
      Serial.print("falhou, rc="); 
      Serial.print(client.state()); 
      Serial.println(" tentando novamente 500 mseg"); 
      delay(500); 
    } 
  } 
}

/**********************************************************************************************
*     ENVIA AS INFORMAÇÕES PARA O PROTOCOLO MQTT
*
* Exemplo de JSON enviado:
*{   "equipamento":"teste",
*    "hora":"2025-07-17 21:11:17",
*    "id_leitura":"3",
*    "observacao":""  
* }
*/
bool mqtt_send_data(const char* nome_equipamento, const char* horario, long id_leitura, const char* observacao) {
    if (!client.connected()) {
        return false;
    }
    client.loop();
    char time_str_buffer[16];           char* timestamp = get_time_str(time_str_buffer, sizeof(time_str_buffer));    
    long timestamp2 = atol(time_str_buffer); // Converte string para long


    StaticJsonDocument<256> doc;
    doc["table"] = "device_readings";
    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = timestamp2;
    doc["wifi_rssi_dbm"] = WiFi.RSSI();    
    doc["takt_time_id"] = id_leitura;
    doc["note"] = observacao;

    char jsonBuffer[256] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0

    // web server update
    idBatida = id_leitura; // Atualiza o ID da batida
    Serial.println("MQTT: Dados enviados.." + String(CLIENTE));    
    //handleRoot(); // Atualiza a página web após enviar os dados


    return result;
}

// Versão anterior removida - mantendo apenas a versão corrigida no final do arquivo

/**********************************************************************************************
*     ENVIA e SALVA AS INFORMAÇÕES DE SETTINGS PARA O PROTOCOLO MQTT
*
* Exemplo de JSON enviado:
*{   "table": "device_settings",
*    "device_id": DISPOSITIVO_ID,
*    "timestamp": timestamp2,
*    "level_min_cm": level_min,
*    "level_max_cm": level_max,
*    "sample_time_s": SAMPLE_INTERVAL,
*    "notes":""  
* }
*/

bool mqtt_send_settings(){//const char* nome_equipamento, const char* horario, long id_leitura, const char* observacao) {
    if (!client.connected()) {
       return false;
    }
    client.loop();
    char time_str_buffer[16];           char* timestamp = get_time_str(time_str_buffer, sizeof(time_str_buffer));    
    long timestamp2 = atol(time_str_buffer); // Converte string para long

    StaticJsonDocument<512> doc;

    doc["table"] = "device_settings";
    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = timestamp2;    
    doc["level_min_cm"] = level_min;
    doc["level_max_cm"] = level_max;    
    doc["sample_time_s"] = SAMPLE_INTERVAL;
    doc["filter_threshold_pct"] = filter_threshold;    
    doc["notes"] = OBSERVACAO_SETTINGS;

    char jsonBuffer[512] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0
    Serial.println("MQTT: Dados settings enviados.." + String(topico));
    Serial.println("JSON enviado: " + String(jsonBuffer)); 
    return result;

}

/**********************************************************************************************
*     ENVIA AS INFORMAÇÕES PARA O PROTOCOLO MQTT
*
* Exemplo de JSON enviado:
*{   "equipamento":"teste",
*    "hora":"2025-07-17 21:11:17",
*    "id_leitura":"3",
*    "observacao":""  
* }
*/

bool mqtt_send_info(){//const char* nome_equipamento, const char* horario, long id_leitura, const char* observacao) {
    if (!client.connected()) {
       return false;
    }
    client.loop();
    char time_str_buffer[16];           char* timestamp = get_time_str(time_str_buffer, sizeof(time_str_buffer));    
    long timestamp2 = atol(time_str_buffer); // Converte string para long

    StaticJsonDocument<1024> doc;

    doc["table"] = "device_info";
    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = timestamp2;
    doc["client"] = CLIENTE;
    doc["device_id"] = DISPOSITIVO_ID;    
    doc["plant"] = LOCAL;
    doc["line"] = LINHA;    
    doc["machine_type"] = TIPO_EQUIPAMENTO;
    doc["machine_model"]  = MODELO_MAQUINA;
    doc["machine_manufacturer"] = FABRICANTE_MAQUINA;
    doc["sensor_type"] = TIPO_SENSOR;
    doc["sensor_manufacturer"] = FABRICANTE_SENSOR;
    doc["sensor_model"] = MODELO_SENSOR;
    doc["ip_address"] = WiFi.localIP().toString();
    doc["mac_address"] = WiFi.macAddress();
    doc["firmware_version"] = VERSION;
    doc["hardware_version"] = VERSAO_HARDWARE;
    doc["installation_date"] = DATA_INSTALACAO;    
    doc["sample_time_s"] = SAMPLE_INTERVAL;    //ToDo: colocar no topico: settings
    doc["notes"] = OBSERVACAO_DEVICE_INFO;
    

    char jsonBuffer[1024] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0
    Serial.println("MQTT: Dados info enviados.." + String(topico));
    Serial.println("JSON enviado: " + String(jsonBuffer)); 
    return result;

/* Exemplo de JSON enviado: 
{
    "table" = "device_info";
    "device_id" = "presto-plh-l01-rsv-001";
    "timestamp" = 1759253363;
    "client" = "presto";
    "plant" = "palhoca";
    "line" = "l01";
    "machine_type" = "reservatorio";
    "machine_manufacturer" = "Presto";
    "machine_model" = "X1000";
    "machine_serial" = "123456789";
    "machine_firmware" = "v1.0.0";

    "sensor_type" = "ultrasonic"; 
    "sensor_manufacturer" = "SensorCo";
    "sensor_model" = "UltraX";
    "sensor_serial" = "987654321";
    "sensor_firmware" = "v2.1.0";
    "sample_time_s" = 60;
    
    "location" = "Instalação interna";
    "installation_date" = "2023-10-01";
    "last_maintenance_date" = "2024-06-15";
    "next_maintenance_date" = "2024-12-15";
    "operational_status" = "Ativo";

    "last_calibration_date" = "2024-01-10";
    "next_calibration_date" = "2025-01-10";
    
    "warranty_expiration_date" = "2025-10-01";
    "ip_address" = "192.168.1.100";
    "mac_address" = "00:1A:2B:3C:4D:5E";
    "firmware_version" = "v1.0.0";
    "hardware_version" = "v1.0";
    
    
    "status" = "OK";
    "error_code" = 0;
    "notes" = "Teste adriano"

*/
}

/*
*    Retorna a Informação do dispositivo
*/
bool mqtt_send_settings_device() {
    if (!client.connected()) {
       return false;
    }
    client.loop();
    char time_str_buffer[16];           char* timestamp = get_time_str(time_str_buffer, sizeof(time_str_buffer));    
    long timestamp2 = atol(time_str_buffer); // Converte string para long

    StaticJsonDocument<512> doc;

    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = timestamp2;
    doc["sensor"] = TIPO_SENSOR;
    doc["manufacturer_sensor"] = FABRICANTE_SENSOR;
    doc["sensor_model"] = MODELO_SENSOR;
    doc["board_soc"] = PLACA_SOC;
    doc["hardware_version"] = VERSAO_HARDWARE;
    doc["firmware_version"] = VERSION;
    doc["installation_date"] = DATA_INSTALACAO;


    char jsonBuffer[512] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0
    Serial.println("MQTT: settings_device enviado.. Topico:  " + String(topico));   
    Serial.println("JSON enviado: " + String(jsonBuffer));          
    return result;
}


/*
*  Retorna a Informação do cliente
*/
bool mqtt_send_settings_client() {
    if (!client.connected()) {
       return false;
    }
    client.loop();
    char time_str_buffer[16];           char* timestamp = get_time_str(time_str_buffer, sizeof(time_str_buffer));    
    long timestamp2 = atol(time_str_buffer); // Converte string para long

    StaticJsonDocument<512> doc;

    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = timestamp2;
    doc["client"] = CLIENTE;
    doc["location"] = LOCAL;
    doc["line"] = LINHA;
    
    

    char jsonBuffer[512] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0
    Serial.println("MQTT: settings_cliente enviado.. Topico:  " + String(topico));         
    Serial.println("JSON enviado: " + String(jsonBuffer));    
    return result;
  }


/*
* Retorna a Informação do equipamento
*/
bool mqtt_send_settings_equip() {
    if (!client.connected()) {
       return false;
    }
    client.loop();
    char time_str_buffer[16];           char* timestamp = get_time_str(time_str_buffer, sizeof(time_str_buffer));    
    long timestamp2 = atol(time_str_buffer); // Converte string para long

    StaticJsonDocument<512> doc;

    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = timestamp2;
    doc["equipment_type"] = TIPO_EQUIPAMENTO;
    doc["equipment_id"] = ID_EQUIPAMENTO;
    doc["equipment_name"] = NOME_EQUIPAMENTO;    
    doc["machine_manufacturer"] = FABRICANTE_MAQUINA;
    doc["machine_model"] = MODELO_MAQUINA; 



    char jsonBuffer[512] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0
    Serial.println("MQTT: settings_equip enviado.. Topico:  " + String(topico));           
    Serial.println("JSON enviado: " + String(jsonBuffer));  
    return result;
  }



 
 bool mqtt_send_readings() {
    if (!client.connected()) {
        return false;
    }
    client.loop();
    char time_str_buffer[16];           char* timestamp = get_time_str(time_str_buffer, sizeof(time_str_buffer));    
    long timestamp2 = atol(time_str_buffer); // Converte string para long

    StaticJsonDocument<512> doc;    

    doc["table"] = "device_readings";
    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = timestamp2;
    doc["wifi_rssi_dbm"] = WiFi.RSSI();    
    doc["level_effective_cm"] = roundf((level_min - level_max) * 100) / 100.0; // altura útil do reservatório        
    doc["level_available_cm"] = roundf((level_min - altura_medida) * 100) / 100.0; // nível disponível no reservatório
    doc["level_available_%"] = roundf(percentual_reservatorio * 100) / 100.0;		    
    doc["temp_c"] =  roundf(temperatura * 100) / 100.0;
    doc["humidity_%"] = roundf(humidade * 100) / 100.0; 
    doc["notes"] = OBSERVACAO_READINGS;
    

    /* Exemplo de JSON enviado:
-{
-    "table" = "device_readings";
-    "device_id" = "presto-plh-l01-rsv-001";
-    "timestamp" = 1759253363;
-    "wifi_rssi_dbm" = -67;
-    "zigbee_rssi_dbm" = 0;
-    "wifi_rssi_%" = 33;
-    "zigbee_rssi_%" = 0;
-    "takt_time_id" = 34564;
-    "temp_c" =  23.45;
-    "temp_f" = 74.21;
-    "humidity_%" = 65.50;
-    "level_h_cm" = 150.25;
-    "level_usage_%" = 75.50;
-    "level_usage_cm" = 0.0;
-    "level_usage_l" = 0.0;
-    "level_usage_m3" = 0.0;
-    "pressure_psi" = 33.5;
-    "pressure_kpa" = 231.0;
-    "pressure_bar" = 2.31;
-    "pressure_mca" = 23.5;
-    "flow_l_min" = 0.0;
-    "flow_m3_h" = 0.0;
-    "voltage_v" = 0;
-    "voltage_bat_v" = 0;
-    "current_a" = 0;
-    "power_w" = 0;
-    "energy_wh" = 0;
-    "energy_kwh" = 0;
-    "frequency_hz" = 0;
-    "power_factor" = 0;
-    "rpm" = 0;
-    "vibration_mm_s" = 0;
-    "vibration_g" = 0;
-    "status" = "OK";
-    "error_code" = 0;
-    "notes" = "Nenhuma observação"
-}  */

    char jsonBuffer[512] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0
    Serial.println("MQTT: device_readings enviado.. Topico:  " + String(topico));
    Serial.println("JSON enviado: " + String(jsonBuffer));            
    return result;
}

/**************************************************************
 * MOSTRAR INFORMAÇÕES DAS PARTIÇÕES OTA
 */
void show_ota_info() {
  Serial.println("=== INFORMAÇÕES DETALHADAS DAS PARTIÇÕES OTA ===");
  
  const esp_partition_t* running = esp_ota_get_running_partition();
  const esp_partition_t* boot_partition = esp_ota_get_boot_partition();
  const esp_partition_t* next_update = esp_ota_get_next_update_partition(NULL);
  
  // Informações da partição atual (em execução)
  if (running) {
    Serial.printf("🟢 PARTIÇÃO EM EXECUÇÃO: %s\n", running->label);
    Serial.printf("   Endereço: 0x%06x\n", running->address);
    Serial.printf("   Tamanho: %u bytes (%.2f MB)\n", running->size, running->size / 1024.0 / 1024.0);
    Serial.printf("   Tipo: %d, Subtipo: %d\n", running->type, running->subtype);
  } else {
    Serial.println("❌ ERRO: Não foi possível obter a partição em execução!");
  }
  
  // Informações da partição de boot
  if (boot_partition) {
    Serial.printf("🔵 PARTIÇÃO DE BOOT: %s\n", boot_partition->label);
    Serial.printf("   Endereço: 0x%06x\n", boot_partition->address);
    Serial.printf("   Tamanho: %u bytes (%.2f MB)\n", boot_partition->size, boot_partition->size / 1024.0 / 1024.0);
    Serial.printf("   Tipo: %d, Subtipo: %d\n", boot_partition->type, boot_partition->subtype);
  } else {
    Serial.println("❌ ERRO: Não foi possível obter a partição de boot!");
  }
  
  // Informações da próxima partição OTA
  if (next_update) {
    Serial.printf("🟡 PRÓXIMA PARTIÇÃO OTA: %s\n", next_update->label);
    Serial.printf("   Endereço: 0x%06x\n", next_update->address);
    Serial.printf("   Tamanho: %u bytes (%.2f MB)\n", next_update->size, next_update->size / 1024.0 / 1024.0);
    Serial.printf("   Tipo: %d, Subtipo: %d\n", next_update->type, next_update->subtype);
  } else {
    Serial.println("❌ ERRO: Não foi possível obter a próxima partição OTA!");
  }
  
  // Análise de consistência
  Serial.println("\n=== ANÁLISE DE CONSISTÊNCIA ===");
  if (running && boot_partition) {
    if (running->address == boot_partition->address) {
      Serial.println("✅ ESTADO NORMAL: Partições running e boot são idênticas");
      Serial.println("   O sistema está executando da partição correta");
    } else {
      Serial.println("⚠️ INCONSISTÊNCIA DETECTADA!");
      Serial.println("   A partição em execução é diferente da partição de boot");
      Serial.println("   Possíveis causas:");
      Serial.println("   - Último OTA não foi ativado corretamente");
      Serial.println("   - Falha na gravação da nova partição de boot");
      Serial.println("   - Sistema reverteu para partição anterior por erro");
    }
  }
  
  // Informações adicionais do sistema
  Serial.println("\n=== INFORMAÇÕES DO SISTEMA ===");
  Serial.printf("Versão do Firmware: %s\n", VERSION);
  Serial.printf("Espaço livre na Flash: %u bytes (%.2f MB)\n", 
                ESP.getFreeSketchSpace(), ESP.getFreeSketchSpace() / 1024.0 / 1024.0);
  Serial.printf("Tamanho do Sketch atual: %u bytes (%.2f MB)\n", 
                ESP.getSketchSize(), ESP.getSketchSize() / 1024.0 / 1024.0);
  Serial.printf("MD5 do Firmware atual: %s\n", ESP.getSketchMD5().c_str());
  
  // Verificar se há espaço suficiente para OTA
  if (ESP.getFreeSketchSpace() > ESP.getSketchSize()) {
    Serial.println("✅ Espaço suficiente para OTA");
  } else {
    Serial.println("❌ AVISO: Espaço insuficiente para OTA!");
  }
  
  Serial.println("===============================================");
}

/**************************************************************
 * ENVIO DE CONFIRMAÇÃO DE CONFIGURAÇÕES VIA MQTT
 */
bool mqtt_send_settings_confirmation() {
    if (!client.connected()) {
       return false;
    }
    
    client.loop();
    char time_str_buffer[16];
    char* timestamp = get_time_str(time_str_buffer, sizeof(time_str_buffer));

    StaticJsonDocument<1024> doc; // Aumentei para comportar mais campos

    doc["table"] = "settings_confirmation";
    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = atol(time_str_buffer);
    
    // Configurações do reservatório
    doc["level_max_cm"] = level_max;
    doc["level_min_cm"] = level_min;
    doc["level_effective_cm"] = roundf((level_min - level_max) * 100) / 100.0; // altura útil
    doc["sample_time_s"] = SAMPLE_INTERVAL;
    doc["filter_threshold_pct"] = filter_threshold;
    
    // Configurações de conectividade (sem senhas por segurança)
    doc["wifi_ssid"] = WiFi.SSID(); // SSID atual conectado
    doc["wifi_status"] = WiFi.status() == WL_CONNECTED ? "connected" : "disconnected";
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["wifi_ip"] = WiFi.localIP().toString();
    
    doc["mqtt_server"] = MQTT_SERVER;
    doc["mqtt_port"] = PORT_MQTT;
    doc["mqtt_user"] = MQTT_USERNAME;
    doc["mqtt_status"] = client.connected() ? "connected" : "disconnected";
    
    doc["status"] = "settings_updated";
    doc["message"] = "Configurações atualizadas com sucesso";

    char json_string[1024]; // Aumentei o buffer
    serializeJson(doc, json_string);

    Serial.println("📤 Enviando confirmação de configurações via MQTT...");
    bool result = client.publish(topico, json_string);
    
    if (result) {
        Serial.println("✅ Confirmação de configurações enviada com sucesso!");
        Serial.println("📋 Configurações atuais:");        
        Serial.println("   📶 WIFI:");
        Serial.println("      • SSID:   " + WiFi.SSID());
        Serial.println("      • Status: " + String(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado"));
        Serial.println("      • IP:     " + WiFi.localIP().toString());
        Serial.println("   📡 MQTT:");
        Serial.println("      • Servidor: " + String(MQTT_SERVER));
        Serial.println("      • Porta:    " + String(PORT_MQTT));
        Serial.println("      • Usuário:  " + String(MQTT_USERNAME));
        Serial.println("      • Status:   " + String(client.connected() ? "Conectado" : "Desconectado"));
        Serial.println("      EMPRESA: ");
        Serial.println("      • Cliente:    " + String(CLIENTE));
        Serial.println("      • Local:      " + String(LOCAL));
        Serial.println("      • Linha:      " + String(LINHA));
        Serial.println("      • ID:         " + String(ID_EQUIPAMENTO));
        Serial.println("      • Fabricante: " + String(FABRICANTE_MAQUINA));
        Serial.println("      • Modelo:     " + String(MODELO_MAQUINA));
        Serial.println("      • Tipo:       " + String(TIPO_SENSOR));
        Serial.println("      DEVICE: ");
        Serial.println("      • Id_dispositivo:     " + String(DISPOSITIVO_ID));                
        Serial.println("      • Tipo Sensor:        " + String(TIPO_SENSOR));
        Serial.println("      • Fabricante Sensor:  " + String(FABRICANTE_SENSOR));
        Serial.println("      • Placa SoC:          " + String(PLACA_SOC));
        Serial.println("      • Modelo Sensor:      " + String(MODELO_SENSOR));        
        Serial.println("      • Versão Hardware:    " + String(VERSAO_HARDWARE));        
        Serial.println("      • Data Instalação:    " + String(DATA_INSTALACAO));        
        Serial.println("      • Obs. (Device Info): " + String(OBSERVACAO_DEVICE_INFO));
        Serial.println("      • Obs. (Settings):    " + String(OBSERVACAO_SETTINGS));
        Serial.println("      • Obs. (Readings):    " + String(OBSERVACAO_READINGS));
        Serial.println("   🏠 RESERVATÓRIO:");
        Serial.println("      • Level Max: " + String(level_max) + " cm");
        Serial.println("      • Level Min: " + String(level_min) + " cm");        
        Serial.println("      • Intervalo: " + String(SAMPLE_INTERVAL) + " segundos");        
        

    } else {
        Serial.println("❌ Falha ao enviar confirmação de configurações!");
    }
    
    return result;
}

/**************************************************************
 * FIM DO ARQUIVO wifi_mqtt.cpp
 */