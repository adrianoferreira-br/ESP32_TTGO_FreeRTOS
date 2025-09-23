#include <WiFi.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Preferences.h>
#include "constants.h"
#include "wifi_mqtt.h"
#include "web_server.h"
#include "state.h"



extern WebServer server;
WebServer server(80); // Porta 80 padrão HTTP


/**************************************************************
 * SETUP WEB SERVER
 */


void setup_webserver()  
{
  server.on("/", handleRoot);
  server.on("/config_mqtt", handleConfigMQTT); // <-- nova rota
  server.begin();
}


/**************************************************************
 * LOOP WEB SERVER
 */

void loop_webserver() {
  server.handleClient();
}




/**************************************************************
 *   IP/
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
    prefs.begin("mqtt", false);
    prefs.putString("server", server_mqtt);
    prefs.putInt("port", port_mqtt);
    prefs.putString("username", user_mqtt);
    prefs.putString("password", pass_mqtt);
    prefs.end();

    // Atualiza variáveis em RAM
    strncpy(MQTT_SERVER, server_mqtt.c_str(), sizeof(MQTT_SERVER));
    PORT_MQTT = port_mqtt;
    strncpy(MQTT_USERNAME, user_mqtt.c_str(), sizeof(MQTT_USERNAME));
    strncpy(MQTT_PASSWORD, pass_mqtt.c_str(), sizeof(MQTT_PASSWORD));

    server.send(200, "text/html", "<h2>Configuração salva! Reinicie o ESP32 para aplicar.</h2><a href='/'>Voltar</a>");
    return;
  }

  String html = "<html><body>";
  html += "<h2>Configurar MQTT</h2>";
  html += "<form method='POST'>";
  html += "Servidor: <input name='server' value='" + String(MQTT_SERVER) + "'><br>";
  html += "Porta: <input name='port' value='" + String(PORT_MQTT) + "'><br>";
  html += "Usuário: <input name='user' value='" + String(MQTT_USERNAME) + "'><br>";
  html += "Senha: <input name='pass' value='" + String(MQTT_PASSWORD) + "' type='password'><br>";
  html += "<input type='submit' value='Salvar'>";
  html += "</form></body></html>";

  server.send(200, "text/html", html);
}