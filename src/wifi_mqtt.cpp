/************************************************************
 *  File: wifi_mqtt.cpp
 *  Description:  WIFI and MQTT Setup and Loop
 *  date: 2025-01-14
 ***********************************************************/


#define MQTT_MAX_PACKET_SIZE 1024  // CRÍTICO: Definir ANTES de incluir PubSubClient

#include "wifi_mqtt.h"
#include "topicos.h"
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

   // Mostra mensagem no display indicando que está procurando rede WiFi
   tft.fillScreen(TFT_BLACK);
   tft.setTextColor(TFT_YELLOW, TFT_BLACK);
   tft.drawString("Procurando WiFi...", 10, 30, 4);
   tft.setTextColor(TFT_WHITE, TFT_BLACK);
   tft.drawString("SSID: " + String(ssid), 10, 60, 2);
   tft.drawString("Aguarde...", 10, 90, 2);

   WiFi.begin(ssid, password); 
   do  
   { 
      delay(1000); 
      Serial.print("."); 
      
      // Atualiza indicador visual no display a cada tentativa
      int dots = i % 4;
      String indicator = "";
      for(int d = 0; d < dots; d++) indicator += ".";
      tft.fillRect(10, 110, 310, 60, TFT_BLACK); // Limpa área
      tft.setTextColor(TFT_CYAN, TFT_BLACK);
      tft.drawString("Tentando" + indicator + "     ", 10, 110, 2);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("Tentativa: " + String(i+1) + "/360", 10, 140, 2);
      
      i++;
   } while (((WiFi.status() != WL_CONNECTED) && (i<360)));

   if (WiFi.status() != WL_CONNECTED)
   {
      Serial.println("Falha ao conectar na rede");
      
      // Mostra mensagem de falha no display
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawString("WiFi FALHOU!", 10, 30, 4);
      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.drawString("SSID: " + String(ssid), 10, 60, 2);
      tft.drawString("Rede nao encontrada", 10, 90, 2);
      tft.setTextColor(TFT_YELLOW, TFT_BLACK);
      tft.drawString("Verifique nome/senha", 10, 110, 2);
      
      delay(5000); // Mantém mensagem por 5 segundos
      
      return;
   } 
   else 
   {
     Serial.println(""); 
     
     // Mostra sucesso no display
     tft.fillScreen(TFT_BLACK);
     tft.setTextColor(TFT_GREEN, TFT_BLACK);
     tft.drawString("WiFi OK!", 10, 30, 4);
     tft.setTextColor(TFT_WHITE, TFT_BLACK);
     tft.drawString("SSID: " + String(ssid), 10, 60, 2);
     
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

  Serial.println("===== INICIANDO CONFIGURAÇÃO OTA =====");
  
  // Inicializar mDNS primeiro
  if (!MDNS.begin(DISPOSITIVO_ID)) {
    Serial.println("❌ OTA: Erro ao inicializar mDNS!");
    Serial.println("   Tentando novamente...");
    delay(1000);
    if (!MDNS.begin(DISPOSITIVO_ID)) {
      Serial.println("❌ OTA: Falha crítica no mDNS - OTA pode não funcionar!");
    }
  } else {
    Serial.println("✅ OTA: mDNS inicializado com sucesso!");
    Serial.print("📡 OTA: mDNS hostname: ");
    Serial.print(DISPOSITIVO_ID);
    Serial.println(".local");
  }

  // ArduinoOTA setup
  ArduinoOTA.setPort(3232); 
  ArduinoOTA.setHostname(DISPOSITIVO_ID);
  
  // ✅ ADICIONA SENHA PARA SEGURANÇA (opcional mas recomendado)
  // ArduinoOTA.setPassword("admin"); // Descomente e defina uma senha se necessário
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("\n🚀 Iniciando atualização OTA do " + type);
    Serial.println("⚠️  NÃO DESLIGUE O DISPOSITIVO!");
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\n✅ ArduinoOTA finalizada com sucesso!");
    Serial.println("🔄 Reiniciando...");
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    static unsigned int lastPercent = 0;
    unsigned int percent = (progress / (total / 100));
    if (percent != lastPercent && percent % 10 == 0) {
      Serial.printf("📊 Progresso OTA: %u%%\n", percent);
      lastPercent = percent;
    }
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("\n❌ Erro ArduinoOTA[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Falha de autenticação");
      Serial.println("💡 Dica: Verifique se a senha OTA está correta");
    }
    else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Falha ao iniciar");
      Serial.println("💡 Dica: Pode ser falta de espaço ou partição incorreta");
    }
    else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Falha de conexão");
      Serial.println("💡 Dica: Verifique firewall e conectividade de rede");
    }
    else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Falha ao receber dados");
      Serial.println("💡 Dica: Conexão WiFi instável ou interferência");
    }
    else if (error == OTA_END_ERROR) {
      Serial.println("Falha ao finalizar");
      Serial.println("💡 Dica: Firmware corrompido ou incompatível");
    }
  });
  
  ArduinoOTA.begin();
  
  Serial.println("✅ OTA: ArduinoOTA inicializado!");
  Serial.println("📋 Informações de conexão OTA:");
  Serial.print("   • IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("   • Hostname: ");
  Serial.println(DISPOSITIVO_ID);
  Serial.print("   • Porta: 3232");
  Serial.println();
  Serial.println("   • mDNS: " + String(DISPOSITIVO_ID) + ".local");
  Serial.println("💡 Para testar: ping " + String(DISPOSITIVO_ID) + ".local");
  Serial.println("============================================");
  
}


/**************************************************************
 * LOOP DO OTA
 */
void loop_ota() {  
  ArduinoOTA.handle();
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
      
   client.setBufferSize(1024); // Configura buffer MQTT para suportar JSONs grandes (até 1024 bytes)
   
   client.setServer(MQTT_SERVER, PORT_MQTT);   
   client.setCallback(callback); 
   
   // Debug: Mostra valores atuais das credenciais MQTT
   Serial.println("=== DEBUG MQTT SETUP ===");
   Serial.println("MQTT_USERNAME: '" + String(MQTT_USERNAME) + "' (len: " + String(strlen(MQTT_USERNAME)) + ")");
   Serial.println("MQTT_PASSWORD: '" + String(MQTT_PASSWORD) + "' (len: " + String(strlen(MQTT_PASSWORD)) + ")");
   Serial.println("MQTT_SERVER: '" + String(MQTT_SERVER) + "'");
   Serial.println("PORT_MQTT: " + String(PORT_MQTT));
   Serial.println("========================");
   
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

