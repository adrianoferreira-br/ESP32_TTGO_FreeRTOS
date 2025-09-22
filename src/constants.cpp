/*
 *
 */
#include "constants.h"


#define Adriano // Presto | Adriano

// Informações do equipamento
char* NOME_EQUIPAMENTO = "teste";   // prensa_1 | prensa_2 | prensa_3 | prc_1  | linea_1 | linea_2 | teste | cx_agua

// Versão
char* VERSION = "v25.09.21";  // Versão atual de uso.

// Informações do sensor
/*#if defined prensa_1 || defined prensa_2 || defined prensa_3 || defined prc_1 || defined linea_1 || defined linea_2
  #define SENSOR_BATIDA true    
#elif defined cx_agua || defined teste
  #define SENSOR_WATER_LEVEL true
#else
  #define SENSOR_BATIDA false
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
//char* MQTT_SERVER = "192.168.0.203";    // Substitua pelo endereço do servidor MQTT
char* MQTT_SERVER = "192.168.127.248";
int PORT_MQTT = 1883;                   // Porta do servidor MQTT      padrão: 1883
char* MQTT_USERNAME = "Adriano";
char* MQTT_PASSWORD = "Rafa1404";       // TODO: criptografar a senha em outro momento. (Cuidado com o Git)



#elif defined Adriano

// Informações para acesso a Internet
char* SSID = "STARLINK"; //"PhoneAdr";  // Substitua pelo seu SSID para acesso a Internet
char* PASSWORD = "11121314"; //"UDJ1-ddsp"; // Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
char* MQTT_SERVER = "192.168.100.4";          //"172.24.96.1";//"82d3aa30f5744315a2bdde52bafe1ec7.s1.eu.hivemq.cloud"; // Substitua pelo endereço do servidor MQTT
int PORT_MQTT = 1883;                             // Porta do servidor MQTT      padrão: 1883
char* MQTT_USERNAME = "Adriano";
char* MQTT_PASSWORD = "Rafa1404";                 // TODO: criptografar a senha em outro momento. (Cuidado com o Git)


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
