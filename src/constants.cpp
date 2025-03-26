/*
 *
 */
#include "constants.h"


// Informações para acesso a Internet
char* SSID = "STARLINK"; //"PRESTO!";//"PhoneAdr";      //     // Substitua pelo seu SSID para acesso a Internet
char* PASSWORD = "11121314"; //"#prestoalimentos";//"UDJ1-ddsp"; //      // Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
char* MQTT_SERVER = "192.168.100.4";          //"192.168.0.203";//"172.24.96.1";//"82d3aa30f5744315a2bdde52bafe1ec7.s1.eu.hivemq.cloud"; // Substitua pelo endereço do servidor MQTT
int PORT_MQTT = 1883;                             // Porta do servidor MQTT      padrão: 1883
char* MQTT_USERNAME = "Adriano";
char* MQTT_PASSWORD = "Rafa1404";                 // TODO: criptografar a senha em outro momento. (Cuidado com o Git)

// Informações referente a aplicação
char* NOME_EQUIPAMENTO = "prensa_1";                    // Nome do equipamento

// Análise de corrente e verificação de estado de operação do equipamento (Processando ou aguarndando insumo)
float CALIBRATION_CURRENT_FACTOR = 2.8;         // Fator de calibração (varia de acordo com o sensor e meio)  
float CALIBRATION_VOLTAGE_FACTOR = 171.8;
int LIMIAR_SUPERIOR = 550;                      // Limiar superior para detecção de equipamento processando
int LIMIAR_INFERIOR = 300;                      // Limiar inferior para detecção de equipamento aguardando insumo
int LIMITE_MAX = 1000;
int AMOSTRAS_COMPROVACAO_ESTADO = 3;             // Tempo de comprovação de estado

// Informações para acesso ao Firebase
char* FIREBASE_HOST = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; // Host do Firebase
char* FIREBASE_AUTH = "<YOUR_GoogleAPIKey_HERE>";                         // Chave de autenticação do Firebase   TODO: Criptografar a chave de autenticação




