/*
 *
 */
#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// Informações para acesso a Internet
extern char* SSID;      // "STARLINK";            // Substitua pelo seu SSID para acesso a Internet
extern char* PASSWORD; //"11121314";              // Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
extern char* MQTT_SERVER; //"192.168.100.4";      // Substitua pelo endereço do servidor MQTT
extern int PORT_MQTT;                             // Porta do servidor MQTT      padrão: 1883
extern char* MQTT_USERNAME;
extern char* MQTT_PASSWORD;                       // TODO: criptografar a senha em outro momento. (Cuidado com o Git)

// Informações referente a aplicação
extern char* NOME_EQUIPAMENTO;                    // Nome do equipamento   ex. "prensa_1"
extern char* VERSION;                            // Versão atual de uso. ex. "v25.4.14"

// Análise de corrente e verificação de estado de operação do equipamento (Processando ou aguarndando insumo)
extern float CALIBRATION_CURRENT_FACTOR;          // Fator de calibração (varia de acordo com o sensor e meio)  
extern float CALIBRATION_VOLTAGE_FACTOR;            
extern int LIMIAR_SUPERIOR;                     // Limiar superior para detecção de equipamento processando
extern int LIMIAR_INFERIOR;                     // Limiar inferior para detecção de equipamento aguardando insumo
extern int LIMITE_MAX;  
extern int AMOSTRAS_COMPROVACAO_ESTADO;           // Tempo de comprovação de estado

// Informações para acesso ao Firebase
extern char* FIREBASE_HOST;                       // Host do Firebase
extern char* FIREBASE_AUTH;                       // Chave de autenticação do Firebase   TODO: Criptografar a chave de autenticação

// Versão
extern char* VERSION;                             // Versão atual de uso. ex. "v25.4.15"





#endif // CONSTANTS_H_ 


//PRETO!    #prestoalimentos