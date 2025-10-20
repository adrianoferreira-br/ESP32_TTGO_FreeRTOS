/*
 *
 */
#include <Arduino.h>
#include "constants.h"



//Definir em constant.h
//#define Adriano // Presto | Adriano
//#define EQUIP_RESERVATORIO // EQUIP_PRENSA | EQUIP_PROCESSAMENTO | EQUIP_LINEA | EQUIP_RESERVATORIO | EQUIP_OUTRO

// Informações do equipamento 
char CLIENTE[32] = "adriano"; // presto | adriano
char LOCAL[32] = "floripa";      // palhoca | floripa | 
char TIPO_EQUIPAMENTO[32] = "reservatorio"; // prensa | processamento | linea | reservatorio | teste
char ID_EQUIPAMENTO[32] = "002"; // Identificação do equipamento (deve ser único para cada equipamento)
char DISPOSITIVO_ID[64] = "adriano-fln-l01-tst-001"; // Identificação do dispositivo (deve ser único para cada equipamento, usa no mqtt client ID

char NOME_EQUIPAMENTO[32] = "cx_agua";   // prensa_1 | prensa_2 | prensa_3 | prc_1  | linea_1 | linea_2 | teste | cx_agua
char LINHA[32] = "L01"; // Linha de produção onde o equipamento está instalado: L01 | L02 | L03 | etc
char FABRICANTE_MAQUINA[64] = ""; //Panitec | Schuler | Komatsu | etc
char MODELO_MAQUINA[64] = ""; // Prensa X1000 | Forno Y2000 | etc
char TIPO_SENSOR[32] = "Ultrassonico"; // Ultrassonico | DHT22 | Batida | TensãoBateria
char OBSERVACAO_READINGS[64] = "Testes dev"; // Observação para as leituras enviadas via MQTT
char OBSERVACAO_DEVICE_INFO[64] = "Testes dev"; // Observação para as informações do dispositivo enviadas via MQTT
char OBSERVACAO_SETTINGS[64] = "Testes dev"; // Observação para as configurações do dispositivo enviadas via MQTT


// Informação do coletor de dados
const String VERSION_TTGO = "V1.1";
char* VERSION = "v25.10.09";                             // Versão atual de uso. ex. "v25.4.15"
char PLACA[64] = "TTGO T-Display V1.1"; // TTGO T-Display | Heltec WiFi Kit 32 | M5Stack Core2
char* FIREBASE_HOST = "seu_projeto.firebaseio.com"; // Host do Firebase
char* FIREBASE_AUTH = "sua_chave_de_autenticacao"; // Chave
char MODELO_SENSOR[32] = ""; // Modelo do sensor
char FABRICANTE_SENSOR[32] = ""; // Fabricante do sensor
char VERSAO_HARDWARE[32] = "v1.0"; // Versão do hardware
char DATA_INSTALACAO[32] = "2025-10-02"; // Data de instalação do equipamento no local 
int   SAMPLE_INTERVAL = 30; //em segundos - Intervalo de amostragem em segundos (padrão 30 minutos)





// Informações do sensor




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
    192.168.127.246        presto-plh-l01-rsv-001
*/



//login:  PRESTO!_admin
//senha: paodequeijo100g
//prensa_3 sem sensor
/*
Acesso ao SSH
53nh@S3rv3r3
2022
indx4

*/