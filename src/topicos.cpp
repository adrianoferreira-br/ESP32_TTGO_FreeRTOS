/************************************************************
 *  File: topicos.cpp
 *  Description: MQTT Topics Handler
 *  date: 2025-11-03
 ***********************************************************/

#include "topicos.h"
#include "mem_flash.h"
#include "state.h"
#include "extern_data.h"
#include <esp_partition.h>
#include <esp_ota_ops.h>

// Referência ao sensor ultrassônico (se necessário)
#ifdef SENSOR_WATER_LEVEL
extern void reset_percentual_filter();
#endif

// Referência ao sensor DS18B20 (se necessário)
#ifdef SENSOR_TEMPERATURA_DS18B20
extern float temperatura_ds18b20;
extern float temperatura_ds18b20_2;  // Segundo sensor
extern int num_sensors_ds18b20;
#endif


// ============ VARIÁVEIS GLOBAIS PARA CONTROLE DE ENVIO DE LEITURAS ============
bool enabled_send_level_readings = false;        // Habilita envio de leituras de nível
bool enabled_send_temperature_readings = false;  // Habilita envio de leituras de temperatura (DHT)
bool enabled_send_temp_DS18B20_readings = false; // Habilita envio de leituras de temperatura (DS18B20)
bool enabled_send_batch_readings = false;      // Habilita envio de leituras de ticket - Sensor 1
bool enabled_send_batch_readings_sensor2 = false; // Habilita envio de leituras de ticket - Sensor 2
bool enabled_send_humidity_readings = false;     // Habilita envio de leituras de umidade

long id_message_batch = 0;                     // ID da mensagem de batch
long id_message_batch2 = 0;                    // ID da mensagem de batch sensor 2


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

            if (doc.containsKey("sample_interval_batch")) {
                int sample_interval_batch_tmp = doc["sample_interval_batch"];
                save_flash_int(KEY_SAMPLE_INTERVAL_BATCH, sample_interval_batch_tmp);  // Usa função centralizada
                sample_interval_batch = sample_interval_batch_tmp; // Atualiza a variável global imediatamente
                reconfigure_batch_timer(sample_interval_batch_tmp); // Reconfigura o timer dinamicamente
                Serial.println("✅ Salvo sample_interval_batch: " + String(sample_interval_batch_tmp) + " segundos");             
            }


            // =============================== CONFIGURAÇÕES DE DISPOSITIVO ===
            
            //CLIENTE
            if (doc.containsKey("client")) {
                String cliente_tmp = doc["client"];
                save_flash_string(KEY_CLIENTE, cliente_tmp.c_str());  // Salva na flash
                strcpy(CLIENTE, cliente_tmp.c_str()); // Atualiza variável global
                Serial.println("✅ Salvo CLIENTE: " + cliente_tmp);
            }
            // LOCAL
            if (doc.containsKey("location")) {
                String local_tmp = doc["location"];
                save_flash_string(KEY_LOCAL, local_tmp.c_str());  // Salva na flash
                strcpy(LOCAL, local_tmp.c_str()); // Atualiza variável global
                Serial.println("✅ Salvo LOCAL: " + local_tmp);
            }

            // LINE            
            if (doc.containsKey("line")) {
                String line_tmp = doc["line"];
                save_flash_string(KEY_LINHA, line_tmp.c_str());
                strcpy(LINHA, line_tmp.c_str());
                Serial.println("✅ Salvo linha: " + line_tmp);
            }

            // TIPO EQUIPAMENTO
            if (doc.containsKey("type_equip")) {
                String tipo_equip_tmp = doc["type_equip"];
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
            if (doc.containsKey("name_equip")) {
                String nome_equip_tmp = doc["name_equip"];
                save_flash_string(KEY_NOME_EQUIP, nome_equip_tmp.c_str());  // Usa função centralizada
                strcpy(NOME_EQUIPAMENTO, nome_equip_tmp.c_str()); // Atualiza variável global
                Serial.println("✅ Salvo name_equip: " + nome_equip_tmp);
            }

            // DISPOSITIVO_ID
            if (doc.containsKey("device_id")) {
                String dispositivo_id_tmp = doc["device_id"];
                save_flash_string(KEY_DISPOSITIVO_ID, dispositivo_id_tmp.c_str());
                strcpy(DISPOSITIVO_ID, dispositivo_id_tmp.c_str());
                Serial.println("✅ Salvo dispositivo_id: " + dispositivo_id_tmp);
            }

            // FABRICANTE MAQUINA
            if (doc.containsKey("manufacturer_machine")) {
                String fabricante_maquina_tmp = doc["manufacturer_machine"];
                save_flash_string(KEY_FABRICANTE_MAQUINA, fabricante_maquina_tmp.c_str());
                strcpy(FABRICANTE_MAQUINA, fabricante_maquina_tmp.c_str());
                Serial.println("✅ Salvo fabricante_maquina: " + fabricante_maquina_tmp);
            }

            // MODELO MAQUINA
            // MODELO MAQUINA
            if (doc.containsKey("model_machine")) {
                String modelo_maquina_tmp = doc["model_machine"];
                save_flash_string(KEY_MODELO_MAQUINA, modelo_maquina_tmp.c_str());
                strcpy(MODELO_MAQUINA, modelo_maquina_tmp.c_str());
                Serial.println("✅ Salvo model_machine: " + modelo_maquina_tmp);
            }

            // SERIAL MAQUINA
            if (doc.containsKey("serial_machine")) {
                String serial_maquina_tmp = doc["serial_machine"];
                save_flash_string(KEY_SERIAL_MAQUINA, serial_maquina_tmp.c_str());
                strcpy(SERIAL_MAQUINA, serial_maquina_tmp.c_str());
                Serial.println("✅ Salvo serial_machine: " + serial_maquina_tmp);
            }

            // TIPO SENSOR
            if (doc.containsKey("type_sensor")) {
                String type_sensor_tmp = doc["type_sensor"];
                save_flash_string(KEY_TIPO_SENSOR, type_sensor_tmp.c_str());
                strcpy(TIPO_SENSOR, type_sensor_tmp.c_str());
                Serial.println("✅ Salvo type_sensor: " + type_sensor_tmp);
            }

            // NOTES DEVICE INFO
            if (doc.containsKey("notes_device_info")) {
                String notes_device_info_tmp = doc["notes_device_info"];
                save_flash_string(KEY_DEVICE_INFO, notes_device_info_tmp.c_str());
                strcpy(OBSERVACAO_DEVICE_INFO, notes_device_info_tmp.c_str());
                Serial.println("✅ Salvo notes_device_info: " + notes_device_info_tmp);
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

            

            // PLACA_SOC
            if (doc.containsKey("placa_soc")) {
                String placa_soc_tmp = doc["placa_soc"];
                save_flash_string(KEY_PLACA_SOC, placa_soc_tmp.c_str());
                strcpy(PLACA_SOC, placa_soc_tmp.c_str());
                Serial.println("✅ Salvo placa_soc: " + placa_soc_tmp);
            }

            // FABRICANTE_SENSOR
            if (doc.containsKey("sensor_manufacturer")) {
                String fabricante_sensor_tmp = doc["sensor_manufacturer"];
                save_flash_string(KEY_FABRICANTE_SENSOR, fabricante_sensor_tmp.c_str());
                strcpy(FABRICANTE_SENSOR, fabricante_sensor_tmp.c_str());
                Serial.println("✅ Salvo fabricante_sensor: " + fabricante_sensor_tmp);
            }

            // MODELO_SENSOR
            if (doc.containsKey("sensor_model")) {
                String modelo_sensor_tmp = doc["sensor_model"];
                save_flash_string(KEY_MODELO_SENSOR, modelo_sensor_tmp.c_str());
                strcpy(MODELO_SENSOR, modelo_sensor_tmp.c_str());
                Serial.println("✅ Salvo modelo_sensor: " + modelo_sensor_tmp);
            }

            // VERSAO_HARDWARE
            if (doc.containsKey("hardware_version")) {
                String versao_hardware_tmp = doc["hardware_version"];
                save_flash_string(KEY_VERSAO_HARDWARE, versao_hardware_tmp.c_str());
                strcpy(VERSAO_HARDWARE, versao_hardware_tmp.c_str());
                Serial.println("✅ Salvo versao_hardware: " + versao_hardware_tmp);
            }

            // DATA_INSTALACAO
            if (doc.containsKey("installation_date")) {
                String data_instalacao_tmp = doc["installation_date"];
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
 * FUNÇÃO AUXILIAR PARA DECODIFICAR ERROS MQTT
 */
String getMqttErrorMessage(int errorCode) {
  switch(errorCode) {
    case -4: return "MQTT_CONNECTION_TIMEOUT";
    case -3: return "MQTT_CONNECTION_LOST";
    case -2: return "MQTT_CONNECT_FAILED";
    case -1: return "MQTT_DISCONNECTED";
    case 0:  return "MQTT_CONNECTED";
    case 1:  return "MQTT_CONNECT_BAD_PROTOCOL";
    case 2:  return "MQTT_CONNECT_BAD_CLIENT_ID";
    case 3:  return "MQTT_CONNECT_UNAVAILABLE";
    case 4:  return "MQTT_CONNECT_BAD_CREDENTIALS";
    case 5:  return "MQTT_CONNECT_UNAUTHORIZED";
    default: return "CÓDIGO_DESCONHECIDO_" + String(errorCode);
  }
}

/**************************************************************
 *  RECONEXÃO DO MQTT COM SEUS RESPECTIVOS TÓPICOS
 */
void reconnect() 
{ 
  static unsigned long last_reconnect_attempt = 0;
  static int tentativas_restantes = 3;
  const unsigned long reconnect_interval = 5000; // 5 segundos entre tentativas
  
  // ✅ CONTROLE NÃO-BLOQUEANTE: Só tenta reconectar após intervalo
  unsigned long now = millis();
  if (now - last_reconnect_attempt < reconnect_interval) {
    return; // Sai sem bloquear o loop
  }
  
  // Atualiza timestamp da tentativa
  last_reconnect_attempt = now;
  
  if (!client.connected() && tentativas_restantes > 0) 
  { 
    tentativas_restantes--;
    Serial.println("=== TENTATIVA CONEXÃO MQTT (restam " + String(tentativas_restantes) + ") ===");
    Serial.println("Device ID: '" + String(DISPOSITIVO_ID) + "'");
    Serial.println("Username: '" + String(MQTT_USERNAME) + "' (len: " + String(strlen(MQTT_USERNAME)) + ")");
    Serial.println("Password: '" + String(MQTT_PASSWORD) + "' (len: " + String(strlen(MQTT_PASSWORD)) + ")");
    Serial.println("Servidor: " + String(MQTT_SERVER) + ":" + String(PORT_MQTT));
    
    bool connected = false;
    
    // Estratégia 1: Tentar com as credenciais configuradas
    if (strlen(MQTT_USERNAME) > 0 && strlen(MQTT_PASSWORD) > 0) {
      Serial.print("Tentativa 1: Conectando com credenciais...");
      connected = client.connect(DISPOSITIVO_ID, MQTT_USERNAME, MQTT_PASSWORD);
      
      if (connected) {
        Serial.println(" ✅ SUCESSO COM CREDENCIAIS!");
      } else {
        Serial.println(" ❌ FALHOU COM CREDENCIAIS!");
        int errorCode = client.state();
        Serial.println("Erro: " + String(errorCode) + " - " + getMqttErrorMessage(errorCode));
      }
    }
    
    // Estratégia 2: Se falhou com credenciais, tentar sem autenticação
    if (!connected) {
      Serial.print("Tentativa 2: Conectando sem autenticação...");
      connected = client.connect(DISPOSITIVO_ID);
      
      if (connected) {
        Serial.println(" ✅ SUCESSO SEM AUTENTICAÇÃO!");
        Serial.println("⚠️ ATENÇÃO: Conectado sem autenticação - broker permite acesso anônimo");
      } else {
        Serial.println(" ❌ FALHOU SEM AUTENTICAÇÃO!");
      }
    }
    
    if (connected) {
      // ✅ SUCESSO - RESETA CONTADOR       
      tentativas_restantes = 3;
      last_reconnect_attempt = 0;
      
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
      int errorCode = client.state();
      Serial.println("❌ FALHA NA CONEXÃO!");
      Serial.println("Último erro: " + String(errorCode) + " - " + getMqttErrorMessage(errorCode));
      
      // Mensagens específicas para problemas comuns
      if (errorCode == 5) {
        Serial.println("🔐 PROBLEMA DE AUTENTICAÇÃO:");
        Serial.println("   • Verifique se o usuário '" + String(MQTT_USERNAME) + "' existe no broker");
        Serial.println("   • Verifique se a senha está correta");
        Serial.println("   • Verifique se o broker requer autenticação");
      } else if (errorCode == 4) {
        Serial.println("🔑 CREDENCIAIS INCORRETAS:");
        Serial.println("   • Username: '" + String(MQTT_USERNAME) + "'");
        Serial.println("   • Password: '" + String(MQTT_PASSWORD) + "'");
      } else if (errorCode == 3) {
        Serial.println("🌐 PROBLEMA DE CONECTIVIDADE:");
        Serial.println("   • Servidor: " + String(MQTT_SERVER) + ":" + String(PORT_MQTT));
        Serial.println("   • Verifique se o broker está online");
      }
      
      if (tentativas_restantes > 0) {
        Serial.println("⏳ Próxima tentativa em " + String(reconnect_interval/1000) + " segundos..."); 
      } else {
        Serial.println("⏸️ Tentativas esgotadas. Aguardando " + String(reconnect_interval/1000) + " segundos antes de reiniciar...");
        tentativas_restantes = 3; // Reseta para próximo ciclo
      }
      // ✅ REMOVIDO delay(2000) - agora usa controle não-bloqueante com millis()
    } 
  } 
}

/**********************************************************************************************
*     ENVIA AS INFORMAÇÕES PARA O PROTOCOLO MQTT
*
* Exemplo de JSON enviado:
*{   "equipamento":"teste",
*    "hora":"2025-07-17 21:11:17",
*    "id_message_batch":"3",
*    "observacao":""  
* }
*/
bool mqtt_send_data(const char* nome_equipamento, const char* horario, long id_message_batch, const char* observacao) {
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
    doc["takt_time_id"] = id_message_batch;
    doc["note"] = observacao;

    char jsonBuffer[256] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0

    // web server update
    idBatida = id_message_batch; // Atualiza o ID da batida
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

bool mqtt_send_settings(){//const char* nome_equipamento, const char* horario, long id_message_batch, const char* observacao) {
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
    doc["sample_interval_batch"] = sample_interval_batch;
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
*    "id_message_batch":"3",
*    "observacao":""  
* }
*/

bool mqtt_send_info(){//const char* nome_equipamento, const char* horario, long id_message_batch, const char* observacao) {
    if (!client.connected()) {
       Serial.println("❌ mqtt_send_info: Cliente MQTT desconectado!");
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
    doc["machine_serial"] = SERIAL_MAQUINA;
    doc["sensor_type"] = TIPO_SENSOR;
    doc["sensor_manufacturer"] = FABRICANTE_SENSOR;
    doc["sensor_model"] = MODELO_SENSOR;
    doc["ip_address"] = WiFi.localIP().toString(); 
    doc["mac_address"] = WiFi.macAddress();
    doc["firmware_version"] = VERSION;
    doc["hardware_version"] = VERSAO_HARDWARE;
    doc["installation_date"] = DATA_INSTALACAO;        
    doc["notes"] = OBSERVACAO_DEVICE_INFO;
    

    char jsonBuffer[1024] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    
    // Verifica se JSON foi serializado corretamente
    if (jsonLen == 0) {
        Serial.println("❌ mqtt_send_info: Falha ao serializar JSON!");
        return false;
    }
    
    // Verifica se JSON cabe no buffer
    if (jsonLen >= sizeof(jsonBuffer)) {
        Serial.printf("❌ mqtt_send_info: JSON truncado! Tamanho: %d bytes (buffer: %d bytes)\n", jsonLen, sizeof(jsonBuffer));
    }
    
    Serial.printf("mqtt_send_info: Enviando %d bytes para tópico '%s'\n", jsonLen, topico);
    Serial.println("JSON: " + String(jsonBuffer));
    
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0
    
    if (result) {
        Serial.println("mqtt_send_info: Publicação bem-sucedida!");
    } else {
        Serial.println("❌ mqtt_send_info: Falha na publicação MQTT!");
        Serial.printf("   Estado MQTT: %d (%s)\n", client.state(), getMqttErrorMessage(client.state()).c_str());
    }
    
    return result;

/* avaliar essas informações: 
{    
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

    StaticJsonDocument<1024> doc;

    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = timestamp2;
    doc["type_sensor"] = TIPO_SENSOR;
    doc["manufacturer_sensor"] = FABRICANTE_SENSOR;
    doc["sensor_model"] = MODELO_SENSOR;
    doc["board_soc"] = PLACA_SOC;
    doc["temperature_cpu_c"] = roundf(temperatureRead() * 100) / 100.0;
    doc["hardware_version"] = VERSAO_HARDWARE;
    doc["firmware_version"] = VERSION;
    doc["installation_date"] = DATA_INSTALACAO;


    char jsonBuffer[1024] = {0};
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
    doc["level_max"] = level_max;
    doc["level_min"] = level_min;
    doc["level_effective_cm"] = roundf((level_min - level_max) * 100) / 100.0; // altura útil
    doc["sample_time_s"] = SAMPLE_INTERVAL;
    doc["sample_interval_batch"] = sample_interval_batch;
    doc["filter_threshold_pct"] = filter_threshold;
    
    // Configurações de conectividade (sem senhas por segurança)
    doc["wifi_ssid"] = WiFi.SSID(); // SSID atual conectado    
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
        Serial.println("== Configurações atuais: ==");        
        Serial.println("    WIFI:");
        Serial.println("      • SSID:   " + WiFi.SSID());
        Serial.println("      • Status: " + String(WiFi.status() == WL_CONNECTED ? "Conectado" : "Desconectado"));
        Serial.println("      • IP:     " + WiFi.localIP().toString());
        Serial.println("    MQTT:");
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
        Serial.println("    RESERVATÓRIO:");
        Serial.println("      • Level Max: " + String(level_max) + " cm");
        Serial.println("      • Level Min: " + String(level_min) + " cm");        
        Serial.println("      • Intervalo: " + String(SAMPLE_INTERVAL) + " segundos");        
        

    } else {
        Serial.println("❌ Falha ao enviar confirmação de configurações!");
    }
    
    return result;
}


/*
*  envia as leituras habilitadas
*/
bool mqtt_send_datas_readings() {
    if (!client.connected()) {
        return false;
    }
    client.loop();
    char time_str_buffer[16];           char* timestamp = get_time_str(time_str_buffer, sizeof(time_str_buffer));    
    long timestamp2 = atol(time_str_buffer); // Converte string para long

    StaticJsonDocument<1024> doc;    

    doc["table"] = "device_readings";
    doc["device_id"] = DISPOSITIVO_ID;
    doc["timestamp"] = timestamp2;
    doc["wifi_rssi_dbm"] = WiFi.RSSI();

    // Cria um array para as leituras
    JsonArray readings = doc.createNestedArray("readings");
    
    // se habilitado = reservatório incluir na lista leitura do reservatório
    if (enabled_send_level_readings) {        
        JsonObject reading = readings.createNestedObject();        
        reading["metric_name"] = "level";
        reading["value"] = roundf(percentual_reservatorio * 100) / 100.0;
        reading["unit"] = "percent";
        reading["message_code"] = 0;        
        enabled_send_level_readings = false;
    }

    // se habilitado = temperatura incluir na lista leitura da temperatura
    if (enabled_send_temperature_readings) {        
        JsonObject reading = readings.createNestedObject();        
        reading["metric_name"] = "temperature";
        reading["value"] = roundf(temperatura * 100) / 100.0;        
        reading["unit"] = "celsius";
        reading["message_code"] = 0;
        enabled_send_temperature_readings = false;
    }
    
    // se habilitado = humidade incluir na lista leitura de humidade
    if (enabled_send_humidity_readings) {        
        JsonObject reading = readings.createNestedObject();        
        reading["metric_name"] = "humidity";
        reading["value"] = roundf(humidade * 100) / 100.0;        
        reading["unit"] = "percent";
        reading["message_code"] = 0;    
        enabled_send_humidity_readings = false;
    }

    #ifdef SENSOR_TEMPERATURA_DS18B20
    // se habilitado = temperatura DS18B20 incluir na lista leitura da temperatura
    if (enabled_send_temp_DS18B20_readings) {
        // Sensor 0 (primeiro sensor)
        JsonObject reading = readings.createNestedObject();        
        reading["metric_name"] = "temperature";
        reading["value"] = roundf(temperatura_ds18b20 * 100) / 100.0;        
        reading["unit"] = "celsius";
        reading["interval"] = SAMPLE_INTERVAL;  // Intervalo correto: Timer 0 usa SAMPLE_INTERVAL
        reading["message_code"] = 0;
        
        // Sensor 1 (segundo sensor, se existir)
        if (num_sensors_ds18b20 >= 2) {
            JsonObject reading2 = readings.createNestedObject();
            reading2["metric_name"] = "temperature2";  // Diferencia do primeiro
            reading2["value"] = roundf(temperatura_ds18b20_2 * 100) / 100.0;
            reading2["unit"] = "celsius";
            reading2["interval"] = SAMPLE_INTERVAL;
            reading2["message_code"] = 0;
        }
        
        enabled_send_temp_DS18B20_readings = false;
    }
    #endif

    // se habilitado = ticket incluir na lista leitura do ticket - SENSOR 1
    if (enabled_send_batch_readings) {        
        JsonObject reading = readings.createNestedObject();
        reading["metric_name"] = "batch_time";
        reading["value"] = qtd_batidas_intervalo;
        reading["interval"] = sample_interval_batch;
        reading["message_id"] = id_message_batch;
        if (message_error_code != 0) {
            reading["message_code"] = message_error_code;
        }
        enabled_send_batch_readings = false;
    }
    
    // se habilitado = ticket incluir na lista leitura do ticket - SENSOR 2
    if (enabled_send_batch_readings_sensor2) {        
        JsonObject reading = readings.createNestedObject();
        reading["metric_name"] = "batch_time_sensor2";
        reading["value"] = qtd_batidas_intervalo_sensor2;
        reading["interval"] = sample_interval_batch; //todo: diferenciar intervalo sensor 2
        reading["message_id"] = id_message_batch2; 
        if (message_error_code != 0) {
            reading["message_code"] = message_error_code;
        }
        enabled_send_batch_readings_sensor2 = false;
    }
    

    char jsonBuffer[1024] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0
    Serial.println("MQTT: device_readings enviado.. Topico:  " + String(topico));
    Serial.println("JSON enviado: " + String(jsonBuffer));            
    return result;
}




/**************************************************************
 * FIM DO ARQUIVO wifi_mqtt.cpp
 */







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