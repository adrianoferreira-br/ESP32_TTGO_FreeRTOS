/*
 *
 */
#include "constants.h"


// Informações para acesso a Internet
const char* SSID = "PhoneAdr";      // "STARLINK";      // Substitua pelo seu SSID para acesso a Internet
const char* PASSWORD = "UDJ1-ddsp"; //"11121314";       // Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
const char* MQTT_SERVER = "82d3aa30f5744315a2bdde52bafe1ec7.s1.eu.hivemq.cloud"; //"192.168.100.4";          // Substitua pelo endereço do servidor MQTT
const int PORT_MQTT = 8883;                             // Porta do servidor MQTT      padrão: 1883
const char* MQTT_USERNAME = "Adriano";
const char* MQTT_PASSWORD = "Rafa1404";                 // TODO: criptografar a senha em outro momento. (Cuidado com o Git)

// Análise de corrente e verificação de estado de operação do equipamento (Processando ou aguarndando insumo)
const float CALIBRATION_CURRENT_FACTOR = 1.1;         // Fator de calibração (varia de acordo com o sensor e meio)  
const float LIMIAR_SUPERIOR = 2.0;                      // Limiar superior para detecção de equipamento processando
const float LIMIAR_INFERIOR = 1.5;                      // Limiar inferior para detecção de equipamento aguardando insumo
const int AMOSTRAS_COMPROVACAO_ESTADO = 3;             // Tempo de comprovação de estado

// Informações para acesso ao Firebase
const char* FIREBASE_HOST = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; // Host do Firebase
const char* FIREBASE_AUTH = "<YOUR_GoogleAPIKey_HERE>";                         // Chave de autenticação do Firebase   TODO: Criptografar a chave de autenticação




