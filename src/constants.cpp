/*
 *
 */
#include <Arduino.h>
#include "constants.h"



//Definir em constant.h
#define Adriano // Presto | Adriano


// Informações do cliente
char CLIENTE[32] = "presto"; // presto | adriano
char LOCAL[32] = "palhoca";      // palhoca | floripa | 


// Informações do equipamento
char TIPO_EQUIPAMENTO[32] = "teste"; // dosadora | prensa | processamento | linea | reservatorio | teste
char ID_EQUIPAMENTO[32] = "001"; // Identificação do equipamento (deve ser único para cada equipamento)
char NOME_EQUIPAMENTO[32] = "teste";   // dosadora_1 | prensa_1 | prensa_2 | prensa_3 | prc_1  | linea_1 | linea_2 | teste | cx_agua
char LINHA[32] = "l01"; // Linha de produção onde o equipamento está instalado: L01 | L02 | L03 | etc
char FABRICANTE_MAQUINA[64] = "Panitec"; //Panitec | Schuler | Komatsu | etc
char MODELO_MAQUINA[64] = ""; // Prensa X1000 | Forno Y2000 | etc
char SERIAL_MAQUINA[64] = "SN000000"; // Número de série da máquina


// Informações do dispositivo
char DISPOSITIVO_ID[64] = "presto-plh-tst-001"; // Identificação do dispositivo (deve ser único para cada equipamento, usa no mqtt client ID
char TIPO_SENSOR[32] = "pulse eletrico"; // Ultrassonico | DHT22 | Batida | TensãoBateria
char OBSERVACAO_READINGS[64] = ""; // Observação para as leituras enviadas via MQTT
char OBSERVACAO_DEVICE_INFO[64] = ""; // Observação para as informações do dispositivo enviadas via MQTT
char OBSERVACAO_SETTINGS[64] = ""; // Observação para as configurações do dispositivo enviadas via MQTT

char PLACA_SOC[64] = "TTGO T-Display V1.1"; // TTGO T-Display | Heltec WiFi Kit 32 | M5Stack Core2 | LILYGO S3 T-display
char MODELO_SENSOR[32] = ""; // Modelo do sensor
char FABRICANTE_SENSOR[32] = ""; // Fabricante do sensor
char VERSAO_HARDWARE[32] = "v1.0"; // Versão do hardware

char DATA_INSTALACAO[32] = "2025-11-12"; // Data de instalação do equipamento no local 
char* VERSION = "v25.12.12";                             // Versão atual de uso. ex. "v25.4.15"


// informações da aplicação
int  SAMPLE_INTERVAL = 60; //em segundos - Intervalo de envio MQTT do reservatório
int  sample_interval_batch = 60; //em segundos - Intervalo de envio batch_time via MQTT (padrão 60 segundos)

// Informação do coletor de dados

char* FIREBASE_HOST = "seu_projeto.firebaseio.com"; // Host do Firebase
char* FIREBASE_AUTH = "sua_chave_de_autenticacao"; // Chave






// Informações do sensor




// Definições de constantes para o projeto

#ifdef Presto  
// Informações para acesso a Internet
char* SSID = "PRESTO!_IoT";                 //Substitua pelo seu SSID para acesso a Internet
char* PASSWORD = "prestoiot100";    //Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
char MQTT_SERVER[32] =  "192.168.0.210";//"srv.vamodale.com"; //"192.168.100.4";          //"172.24.96.1";//"82d3aa30f5744315a2bdde52bafe1ec7.s1.eu.hivemq.cloud"; // Substitua pelo endereço do servidor MQTT
int  PORT_MQTT = 1883;                       // Porta do servidor MQTT      padrão: 1883
char MQTT_USERNAME[32] = "";
char MQTT_PASSWORD[32] = "";     // TODO: criptografar a senha em outro momento. (Cuidado com o Git)
char topico[64];



#elif defined Adriano

// Informações para acesso a Internet
char* SSID = "STARLINK"; //"PhoneAdr";  // Substitua pelo seu SSID para acesso a Internet
char* PASSWORD = "11121314"; //"UDJ1-ddsp"; // Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
char MQTT_SERVER[32] =  "srv.vamodale.com";//"mqtt.soscode.com.br"; //"192.168.100.4";          //"172.24.96.1";//"82d3aa30f5744315a2bdde52bafe1ec7.s1.eu.hivemq.cloud"; // Substitua pelo endereço do servidor MQTT
int PORT_MQTT = 1883;//18883;                       // Porta do servidor MQTT      padrão: 1883
char MQTT_USERNAME[32] = "";//"admin";
char MQTT_PASSWORD[32] = "";//"mqtt123";     // TODO: criptografar a senha em outro momento. (Cuidado com o Git)
char topico[64];


#endif // Presto

/*

comando para apagar a flash:
pio run --target erase --environment ttgo-t-display




// Exemplo de JSON para atualização de configurações do reservatório via mqtt
{
  "level_max": 25.0,
  "level_min": 2.0,
  "sample_time_s": 60
  "filter_threshold": 15.0
}

// Exemplo de JSON para atualização de informações do dispositivo via mqtt
{  
  "dispositivo_id": "presto-poa-l02-rsv-001",
  "cliente": "presto",
  "local": "porto_alegre",
  "linha": "L02",
  "nome_equip": "reservatorio_principal"    
}

// Exemplo de JSON para atualização de configurações de conectividade via mqtt
{
  "wifi_ssid": "Nova_Rede",
  "wifi_password": "nova_senha",
  "mqtt_server": "novo.servidor.com",
  "mqtt_port": 8883,
  "mqtt_user": "novo_usuario",
  "mqtt_password": "nova_senha_mqtt"
}


// Exemplo de JSON para atualização de informações do dispositivo via mqtt
{
  "fabricante_maquina": "Schuler",
  "modelo_maquina": "Prensa Hidraulica SH-2000",
  "tipo_sensor": "Pressao",
  "fabricante_sensor": "Danfoss", 
  "modelo_sensor": "MBS-3000",
  "versao_hardware": "v2.1",
  "data_instalacao": "2025-10-20",
  "observacao_device_info": "Instalado na linha de produção principal"
}


*/



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