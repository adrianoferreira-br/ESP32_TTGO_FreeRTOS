/*  File: wifi_mqtt.cpp
 *  Description:  WIFI and MQTT
 *  date: 2025-01-14
 */
#include "wifi_mqtt.h"
#include "constants.h"
#include "main.h"

WiFiClient espClient;
PubSubClient client(espClient);

const char* ssid = SSID;                //"STARLINK";//"PhoneAdr"; // Substitua pelo seu SSID 
const char* password = PASSWORD;        //"11121314";//"UDJ1-ddsp";// "SUA_SENHA"; // Substitua pela sua senha 
const char* mqtt_server = MQTT_SERVER;  //"192.168.100.4";//"broker.hivemq.com";
const int port_mqtt = PORT_MQTT;        //1883

//String ip;

/*
 *
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
  client.publish("AdrPresto", "Hello MQTT from ESP32 adr"); //"test/topic"  )
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