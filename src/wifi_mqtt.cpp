/*  File: wifi_mqtt.cpp
 *  Description:  WIFI and MQTT
 *  date: 2025-01-14
 */
#include "wifi_mqtt.h"
#include "constants.h"
#include "main.h"

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = SSID;//"STARLINK";//"PhoneAdr"; // Substitua pelo seu SSID 
const char* password = PASSWORD;//"11121314";//"UDJ1-ddsp";// "SUA_SENHA"; // Substitua pela sua senha 
const char* mqtt_server = MQTT_SERVER; //"192.168.100.4";//"broker.hivemq.com";
const int port_mqtt = PORT_MQTT; //1883
//String ip;

/*
 *
 */
void setup_wifi() 
{
   int i = 0;

   delay(10); 
   Serial.begin(115200); 
   Serial.println(); 
   Serial.print("Conectando a "); 
   Serial.println(ssid); 
   WiFi.begin(ssid, password); 
   do 
   { 
        delay(500); 
        Serial.print("."); 
        i++;
   } while (((WiFi.status() != WL_CONNECTED) && (i<10)));

   if (WiFi.status() != WL_CONNECTED)
   {
      Serial.println("Falha ao conectar na rede");
      return;
   } else {
    Serial.println(""); 
    Serial.println("WiFi conectado"); 
    Serial.print("Endereço IP: "); 
    Serial.println(WiFi.localIP());    
   }  
}

/*
 *
 */
void setup_mqtt()
{
   client.setServer(mqtt_server, port_mqtt);
   client.setCallback(callback); 
}
 
/*
 *
 */
void loop_mqqt() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) { 
    reconnect(); 
  } 
  client.loop(); 
  client.publish("test/topic", "Hello MQTT from ESP32"); 
  delay(2000);   
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
  while (!client.connected()) 
  { 
    Serial.print("Tentando conectar ao MQTT..."); 
    if (client.connect("ESP32Client")) 
    { 
      Serial.println("Conectado"); 
      client.subscribe("test/topic"); 
    } 
    else 
    { 
      Serial.print("falhou, rc="); 
      Serial.print(client.state()); 
      Serial.println(" tentando novamente em 5 segundos"); 
      delay(5000); 
    } 
  } 
}
