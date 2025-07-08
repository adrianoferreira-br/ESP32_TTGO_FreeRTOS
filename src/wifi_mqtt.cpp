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
#include "esp_ota_ops.h"
//Ota
#include <ArduinoOTA.h>


WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = SSID;                //"STARLINK";//"PhoneAdr"; // Substitua pelo seu SSID 
const char* password = PASSWORD;        //"11121314";//"UDJ1-ddsp";// "SUA_SENHA"; // Substitua pela sua senha 
const char* mqtt_server = MQTT_SERVER;  //"192.168.100.4";//"broker.hivemq.com";
const int port_mqtt = PORT_MQTT;        //1883

//String ip;

//WebServer server(80);

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
        delay(5000); 
        Serial.print("."); 
        i++;
   } while (((WiFi.status() != WL_CONNECTED) && (i<20)));

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
 * MOSTRA INFO DAS PARTIÇÕES 
 */
void show_partitions() {

    Serial.println("Lista de partições:");

    // Percorre todas as partições e imprime detalhes
    esp_partition_iterator_t it = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
    while (it != NULL) {
        const esp_partition_t* part = esp_partition_get(it);
        Serial.printf("Nome: %s | Tipo: %d | Subtipo: %d | Tamanho: %d bytes | Endereço: 0x%06X\n",
                      part->label, part->type, part->subtype, part->size, part->address);
        it = esp_partition_next(it);
    }
    esp_partition_iterator_release(it);

    
    // Verificar a partição ativa
    const esp_partition_t *running = esp_ota_get_running_partition();
    Serial.print("Running partition type: ");
    Serial.println(running->type);
    Serial.print("Running partition subtype: ");
    Serial.println(running->subtype);
    Serial.print("Running partition address: ");
    Serial.println(running->address, HEX);

    Serial.printf("Flash chip size: %u bytes\n", spi_flash_get_chip_size());

    // Inicializar a partição OTA apenas se necessário
    const esp_partition_t *next_update_partition = esp_ota_get_next_update_partition(NULL);
    if (next_update_partition != NULL) {
        esp_err_t err = esp_ota_set_boot_partition(next_update_partition);
        if (err != ESP_OK) {
          Serial.print("Failed to set boot partition: ");
          Serial.println(esp_err_to_name(err));
        } else {
          Serial.println("Boot partition set successfully");
        }
    } else {
        Serial.println("No OTA update partition found");
    }

    // Verificar novamente a partição ativa após a inicialização
    running = esp_ota_get_running_partition();
    Serial.print("Running partition type: ");
    Serial.println(running->type);
    Serial.print("Running partition subtype: ");
    Serial.println(running->subtype);
    Serial.print("Running partition address: ");
    Serial.println(running->address, HEX);

  }


/**************************************************************
 * INICIALIZAÇÃO DO OTA 
 */
void config_ota() {
   
    ArduinoOTA.setPort(3232);
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
          type = "sketch";
      } else { // U_SPIFFS
          type = "filesystem";
      }
      Serial.println("Start updating " + type);
    });

    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
          Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
          Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
          Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
          Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
          Serial.println("End Failed");
        }
    });

    ArduinoOTA.begin();
    Serial.println("OTA ready");

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
  
  //server.handleClient();
 

}


/*
 *
 */
void callback(char* topic, byte* payload, unsigned int length) 
{
   Serial.print("Mensagem recebida no tópico: "); 
   Serial.print(topic); 
   Serial.print(". Mensagem: "); 
   String message; 
   for (unsigned int i = 0; i < length; i++) 
   { 
    message += (char)payload[i]; 
   } 
   Serial.println(message); 
}



void reconnect() 
{ 
  int qnt = 3;
  while (!client.connected() && qnt > 1) 
  { 
    qnt--;
    Serial.print("Tentando conectar ao MQTT..."); 
    if (client.connect("ESP32Client")) 
    { 
      Serial.println("Conectado"); 
      client.subscribe("AdrPresto",1); // Inscreve-se no tópico "AdrPresto" com QoS 1
    } 
    else 
    { 
      Serial.print("falhou, rc="); 
      Serial.print(client.state()); 
      Serial.println(" tentando novamente em 3 segundos"); 
      delay(3000); 
    } 
  } 
}

/**********************************************************************************************
 *     ENVIA AS INFORMAÇÕES PARA O PROTOCOLO MQTT
 */
void mqtt_send_data(String nome_equipamento, String horario, int id_leitura, String observacao) {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Crie um objeto JSON
  StaticJsonDocument<256> doc;
  doc["equipamento"] = nome_equipamento;
  doc["hora"] = horario;
  doc["id_leitura"] = String(id_leitura);
  doc["observacao"] = observacao;


  // Serialize o objeto JSON para uma string
  char jsonBuffer[256];
  serializeJson(doc, jsonBuffer);

  
  // O método publish(topic, payload, retained, qos) está disponível nas versões recentes do PubSubClient
  bool result = client.publish("AdrPresto", (const uint8_t*)jsonBuffer, strlen(jsonBuffer)); // retained = false, qos = 1
  if (!result) {    
    Serial.println("Falha ao enviar mensagem JSON com QoS 1");
  }
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