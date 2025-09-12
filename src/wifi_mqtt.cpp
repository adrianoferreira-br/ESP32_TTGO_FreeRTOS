/*  File: wifi_mqtt.cpp
 *  Description:  WIFI and MQTT
 *  date: 2025-01-14
 */


#include "wifi_mqtt.h"
#include "constants.h"
#include "main.h"
#include <ArduinoJson.h>
//partions
#include "esp_partition.h"
// WebServer
#include <WebServer.h>
// Memória NVS  (Non-Volatile Storage)
#include <Preferences.h>
//OTA
#include <ArduinoOTA.h>



WebServer server(80); // Porta 80 padrão HTTP


WiFiClient espClient;
PubSubClient client(espClient);
Preferences prefs; // Cria o objeto Preferences

const char* ssid = SSID;                //"STARLINK";//"PhoneAdr"; // Substitua pelo seu SSID 
const char* password = PASSWORD;        //"11121314";//"UDJ1-ddsp";// "SUA_SENHA"; // Substitua pela sua senha 
const char* mqtt_server = MQTT_SERVER;  //"192.168.100.4";//"broker.hivemq.com";
const int port_mqtt = PORT_MQTT;        //1883

long idBatida = 0; // ID da batida

//String ip;


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
   } else {
    Serial.println(""); 
    tft.fillScreen(TFT_BLACK);    
    Serial.println("WiFi conectado"); 
    Serial.print("Endereço IP: "); 
    Serial.println(WiFi.localIP());                
   }       

   setup_webserver();




  }


  

/**************************************************************
 * LOOP DO WIFI 
 */
void loop_wifi(){
  // Preenche informações referente a rede
  if (WiFi.status() == WL_CONNECTED) {       
      show_ip();              
  } else {
      tft.setTextColor(TFT_RED, TFT_BLACK);    
      tft.drawString("Disconnected     ", 0, 0, 2);  
      tft.drawString("                 ", 130, 0, 2);  
      
  }   
}


/**************************************************************
 * WEB SERVER
 */

void handleRoot() {

  String html = "<html><head><meta http-equiv='refresh' content='1'></head><body>";

  html += "<h1>Presto Alimentos - Monitoramento de Maquina</h1>";
  html += "<h2>Equipamento: ";
  html += NOME_EQUIPAMENTO;
  html += "</h2>";
  html += "<h2>Batida nr:   " + String(idBatida) + "</h2>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}


void setup_webserver()  
{
  server.on("/", handleRoot);
  server.begin();
}




/**************************************************************
 * MOSTRA INFO DAS PARTIÇÕES 
 */
void show_partitions() {
  Serial.println("==== Partições encontradas ====");
  const esp_partition_t* part = NULL;
  esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
  while (it != NULL) {
    part = esp_partition_get(it);
    Serial.printf("APP: %s, Offset: 0x%06x, Size: 0x%06x\n", part->label, part->address, part->size);
    it = esp_partition_next(it);
  }
  esp_partition_iterator_release(it);

  it = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
  while (it != NULL) {
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
  ArduinoOTA.setHostname("meu_esp32"); // Nome que aparecerá na IDE Arduino
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
    if (error == OTA_AUTH_ERROR) Serial.println("Falha de autenticação");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Falha ao iniciar");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Falha de conexão");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Falha ao receber");
    else if (error == OTA_END_ERROR) Serial.println("Falha ao finalizar");
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
   client.setServer(mqtt_server, port_mqtt);
   client.setCallback(callback); 
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
  //client.publish("AdrPresto", "Hello MQTT from ESP32");
  //delay(2000);   
  
 

}


/*
 *
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
    // Aqui você pode adicionar o código para processar a mensagem recebida  
    prefs.begin("my-app", false); // Abre o namespace "my-app" em modo de leitura e escrita
    prefs.putString("last_message", message); // Armazena a mensagem com a chave "last_message"
    prefs.end(); // Fecha o namespace "my-app"
  }

  if (String(topic) == "info_ip") {    
    Serial.println("Lendo o IP salvo na NVS conforme comando recebido...");
    prefs.begin("my-app", true); // Abre o namespace "my-app" em modo somente leitura
    String lastMessage = prefs.getString("last_message", "Nenhum IP salvo"); // Lê a mensagem armazenada com a chave "last_message"
    Serial.println("Última mensagem salva: " + lastMessage);
    prefs.end(); // Fecha o namespace "my-app"
  } 

}



void reconnect() 
{ 
  int qnt = 3;
  while (!client.connected() && qnt > 1) 
  { 
    qnt--;
    Serial.print("Tentando conectar ao MQTT..."); 
    if (client.connect(NOME_EQUIPAMENTO)) 
    { 
      Serial.println("Conectado"); 
      //client.subscribe("AdrPresto",1); // Inscreve-se no tópico "AdrPresto" com QoS 1      
      client.subscribe("Reboot_",1);
      client.subscribe("config_ip",1);
      client.subscribe("info_ip",1);
      client.subscribe("config_mqtt",1);
      client.subscribe("info_mqtt",1);

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
 */
bool mqtt_send_data(const char* nome_equipamento, const char* horario, long id_leitura, const char* observacao) {
    if (!client.connected()) {
        return false;
    }
    client.loop();

    StaticJsonDocument<256> doc;
    doc["equipamento"] = nome_equipamento;
    doc["hora"] = horario;
    doc["id_leitura"] = String(id_leitura);
    doc["observacao"] = observacao;

    char jsonBuffer[256] = {0};
    size_t jsonLen = serializeJson(doc, jsonBuffer);
    bool result = client.publish("AdrPresto", (const uint8_t*)jsonBuffer, jsonLen, false); // QoS 0

    // web server update
    idBatida = id_leitura; // Atualiza o ID da batida
    handleRoot(); // Atualiza a página web após enviar os dados
    server.handleClient(); // Processa requisições do servidor web

    return result;
}

/**********************************************************************************************
 *     MOSTRA O IP DA REDE NO DISPLAY
 */
void show_ip () {
  
  // Mostra o IP
  char ipStr[16];  
  int NivelSinal = 0;
  IPAddress ip = WiFi.localIP();


  sprintf(ipStr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);    
  tft.drawString(ipStr, 0, 0, 2); 

  // Mostra o nível do sinal
   NivelSinal = WiFi.RSSI();
      //Serial.print("Nível Sinal:" + (String)NivelSinal + "dBm");
      if (NivelSinal >= -50) {
        tft.setTextColor(TFT_GREEN, TFT_BLACK);    
        tft.drawString("Otimo   ", 150,0 , 2);
      } else if (NivelSinal >= -70) {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);    
        tft.drawString("Bom    ", 150, 0, 2);
      } else if (NivelSinal >= -80) {
        tft.setTextColor(TFT_ORANGE, TFT_BLACK);    
        tft.drawString("Ruim   ", 150, 0, 2);
      } else {
        tft.setTextColor(TFT_RED, TFT_BLACK);    
        tft.drawString("Pessimo", 150, 0, 2);
      }
      tft.drawString((String)NivelSinal, 205, 1, 4);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);    
}






/*
{
"equipamento":"teste",
"hora":"2025-07-17 21:11:17",
"id_leitura":"3",
"observacao":""
}
*/