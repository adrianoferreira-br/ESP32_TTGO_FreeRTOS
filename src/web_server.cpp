#include <WiFi.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Update.h>
#include <ArduinoOTA.h>
#include <esp_ota_ops.h>
#include <esp_image_format.h>
#include <esp_flash_partitions.h>
#include <esp_partition.h>
#include <esp_task_wdt.h>
#include <MD5Builder.h>
#include "constants.h"
#include "mem_flash.h"
#include "wifi_mqtt.h"
#include "web_server.h"
#include "state.h"

// Variáveis globais para OTA ESP-IDF nativo
static esp_ota_handle_t ota_handle = 0;
static const esp_partition_t *ota_partition = NULL;
static size_t totalReceived = 0;
static size_t lastReported = 0;
static MD5Builder md5;
static String firmwareHash = "";
static bool ota_upload_success = false;
static bool ota_in_progress = false;  // Previne uploads concorrentes



extern WebServer server;
WebServer server(80); // Porta 80 padrão HTTP


/**************************************************************
 * SETUP WEB SERVER
 */


void setup_webserver()  
{
  server.on("/", handleRoot);
  server.on("/config_mqtt", handleConfigMQTT); // <-- nova rota
  server.on("/readings", handleReadings);
  server.on("/info", handleInfo);
  server.on("/ota", HTTP_GET, handleOTA);
  server.on("/ota", HTTP_POST, handleOTA, handleOTA);  // POST com handler de upload
  server.begin();
}


/**************************************************************
 * LOOP WEB SERVER
 */

void loop_webserver() {
  server.handleClient();
}



void handleRoot() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='30'>";
  html += "<title>Presto Alimentos - " + String(NOME_EQUIPAMENTO) + "</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += "h1 { color: #2c5530; text-align: center; margin-bottom: 30px; }";
  html += "h2 { color: #333; margin: 15px 0; }";
  html += ".menu-item { display: block; background: #007cba; color: white; text-decoration: none; padding: 15px 20px; margin: 10px 0; border-radius: 8px; text-align: center; font-weight: bold; transition: background 0.3s ease; }";
  html += ".menu-item:hover { background: #005a87; }";
  html += ".menu-item.ota { background: #28a745; }";
  html += ".menu-item.ota:hover { background: #1e7e34; }";
  html += ".info { background: #e8f4fd; padding: 15px; border-radius: 5px; margin: 20px 0; text-align: center; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>🏭 Presto Alimentos</h1>";
  html += "<div class='info'>";
  html += "<p><strong>Equipamento:</strong> " + String(NOME_EQUIPAMENTO) + "</p>";
  html += "<p><strong>Device ID:</strong> " + String(DISPOSITIVO_ID) + "</p>";
  html += "<p><strong>IP:</strong> " + WiFi.localIP().toString() + "</p>";
  html += "</div>";
  html += "<a href='/info' class='menu-item'>📊 Informações do Sistema</a>";
  html += "<a href='/readings' class='menu-item'>📈 Monitoramento</a>";
  html += "<a href='/config_mqtt' class='menu-item'>⚙️ Configurar MQTT</a>";
  html += "<a href='/ota' class='menu-item ota'>🚀 Atualização OTA</a>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}



/**************************************************************
 *   IP/
 */

void handleInfo() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='5'>";
  html += "<title>Informações do Sistema - Presto Alimentos</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += "h1 { color: #2c5530; text-align: center; margin-bottom: 30px; }";
  html += "h2 { color: #333; margin: 15px 0; }";
  html += ".info-section { background: #e8f4fd; padding: 20px; border-radius: 8px; margin: 20px 0; }";
  html += ".info-item { display: flex; justify-content: space-between; padding: 8px 0; border-bottom: 1px solid #ddd; }";
  html += ".info-item:last-child { border-bottom: none; }";
  html += ".label { font-weight: bold; color: #555; }";
  html += ".value { color: #007cba; font-weight: bold; }";
  html += ".back-link { text-align: center; margin: 20px 0; }";
  html += ".back-link a { color: #007cba; text-decoration: none; font-weight: bold; padding: 10px 20px; background: #f8f9fa; border-radius: 5px; }";
  html += ".back-link a:hover { background: #e9ecef; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>📊 Informações do Sistema</h1>";
  html += "<div class='info-section'>";
  html += "<div class='info-item'><span class='label'>Equipamento:</span><span class='value'>" + String(NOME_EQUIPAMENTO) + "</span></div>";
  html += "<div class='info-item'><span class='label'>Device ID:</span><span class='value'>" + String(DISPOSITIVO_ID) + "</span></div>";
  html += "<div class='info-item'><span class='label'>Endereço IP:</span><span class='value'>" + WiFi.localIP().toString() + "</span></div>";
  html += "<div class='info-item'><span class='label'>Versão Firmware:</span><span class='value'>" + String(VERSION) + "</span></div>";
  html += "<div class='info-item'><span class='label'>MQTT Server:</span><span class='value'>" + String(MQTT_SERVER) + "</span></div>";
  html += "<div class='info-item'><span class='label'>MQTT Port:</span><span class='value'>" + String(PORT_MQTT) + "</span></div>";
  html += "<div class='info-item'><span class='label'>MQTT User:</span><span class='value'>" + String(MQTT_USERNAME) + "</span></div>";
  html += "<div class='info-item'><span class='label'>MQTT Topic:</span><span class='value'>" + String(topico) + "</span></div>";
  html += "<div class='info-item'><span class='label'>Free Heap:</span><span class='value'>" + String(ESP.getFreeHeap()) + " bytes</span></div>";
  html += "<div class='info-item'><span class='label'>Flash Size:</span><span class='value'>" + String(ESP.getFlashChipSize()) + " bytes</span></div>";
  html += "</div>";
  html += "<div class='back-link'>";
  html += "<a href='/'>⬅️ Voltar ao Menu Principal</a>";
  html += "</div>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}


/**************************************************************
 *    IP/readings
 */

void handleReadings() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta http-equiv='refresh' content='2'>";
  html += "<title>Monitoramento - Presto Alimentos</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += "h1 { color: #2c5530; text-align: center; margin-bottom: 30px; }";
  html += ".status-card { background: #e8f4fd; padding: 20px; border-radius: 8px; margin: 15px 0; border-left: 4px solid #007cba; }";
  html += ".status-card.critical { background: #ffe6e6; border-left-color: #dc3545; }";
  html += ".status-card.warning { background: #fff3cd; border-left-color: #ffc107; }";
  html += ".status-card.success { background: #d4edda; border-left-color: #28a745; }";
  html += ".reading-item { display: flex; justify-content: space-between; align-items: center; padding: 10px 0; border-bottom: 1px solid #ddd; }";
  html += ".reading-item:last-child { border-bottom: none; }";
  html += ".label { font-weight: bold; color: #555; }";
  html += ".value { font-size: 1.2em; font-weight: bold; color: #007cba; }";
  html += ".value.critical { color: #dc3545; }";
  html += ".value.warning { color: #ffc107; }";
  html += ".value.success { color: #28a745; }";
  html += ".back-link { text-align: center; margin: 20px 0; }";
  html += ".back-link a { color: #007cba; text-decoration: none; font-weight: bold; padding: 10px 20px; background: #f8f9fa; border-radius: 5px; }";
  html += ".back-link a:hover { background: #e9ecef; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>📈 Monitoramento em Tempo Real</h1>";
  
  // Determinar classe do status baseado no nível
  String statusClass = "success";
  if (percentual_reservatorio < 20) statusClass = "critical";
  else if (percentual_reservatorio < 50) statusClass = "warning";
  
  html += "<div class='status-card " + statusClass + "'>";
  html += "<div class='reading-item'><span class='label'>💧 Nível do Reservatório:</span><span class='value " + statusClass + "'>" + String(percentual_reservatorio, 1) + "%</span></div>";
  html += "</div>";
  
  html += "<div class='status-card'>";
  html += "<div class='reading-item'><span class='label'>📏 Altura Total:</span><span class='value'>" + String(level_min, 1) + " cm</span></div>";
  html += "<div class='reading-item'><span class='label'>📐 Altura Útil:</span><span class='value'>" + String(level_min - level_max, 1) + " cm</span></div>";
  html += "<div class='reading-item'><span class='label'>📊 Altura Medida:</span><span class='value'>" + String(altura_medida, 1) + " cm</span></div>";
  html += "<div class='reading-item'><span class='label'>🔢 Batida Nr:</span><span class='value'>" + String(idBatida) + "</span></div>";
  html += "</div>";
  
  html += "<div class='status-card'>";
  html += "<div class='reading-item'><span class='label'>🌡️ Temperatura:</span><span class='value'>" + String(temperatura) + " °C</span></div>";
  html += "<div class='reading-item'><span class='label'>💧 Umidade:</span><span class='value'>" + String(humidade) + " %</span></div>";
  html += "</div>";
  
  html += "<div class='back-link'>";
  html += "<a href='/'>⬅️ Voltar ao Menu Principal</a>";
  html += "</div>";
  html += "</div></body></html>";
  server.send(200, "text/html", html);
}



/**************************************************************
 *    IP/config_mqtt
 */

void handleConfigMQTT() {
  if (server.method() == HTTP_POST) {
    String server_mqtt = server.arg("server");
    int port_mqtt = server.arg("port").toInt();
    String user_mqtt = server.arg("user");
    String pass_mqtt = server.arg("pass");

    // Salva na NVS
    save_flash_string(KEY_MQTT_SERVER, server_mqtt.c_str());
    save_flash_int(KEY_MQTT_PORT, port_mqtt);
    save_flash_string(KEY_MQTT_USER, user_mqtt.c_str());
    save_flash_string(KEY_MQTT_PASS, pass_mqtt.c_str());
    

    // Atualiza variáveis em RAM
    strncpy(MQTT_SERVER, server_mqtt.c_str(), sizeof(MQTT_SERVER));
    PORT_MQTT = port_mqtt;
    strncpy(MQTT_USERNAME, user_mqtt.c_str(), sizeof(MQTT_USERNAME));
    strncpy(MQTT_PASSWORD, pass_mqtt.c_str(), sizeof(MQTT_PASSWORD));

    String successHtml = "<!DOCTYPE html><html><head>";
    successHtml += "<meta charset='UTF-8'>";
    successHtml += "<title>Configuração Salva - Presto Alimentos</title>";
    successHtml += "<style>";
    successHtml += "body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }";
    successHtml += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); text-align: center; }";
    successHtml += "h1 { color: #28a745; margin-bottom: 20px; }";
    successHtml += ".success-message { background: #d4edda; color: #155724; padding: 20px; border-radius: 5px; margin: 20px 0; border: 1px solid #c3e6cb; }";
    successHtml += ".back-link a { color: #007cba; text-decoration: none; font-weight: bold; padding: 10px 20px; background: #f8f9fa; border-radius: 5px; }";
    successHtml += ".back-link a:hover { background: #e9ecef; }";
    successHtml += "</style></head><body>";
    successHtml += "<div class='container'>";
    successHtml += "<h1>✅ Configuração Salva com Sucesso!</h1>";
    successHtml += "<div class='success-message'>";
    successHtml += "<p>As configurações MQTT foram salvas na memória flash.</p>";
    successHtml += "<p><strong>Reinicie o dispositivo para aplicar as mudanças.</strong></p>";
    successHtml += "</div>";
    successHtml += "<div class='back-link'>";
    successHtml += "<a href='/'>⬅️ Voltar ao Menu Principal</a>";
    successHtml += "</div>";
    successHtml += "</div></body></html>";
    server.send(200, "text/html", successHtml);
    return;
  }

  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset='UTF-8'>";
  html += "<title>Configuração MQTT - Presto Alimentos</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }";
  html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
  html += "h1 { color: #2c5530; text-align: center; margin-bottom: 30px; }";
  html += ".form-group { margin: 20px 0; }";
  html += "label { display: block; font-weight: bold; color: #555; margin-bottom: 5px; }";
  html += "input[type='text'], input[type='number'], input[type='password'] { width: 100%; padding: 12px; border: 2px solid #ddd; border-radius: 5px; font-size: 16px; box-sizing: border-box; }";
  html += "input[type='text']:focus, input[type='number']:focus, input[type='password']:focus { border-color: #007cba; outline: none; }";
  html += "input[type='submit'] { width: 100%; padding: 15px; background: #007cba; color: white; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; margin-top: 20px; }";
  html += "input[type='submit']:hover { background: #005a87; }";
  html += ".info { background: #e8f4fd; padding: 15px; border-radius: 5px; margin: 20px 0; }";
  html += ".back-link { text-align: center; margin: 20px 0; }";
  html += ".back-link a { color: #007cba; text-decoration: none; font-weight: bold; padding: 10px 20px; background: #f8f9fa; border-radius: 5px; }";
  html += ".back-link a:hover { background: #e9ecef; }";
  html += "</style></head><body>";
  html += "<div class='container'>";
  html += "<h1>⚙️ Configuração MQTT</h1>";
  html += "<div class='info'>";
  html += "<p><strong>Atenção:</strong> Após salvar as configurações, reinicie o dispositivo para aplicar as mudanças.</p>";
  html += "</div>";
  html += "<form method='POST'>";
  html += "<div class='form-group'><label for='server'>Servidor MQTT:</label><input type='text' id='server' name='server' value='" + String(MQTT_SERVER) + "' required></div>";
  html += "<div class='form-group'><label for='port'>Porta:</label><input type='number' id='port' name='port' value='" + String(PORT_MQTT) + "' min='1' max='65535' required></div>";
  html += "<div class='form-group'><label for='user'>Usuário:</label><input type='text' id='user' name='user' value='" + String(MQTT_USERNAME) + "'></div>";
  html += "<div class='form-group'><label for='pass'>Senha:</label><input type='password' id='pass' name='pass' value='" + String(MQTT_PASSWORD) + "'></div>";
  html += "<input type='submit' value='💾 Salvar Configuração'>";
  html += "</form>";
  html += "<div class='back-link'>";
  html += "<a href='/'>⬅️ Voltar ao Menu Principal</a>";
  html += "</div>";
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}


/**************************************************************
 *    IP/ota - Atualização OTA via HTTP
 */

void handleOTA() {
  Serial.printf("WEB OTA: ===== DEBUG ENTRADA handleOTA =====\n");
  Serial.printf("WEB OTA: Método: %s\n", (server.method() == HTTP_POST) ? "POST" : "GET");
  Serial.printf("WEB OTA: Tem arg 'upload': %s\n", server.hasArg("upload") ? "SIM" : "NÃO");
  Serial.printf("WEB OTA: Args totais: %d\n", server.args());
  
  for (int i = 0; i < server.args(); i++) {
    Serial.printf("WEB OTA: Arg[%d]: %s = %s\n", i, server.argName(i).c_str(), server.arg(i).c_str());
  }
  
  if (server.method() == HTTP_POST && !server.hasArg("upload")) {
    // Resposta final após upload ESP-IDF nativo (só executa se NÃO tiver arg upload)
    Serial.println("WEB OTA: 📨 POST final recebido (sem arg upload)");
    if (ota_upload_success) {
      Serial.println("WEB OTA: ✅ Upload foi bem-sucedido, enviando resposta e agendando reboot...");
      server.sendHeader("Connection", "close");
      server.send(200, "text/plain", "✅ Firmware atualizado com sucesso! Dispositivo reiniciando em 5 segundos...");
      Serial.println("WEB OTA: ✅ Resposta HTTP 200 enviada ao navegador");
      server.client().flush();  // Força envio imediato dos dados
      delay(100);  // Pequeno delay para garantir envio
      Serial.println("WEB OTA: ⏳ Aguardando 5 segundos antes de reiniciar...");
      delay(5000);
      Serial.println("WEB OTA: 🔄 REINICIANDO ESP32 AGORA!");
      ESP.restart();
    } else {
      Serial.println("WEB OTA: ❌ Upload não foi bem-sucedido");
      server.sendHeader("Connection", "close");
      server.send(500, "text/plain", "❌ Erro: Upload não foi concluído ou falhou na validação");
    }
    return;  // Retorna após processar POST final
  } else if (server.hasArg("upload")) {
    // Handler de upload de arquivo ESP-IDF nativo
    HTTPUpload& upload = server.upload();
    
    if (upload.status == UPLOAD_FILE_START) {
      // Verificar se já há um upload em progresso
      if (ota_in_progress) {
        Serial.println("WEB OTA: ⚠️ Upload já em progresso, ignorando requisição duplicada");
        return;
      }
      
      ota_in_progress = true;  // Marca que upload está em progresso
      Serial.printf("WEB OTA: ===== INICIANDO OTA ESP-IDF NATIVO =====\n");
      Serial.printf("WEB OTA: Arquivo: %s\n", upload.filename.c_str());
      Serial.printf("WEB OTA: Free heap antes: %u bytes\n", ESP.getFreeHeap());
      
      // Desabilitar watchdog durante OTA para evitar timeout
      Serial.println("WEB OTA: Desabilitando watchdog...");
      disableLoopWDT();  // Desabilita watchdog do loop principal
      disableCore0WDT(); // Desabilita watchdog do Core 0
      disableCore1WDT(); // Desabilita watchdog do Core 1
      
      // Verificar se o arquivo tem extensão .bin
      if (!upload.filename.endsWith(".bin")) {
        Serial.println("WEB OTA: ❌ Arquivo deve ter extensão .bin");
        server.send(400, "text/plain", "❌ Erro: Arquivo deve ter extensão .bin");
        return;
      }
      
      // Obter partição de destino OTA
      ota_partition = esp_ota_get_next_update_partition(NULL);
      if (ota_partition == NULL) {
        Serial.println("WEB OTA: ❌ ERRO: Não foi possível encontrar partição OTA válida!");
        server.send(500, "text/plain", "❌ Erro: Partição OTA não encontrada");
        return;
      }
      
      Serial.printf("WEB OTA: Partição destino: %s (0x%06x, %u bytes)\n", 
                    ota_partition->label, ota_partition->address, ota_partition->size);
      
      // Inicializar operação OTA ESP-IDF
      Serial.println("WEB OTA: Chamando esp_ota_begin() - pode demorar 30-60 segundos...");
      esp_err_t err = esp_ota_begin(ota_partition, OTA_SIZE_UNKNOWN, &ota_handle);
      if (err != ESP_OK) {
        Serial.printf("WEB OTA: ❌ Falha ao iniciar ESP-IDF OTA: %s\n", esp_err_to_name(err));
        server.send(500, "text/plain", "❌ Erro ao iniciar OTA nativo");
        return;
      }
      
      Serial.printf("WEB OTA: ✅ OTA ESP-IDF iniciado com handle: %u\n", ota_handle);
      
      totalReceived = 0;
      lastReported = 0;
      md5.begin();
      ota_upload_success = false;  // Reseta flag de sucesso
      
      // NOTA: Não paramos ArduinoOTA para permitir uploads subsequentes via PlatformIO
      // ArduinoOTA.end();  // ❌ REMOVIDO - causava "No response" em uploads posteriores
      
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      // Adicionar dados ao cálculo MD5
      md5.add(upload.buf, upload.currentSize);
      
      // Escrever dados diretamente na partição OTA usando ESP-IDF
      esp_err_t err = esp_ota_write(ota_handle, upload.buf, upload.currentSize);
      if (err != ESP_OK) {
        Serial.printf("WEB OTA: ❌ Erro na escrita ESP-IDF: %s\n", esp_err_to_name(err));
        esp_ota_end(ota_handle);
        ota_handle = 0;
        ota_in_progress = false;  // Libera flag
        server.send(500, "text/plain", "❌ Erro na escrita de dados");
        return;
      }
      
      totalReceived += upload.currentSize;
      
      // Reportar progresso a cada 50KB para evitar spam
      if (totalReceived - lastReported >= 51200) {
        Serial.printf("WEB OTA: ✅ Escrevendo: %u KB na partição %s\n", 
                      totalReceived / 1024, ota_partition->label);
        lastReported = totalReceived;
      }
      
    } else if (upload.status == UPLOAD_FILE_END) {
      Serial.printf("WEB OTA: ===== FINALIZANDO OTA ESP-IDF =====\n");
      Serial.printf("WEB OTA: Total gravado: %u bytes na partição %s\n", 
                    totalReceived, ota_partition->label);
      
      // Finalizar cálculo MD5
      md5.calculate();
      firmwareHash = md5.toString();
      Serial.printf("WEB OTA: MD5 do firmware enviado: %s\n", firmwareHash.c_str());
      
      if (totalReceived == 0) {
        Serial.println("WEB OTA: ❌ Nenhum dado foi gravado!");
        esp_ota_end(ota_handle);
        ota_handle = 0;
        ota_in_progress = false;  // Libera flag
        server.send(500, "text/plain", "❌ Erro: Nenhum dado recebido");
        return;
      }
      
      // Finalizar operação OTA ESP-IDF
      esp_err_t err = esp_ota_end(ota_handle);
      if (err != ESP_OK) {
        Serial.printf("WEB OTA: ❌ Falha ao finalizar ESP-IDF OTA: %s\n", esp_err_to_name(err));
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
          Serial.println("WEB OTA: ❌ Validação de imagem falhou - firmware inválido");
        } else if (err == ESP_ERR_INVALID_ARG) {
          Serial.println("WEB OTA: ❌ Handle OTA inválido");
        } else {
          Serial.printf("WEB OTA: ❌ Erro desconhecido: %s (0x%x)\n", esp_err_to_name(err), err);
        }
        ota_handle = 0;
        ota_in_progress = false;  // Libera flag
        server.send(500, "text/plain", "❌ Erro na validação do firmware");
        return;
      }
      
      Serial.printf("WEB OTA: ✅ Firmware gravado com sucesso na partição %s!\n", ota_partition->label);
      
      // Reinicializar watchdog após OTA (versão compatível)
      Serial.println("WEB OTA: Reinicializando watchdog...");
      enableLoopWDT();   // Reabilita watchdog do loop
      enableCore0WDT();  // Reabilita watchdog do Core 0  
      enableCore1WDT();  // Reabilita watchdog do Core 1
      
      // Configurar partição de boot para o novo firmware
      esp_err_t err2 = esp_ota_set_boot_partition(ota_partition);
      if (err2 != ESP_OK) {
        Serial.printf("WEB OTA: ❌ Falha ao configurar boot: %s\n", esp_err_to_name(err2));
        ota_in_progress = false;  // Libera flag
        server.send(500, "text/plain", "❌ Erro ao configurar nova partição de boot");
        return;
      }
      
      Serial.printf("WEB OTA: ✅ Partição de boot configurada para: %s (0x%06x)\n", 
                    ota_partition->label, ota_partition->address);
      
      // Verificar configuração final
      const esp_partition_t* new_boot = esp_ota_get_boot_partition();
      if (new_boot && new_boot->address == ota_partition->address) {
        Serial.printf("WEB OTA: ✅ CONFIRMADO: Boot partition = %s (0x%06x)\n", 
                      new_boot->label, new_boot->address);
        Serial.println("WEB OTA: ✅ SUCESSO TOTAL - OTA completo e verificado!");
        ota_upload_success = true;  // Marca upload como bem-sucedido
        
        // ✅ ENVIAR RESPOSTA IMEDIATAMENTE E REINICIAR
        Serial.println("WEB OTA: 📤 Enviando resposta HTTP 200 ao navegador...");
        server.sendHeader("Connection", "close");
        server.sendHeader("Cache-Control", "no-cache");
        server.send(200, "text/plain", "✅ Firmware atualizado com sucesso! Dispositivo reiniciando...");
        server.client().stop();  // Fecha conexão imediatamente
        Serial.println("WEB OTA: ✅ Resposta enviada e conexão fechada");
        Serial.println("WEB OTA: ⏳ Aguardando 1 segundo antes de reiniciar...");
        delay(1000);
        Serial.println("WEB OTA: 🔄 REINICIANDO ESP32 AGORA!");
        ESP.restart();  // Nunca retorna daqui
      } else {
        Serial.println("WEB OTA: ⚠️ ATENÇÃO: Partição de boot não foi alterada como esperado");
        ota_upload_success = false;
        ota_in_progress = false;  // Libera flag em caso de erro
        server.send(500, "text/plain", "❌ Erro: Falha ao configurar partição de boot");
        ota_handle = 0;
      }
      
    } else if (upload.status == UPLOAD_FILE_ABORTED) {
      Serial.println("WEB OTA: ⚠️ Upload cancelado pelo usuário");
      if (ota_handle != 0) {
        esp_ota_end(ota_handle);
        ota_handle = 0;
      }
      
      // Reinicializar watchdog após abortar
      Serial.println("WEB OTA: Reinicializando watchdog após cancelamento...");
      enableLoopWDT();
      enableCore0WDT();
      enableCore1WDT();
    }
  } else {
    // Página de upload principal
    String html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<title>ESP32 OTA Update - Presto Alimentos</title>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 40px; background: #f0f0f0; }";
    html += ".container { max-width: 600px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }";
    html += "h2 { color: #333; text-align: center; }";
    html += ".info { background: #e8f4fd; padding: 15px; border-radius: 5px; margin: 20px 0; }";
    html += ".upload-form { margin: 20px 0; }";
    html += "input[type='file'] { width: 100%; padding: 10px; margin: 10px 0; border: 2px dashed #ccc; border-radius: 5px; }";
    html += "input[type='submit'] { width: 100%; padding: 15px; background: #007cba; color: white; border: none; border-radius: 5px; font-size: 16px; cursor: pointer; }";
    html += "input[type='submit']:hover { background: #005a87; }";
    html += ".progress-container { display: none; margin: 20px 0; }";
    html += ".progress-bar { width: 100%; height: 30px; background: #f0f0f0; border-radius: 15px; overflow: hidden; }";
    html += ".progress-fill { height: 100%; background: linear-gradient(90deg, #4CAF50, #45a049); transition: width 0.3s ease; }";
    html += ".progress-text { text-align: center; margin: 10px 0; font-weight: bold; }";
    html += ".status { margin: 10px 0; padding: 10px; border-radius: 5px; }";
    html += ".success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }";
    html += ".error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }";
    html += ".back-link { text-align: center; margin: 20px 0; }";
    html += ".back-link a { color: #007cba; text-decoration: none; font-weight: bold; }";
    html += "</style></head><body>";
    html += "<div class='container'>";
    html += "<h2>🚀 Atualização OTA - " + String(NOME_EQUIPAMENTO) + "</h2>";
    html += "<div class='info'>";
    html += "<p><strong>Device:</strong> " + String(DISPOSITIVO_ID) + "</p>";
    html += "<p><strong>IP:</strong> " + WiFi.localIP().toString() + "</p>";
    html += "<p><strong>Versão Atual:</strong> " + String(VERSION) + "</p>";
    html += "<p><strong>Free Heap:</strong> " + String(ESP.getFreeHeap()) + " bytes</p>";
    html += "<p><strong>Flash Size:</strong> " + String(ESP.getFlashChipSize()) + " bytes</p>";
    html += "<p><strong>Sketch Space:</strong> " + String(ESP.getFreeSketchSpace()) + " bytes</p>";
    html += "</div>";
    html += "<div style='background: #fff3cd; padding: 15px; border-radius: 5px; margin: 20px 0; border-left: 4px solid #ffc107;'>";
    html += "<h4>⚠️ Instruções Importantes:</h4>";
    html += "<ul style='margin: 10px 0; padding-left: 20px;'>";
    html += "<li>Use apenas arquivos .bin compilados para ESP32</li>";
    html += "<li>Verifique se o firmware é compatível com este hardware</li>";
    html += "<li>Não interrompa o processo durante o upload</li>";
    html += "<li>Em caso de erro 9, verifique a integridade do arquivo .bin</li>";
    html += "</ul>";
    html += "</div>";
    html += "<form id='uploadForm' class='upload-form' method='POST' action='/ota?upload=1' enctype='multipart/form-data'>";
    html += "<p><strong>Selecionar arquivo firmware (.bin):</strong></p>";
    html += "<input type='file' name='firmware' accept='.bin' required><br>";
    html += "<input type='submit' value='📤 Upload Firmware'>";
    html += "</form>";
    html += "<div id='progressContainer' class='progress-container'>";
    html += "<div class='progress-text' id='progressText'>Preparando upload...</div>";
    html += "<div class='progress-bar'><div id='progressFill' class='progress-fill' style='width: 0%'></div></div>";
    html += "<div id='status' class='status'></div>";
    html += "</div>";
    html += "<div class='back-link'>";
    html += "<a href='/'>⬅️ Voltar ao Menu Principal</a>";
    html += "</div>";
    html += "<script>";
    html += "function validateFile(file) {";
    html += "  const status = document.getElementById('status');";
    html += "  status.className = 'status';";
    html += "  status.innerHTML = '';";
    html += "  if (!file) {";
    html += "    status.className = 'status error';";
    html += "    status.innerHTML = '❌ Nenhum arquivo selecionado';";
    html += "    return false;";
    html += "  }";
    html += "  if (!file.name.toLowerCase().endsWith('.bin')) {";
    html += "    status.className = 'status error';";
    html += "    status.innerHTML = '❌ Arquivo deve ter extensão .bin';";
    html += "    return false;";
    html += "  }";
    html += "  if (file.size < 100000) {";
    html += "    status.className = 'status error';";
    html += "    status.innerHTML = '⚠️ Arquivo muito pequeno (' + Math.round(file.size/1024) + ' KB). Verifique se é um firmware válido.';";
    html += "    return false;";
    html += "  }";
    html += "  if (file.size > " + String(ESP.getFreeSketchSpace()) + ") {";
    html += "    status.className = 'status error';";
    html += "    status.innerHTML = '❌ Arquivo muito grande (' + Math.round(file.size/1024) + ' KB). Máximo: ' + Math.round(" + String(ESP.getFreeSketchSpace()) + "/1024) + ' KB';";
    html += "    return false;";
    html += "  }";
    html += "  status.className = 'status success';";
    html += "  status.innerHTML = '✅ Arquivo válido: ' + file.name + ' (' + Math.round(file.size/1024) + ' KB)';";
    html += "  return true;";
    html += "}";
    html += "document.querySelector('input[type=\"file\"]').addEventListener('change', function(e) {";
    html += "  validateFile(e.target.files[0]);";
    html += "});";
    html += "document.getElementById('uploadForm').addEventListener('submit', function(e) {";
    html += "  e.preventDefault();";
    html += "  const fileInput = this.querySelector('input[type=\"file\"]');";
    html += "  const file = fileInput.files[0];";
    html += "  if (!validateFile(file)) {";
    html += "    return false;";
    html += "  }";
    html += "  const formData = new FormData(this);";
    html += "  const progressContainer = document.getElementById('progressContainer');";
    html += "  const progressFill = document.getElementById('progressFill');";
    html += "  const progressText = document.getElementById('progressText');";
    html += "  const status = document.getElementById('status');";
    html += "  progressContainer.style.display = 'block';";
    html += "  this.style.display = 'none';";
    html += "  progressText.innerHTML = 'Iniciando upload do firmware: ' + file.name + ' (' + Math.round(file.size/1024) + ' KB)';";
    html += "  const xhr = new XMLHttpRequest();";
    html += "  xhr.upload.addEventListener('progress', function(e) {";
    html += "    if (e.lengthComputable) {";
    html += "      const percent = Math.round((e.loaded / e.total) * 100);";
    html += "      progressFill.style.width = percent + '%';";
    html += "      progressText.innerHTML = 'Uploading... ' + percent + '% (' + Math.round(e.loaded/1024) + ' KB / ' + Math.round(e.total/1024) + ' KB)';";
    html += "    }";
    html += "  });";
    html += "  xhr.addEventListener('load', function() {";
    html += "    if (xhr.status === 200) {";
    html += "      progressText.innerHTML = '✅ Upload concluído com sucesso!';";
    html += "      status.className = 'status success';";
    html += "      status.innerHTML = xhr.responseText;";
    html += "      setTimeout(() => { location.href = '/'; }, 5000);";
    html += "    } else {";
    html += "      progressText.innerHTML = '❌ Erro no upload';";
    html += "      status.className = 'status error';";
    html += "      status.innerHTML = xhr.responseText;";
    html += "    }";
    html += "  });";
    html += "  xhr.addEventListener('error', function() {";
    html += "    progressText.innerHTML = '❌ Erro de conexão';";
    html += "    status.className = 'status error';";
    html += "    status.innerHTML = 'Erro de comunicação com o dispositivo';";
    html += "  });";
    html += "  xhr.open('POST', '/ota?upload=1');";
    html += "  xhr.send(formData);";
    html += "});";
    html += "</script>";
    html += "</div></body></html>";
    server.send(200, "text/html", html);
  }
}