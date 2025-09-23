/*
 *
 */
#ifndef CONSTANTS_H_
#define CONSTANTS_H_


// Endereço e tamanho da EEPROM
#define EEPROM_SIZE 256 // Tamanho necessário para armazenar um float (4 bytes)
#define EEPROM_ADDRESS 0 // Endereço inicial para armazenar o float
#define ADDR_LENGTH_MAX   0   // float (4 bytes)
#define ADDR_IP          10   // string (ex: 16 bytes)
#define ADDR_WIFI_SSID   30   // string (ex: 32 bytes)
#define ADDR_WIFI_PASS   70   // string (ex: 32 bytes)
#define ADDR_MQTT_SERVER 110  // string (ex: 32 bytes)
#define ADDR_MQTT_PORT   150  // int (4 bytes)
#define ADDR_MQTT_USER   160  // string (ex: 32 bytes)
#define ADDR_MQTT_PASS   200  // string (ex: 32 bytes)



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
extern char* VERSION;                             // Versão atual de uso. ex. "v25.4.14"
extern bool SENSOR_TEMPERATURE;                   // true | false
extern bool SENSOR_WATER_LEVEL;                   // true | false
extern bool SENSOR_BATIDA;                        // true | false
extern bool SENSOR_BATTERY_VOLTAGE;            // true | false

// Definições de constantes para o projeto
extern float length_max;


// Informações para acesso ao Firebase
extern char* FIREBASE_HOST;                       // Host do Firebase
extern char* FIREBASE_AUTH;                       // Chave de autenticação do Firebase   TODO: Criptografar a chave de autenticação

// Versão
extern char* VERSION;                             // Versão atual de uso. ex. "v25.4.15"







#endif // CONSTANTS_H_ 


//PRETO!    #prestoalimentos