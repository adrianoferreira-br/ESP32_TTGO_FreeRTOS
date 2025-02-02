/*
 *
 */
#include "constants.h"


// Informações para acesso a Internet
char* SSID = "PhoneAdr";      // "STARLINK";      // Substitua pelo seu SSID para acesso a Internet
char* PASSWORD = "UDJ1-ddsp"; //"11121314";       // Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
char* MQTT_SERVER = "82d3aa30f5744315a2bdde52bafe1ec7.s1.eu.hivemq.cloud"; //"192.168.100.4";          // Substitua pelo endereço do servidor MQTT
int PORT_MQTT = 8883;                             // Porta do servidor MQTT      padrão: 1883
char* MQTT_USERNAME = "Adriano";
char* MQTT_PASSWORD = "Rafa1404";                 // TODO: criptografar a senha em outro momento. (Cuidado com o Git)

// Análise de corrente e verificação de estado de operação do equipamento (Processando ou aguarndando insumo)
float CALIBRATION_CURRENT_FACTOR = 1.1;         // Fator de calibração (varia de acordo com o sensor e meio)  
float CALIBRATION_VOLTAGE_FACTOR = 173.1;
int LIMIAR_SUPERIOR = 1200;                      // Limiar superior para detecção de equipamento processando
int LIMIAR_INFERIOR = 500;                      // Limiar inferior para detecção de equipamento aguardando insumo
int LIMITE_MAX = 2000;
int AMOSTRAS_COMPROVACAO_ESTADO = 3;             // Tempo de comprovação de estado

// Informações para acesso ao Firebase
char* FIREBASE_HOST = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; // Host do Firebase
char* FIREBASE_AUTH = "<YOUR_GoogleAPIKey_HERE>";                         // Chave de autenticação do Firebase   TODO: Criptografar a chave de autenticação




