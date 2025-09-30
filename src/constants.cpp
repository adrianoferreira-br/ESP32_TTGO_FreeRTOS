/*
 *
 */
#include "constants.h"


#define Adriano // Presto | Adriano

// Informações do equipamento 
char* CLIENTE = "presto"; // Presto | Adriano
char* LOCAL = "palhoca";      // palhoca | floripa | 
char* TIPO_EQUIPAMENTO = "reservatorio"; // prensa | processamento | linea | reservatorio | teste
char* ID_EQUIPAMENTO = "001"; // Identificação do equipamento (deve ser único para cada equipamento)
char* DISPOSITIVO_ID = "presto-plh-l01-rsv-001"; // Identificação do dispositivo (deve ser único para cada equipamento)

char* NOME_EQUIPAMENTO = "reservatorio";   // prensa_1 | prensa_2 | prensa_3 | prc_1  | linea_1 | linea_2 | teste | cx_agua
char* LINHA = "L01"; // Linha de produção onde o equipamento está instalado: L01 | L02 | L03 | etc
char* FABRICANTE_MAQUINA = ""; //Panitec | Schuler | Komatsu | etc
char* MODELO_MAQUINA = ""; // Prensa X1000 | Forno Y2000 | etc
char* TIPO_SENSOR = "Ultrassonico"; // Ultrassonico | DHT22 | Batida | TensãoBateria
char* OBSERVACAO_READINGS = "Testes Adriano"; // Observação para as leituras enviadas via MQTT
char* OBSERVACAO_DEVICE_INFO = "Testes Adriano"; // Observação para as informações do dispositivo enviadas via MQTT


// Informação do coletor de dados
char* VERSION = "v25.09.30";  // Versão atual de uso. YY.MM.DD
char* PLACA = "TTGO T-Display V1.1"; // TTGO T-Display | Heltec WiFi Kit 32 | M5Stack Core2
char* FIREBASE_HOST = "seu_projeto.firebaseio.com"; // Host do Firebase
char* FIREBASE_AUTH = "sua_chave_de_autenticacao"; // Chave
char* MODELO_SENSOR = "JSN-SR04T"; // Modelo do sensor
char* FABRICANTE_SENSOR = "Fabricante do Sensor"; // Fabricante do sensor
char* MAC_ADDR = "AA:BB:CC:DD:EE:FF"; // Endereço MAC do sensor
char* VERSAO_HARDWARE = "v1.0"; // Versão do hardware
char* DATA_INSTALACAO = "2025-10-01"; // Data de  



// Informações do sensor
/*#if defined prensa_1 || defined prensa_2 || defined prensa_3 || defined prc_1 || defined linea_1 || defined linea_2
  #define SENSOR_BATIDA true    
#elif defined cx_agua || defined teste
  #define SENSOR_WATER_LEVEL true
#else
  #define SENSOR_BATIDA fals
  #define SENSOR_WATER_LEVEL false
#endif
*/


bool SENSOR_TEMPERATURE         = true;   // true | false
bool SENSOR_WATER_LEVEL         = true;   // true | false
bool SENSOR_BATIDA              = false;    // true | false
bool SENSOR_BATTERY_VOLTAGE     = true;   // true | false


// Definições de constantes para o projeto

#ifdef Presto  
// Informações para acesso a Internet
char* SSID = "PRESTO!_IoT";                 //Substitua pelo seu SSID para acesso a Internet
char* PASSWORD = "prestoiot100";    //Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
char MQTT_SERVER[32] =  "srv.vamodale.com"; //"192.168.100.4";          //"172.24.96.1";//"82d3aa30f5744315a2bdde52bafe1ec7.s1.eu.hivemq.cloud"; // Substitua pelo endereço do servidor MQTT
int  PORT_MQTT = 1883;                       // Porta do servidor MQTT      padrão: 1883
char MQTT_USERNAME[32] = "indx4";
char MQTT_PASSWORD[32] = "indx4_senha";     // TODO: criptografar a senha em outro momento. (Cuidado com o Git)
char topico[64];



#elif defined Adriano

// Informações para acesso a Internet
char* SSID = "STARLINK"; //"PhoneAdr";  // Substitua pelo seu SSID para acesso a Internet
char* PASSWORD = "11121314"; //"UDJ1-ddsp"; // Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
char MQTT_SERVER[32] =  "srv.vamodale.com"; //"192.168.100.4";          //"172.24.96.1";//"82d3aa30f5744315a2bdde52bafe1ec7.s1.eu.hivemq.cloud"; // Substitua pelo endereço do servidor MQTT
int PORT_MQTT = 1883;                       // Porta do servidor MQTT      padrão: 1883
char MQTT_USERNAME[32] = "indx4";
char MQTT_PASSWORD[32] = "indx4_senha";     // TODO: criptografar a senha em outro momento. (Cuidado com o Git)
char topico[64];


#endif // Presto





/*

----------------------- P R E S T O    A L I M E N T O S -----------------------
prc_01
    192.168.0.163         a0-dd-6c-74-fe-04     dinâmico
linea_1
    192.168.0.95          a0-dd-6c-6f-60-4c     dinâmico
linea_2
    192.168.0.167         a0-dd-6c-74-f4-f0     dinamico
prensa_1
    192.168.0.100         a0-dd-6c-74-f4-cc     dinâmico
prensa_2
    192.168.0.104         a0-dd-6c-74-f9-d4     dinâmico
prensa_3
    192.168.0.175         a0-dd-6c-74-f5-f8     dinâmico


teste2
    192.168.127.246
*/
