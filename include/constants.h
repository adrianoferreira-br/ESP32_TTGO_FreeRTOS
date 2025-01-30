/*
 *
 */
#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// Informações para acesso a Internet
extern const char* SSID;      // "STARLINK";            // Substitua pelo seu SSID para acesso a Internet
extern const char* PASSWORD; //"11121314";              // Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
extern const char* MQTT_SERVER; //"192.168.100.4";      // Substitua pelo endereço do servidor MQTT
extern const int PORT_MQTT;                             // Porta do servidor MQTT      padrão: 1883
extern const char* MQTT_USERNAME;
extern const char* MQTT_PASSWORD;                       // TODO: criptografar a senha em outro momento. (Cuidado com o Git)

// Análise de corrente e verificação de estado de operação do equipamento (Processando ou aguarndando insumo)
extern const float CALIBRATION_CURRENT_FACTOR;          // Fator de calibração (varia de acordo com o sensor e meio)  
extern const float LIMIAR_SUPERIOR;                     // Limiar superior para detecção de equipamento processando
extern const float LIMIAR_INFERIOR;                     // Limiar inferior para detecção de equipamento aguardando insumo
extern const int AMOSTRAS_COMPROVACAO_ESTADO;           // Tempo de comprovação de estado

// Informações para acesso ao Firebase
extern const char* FIREBASE_HOST;                       // Host do Firebase
extern const char* FIREBASE_AUTH;                       // Chave de autenticação do Firebase   TODO: Criptografar a chave de autenticação




#endif // CONSTANTS_H_ 