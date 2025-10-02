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
// WebServer
#include <WebServer.h>
#include "web_server.h"
//OTA
#include <ArduinoOTA.h>
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

   delay(10); 
   Serial.begin(115200); 
   Serial.println(); 
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
     tft.fillScreen(TFT_BLACK);    
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
  Serial.println("==== Partições encontradas ====");
  const esp_partition_t* part = NULL;
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
  while (it != NULL) 
  {
    part = esp_partition_get(it);
    Serial.printf("APP: %s, Offset: 0x%06x, Size: 0x%06x\n", part->label, part->address, part->size);
    it = esp_partition_next(it);
  }
  esp_partition_iterator_release(it);

  it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
  while (it != NULL) 
  {
    part = esp_partition_get(it);
    Serial.printf("DATA: %s, Offset: 0x%06x, Size: 0x%06x\n", part->label, part->address, part->size);
    it = esp_partition_next(it);
  }
  esp_partition_iterator_release(it);
  Serial.println("==============================="); 
}



/**************************************************************
 * INICIALIZAÇÃO DO OTA 
 */
void setup_ota(void){

  // Inicialize o OTA
  ArduinoOTA.setHostname(DISPOSITIVO_ID); // Nome que aparecerá na IDE Arduino
  ArduinoOTA.onStart([]() {
    Serial.println("Iniciando OTA...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA finalizada!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progresso: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Erro[%u]: ", error);
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
  Serial.println("OTA: Serviço OTA inicializado!");
  
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
   delay(2000);  
   char mqtt_server[32];
   //read_flash_string(KEY_MQTT_SERVER, mqtt_server, sizeof(mqtt_server));
   //int port_mqtt = read_flash_int(KEY_MQTT_PORT);   
   
   //client.setServer(mqtt_server, port_mqtt);
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

   
  if (String(topic) == "Reboot_") {
    Serial.println("Reiniciando o sistema conforme comando recebido...");
    delay(1000);
    ESP.restart(); // Reinicia o ESP32
  }


  if (String(topic) == "info") {
    Serial.println("Enviando informações do sistema conforme... Resposta topico INFO");
    bool result = mqtt_send_info();  
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
          // Salve na NVS se quiser persistência após reboot
        prefs.begin("mqtt", false);
        prefs.putString("server", MQTT_SERVER);
        prefs.putInt("port", PORT_MQTT);
        prefs.putString("username", MQTT_USERNAME);
        prefs.putString("password", MQTT_PASSWORD);
        prefs.end();
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
      client.subscribe("Reboot_",1);      
      client.subscribe("info",1);
      client.subscribe("config_mqtt",1);
      client.subscribe("config_ip",1);
      //client.subscribe("presto/floripa/forno/001")      
      client.subscribe(topico,1); // Inscreve-se no tópico geral do equipamento
      client.subscribe(CLIENTE,1); // Inscreve-se no tópico do cliente específico      
     // client.subscribe(CLIENTE + "/" + LOCAL , 1); // Inscreve-se no tópico do local específico
     // client.subscribe(CLIENTE + "/" + LOCAL + "/" + TIPO_EQUIPAMENTO, 1); // Inscreve-se no tópico do tipo de equipamento específico
     // client.subscribe(CLIENTE + "/" + LOCAL + "/" + TIPO_EQUIPAMENTO + "/" + String(DISPOSITIVO_ID), 1); // Inscreve-se no tópico do dispositivo específico
      Serial.println("Inscrito nos tópicos com sucesso!");
      
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

    StaticJsonDocument<256> doc;
    doc["equipamento"] = nome_equipamento;
    doc["tipo"] = "data";
    doc["hora"] = horario;
    doc["id_leitura"] = String(id_leitura);
    doc["observacao"] = observacao;

    char jsonBuffer[256] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0

    // web server update
    idBatida = id_leitura; // Atualiza o ID da batida
    Serial.println("MQTT: Dados enviados.." + String(CLIENTE));    
    //handleRoot(); // Atualiza a página web após enviar os dados


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
    

    char jsonBuffer[512] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish(topico, (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0
    Serial.println("MQTT: device_readings enviado.. Topico:  " + String(topico));            
    return result;

/* Exemplo de JSON enviado: 
{
    "table" = "device_readings";
    "device_id" = "presto-plh-l01-rsv-001";
    "timestamp" = 1759253363;
    "wifi_rssi_dbm" = -67;
    "zigbee_rssi_dbm" = 0;
    "wifi_rssi_%" = 33;
    "zigbee_rssi_%" = 0;
    "takt_time_id" = 34564;
    "temp_c" =  23.45;
    "temp_f" = 74.21;
    "humidity_%" = 65.50;
    "level_h_cm" = 150.25;
    "level_usage_%" = 75.50;
    "level_usage_cm" = 0.0;
    "level_usage_l" = 0.0;
    "level_usage_m3" = 0.0;
    "pressure_psi" = 33.5;
    "pressure_kpa" = 231.0;
    "pressure_bar" = 2.31;
    "pressure_mca" = 23.5;
    "flow_l_min" = 0.0;
    "flow_m3_h" = 0.0;
    "voltage_v" = 0;
    "voltage_bat_v" = 0;
    "current_a" = 0;
    "power_w" = 0;
    "energy_wh" = 0;
    "energy_kwh" = 0;
    "frequency_hz" = 0;
    "power_factor" = 0;
    "rpm" = 0;
    "vibration_mm_s" = 0;
    "vibration_g" = 0;
    "status" = "OK";
    "error_code" = 0;
    "notes" = "Nenhuma observação"    
}  */ 

  }