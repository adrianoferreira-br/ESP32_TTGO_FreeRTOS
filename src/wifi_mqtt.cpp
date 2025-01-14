#include "wifi_mqtt.h"


const char* ssid = "STARLINK";//"PhoneAdr"; // Substitua pelo seu SSID 
const char* password = "11121314";//"UDJ1-ddsp";// "SUA_SENHA"; // Substitua pela sua senha 
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

/*
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
*/
void setup_wifi() 
{
   delay(10); 
   Serial.begin(115200); 
   Serial.println(); 
   Serial.print("Conectando a "); 
   Serial.println(ssid); 
   WiFi.begin(ssid, password); 
   while (WiFi.status() != WL_CONNECTED) 
   { 
    delay(500); 
    Serial.print("."); 
   } 
   Serial.println(""); 
   Serial.println("WiFi conectado"); 
   Serial.print("Endereço IP: "); 
   Serial.println(WiFi.localIP()); 

  client.setServer(mqtt_server, 1883); 
  client.setCallback(callback);
  
}


/*
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
