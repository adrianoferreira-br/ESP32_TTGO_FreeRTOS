/*
 *
 */
#ifndef CONSTANTS_H_
#define CONSTANTS_H_




// prefs
#define KEY_LENGTH_MAX   "length_max"
#define KEY_IP           "ip"
#define KEY_WIFI_SSID    "ssid"
#define KEY_WIFI_PASS    "wifipass"
#define KEY_MQTT_SERVER  "mqtt_server"
#define KEY_MQTT_PORT    "mqtt_port"
#define KEY_MQTT_USER    "mqtt_user"
#define KEY_MQTT_PASS    "mqtt_pass"




// Informações para acesso a Internet
extern char* SSID;      // "STARLINK";            // Substitua pelo seu SSID para acesso a Internet
extern char* PASSWORD; //"11121314";              // Substitua pela sua senha de acesso a Internet

// Informações para acesso ao servidor MQTT
extern char MQTT_SERVER[32]; //"192.168.100.4";      // Substitua pelo endereço do servidor MQTT
extern int PORT_MQTT;                             // Porta do servidor MQTT      padrão: 1883
extern char MQTT_USERNAME[32];
extern char MQTT_PASSWORD[32];                       // TODO: criptografar a senha em outro momento. (Cuidado com o Git)
// informação do topoico MQTT
extern char* CLIENTE;
extern char* LOCAL;
extern char* TIPO_EQUIPAMENTO;
extern int DISPOSITIVO_ID;

// topico
extern char topico[32];

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



// informações do sensor ultrassônico
extern float percentual_reservatorio; // percentual do reservatório
extern float length_max;             // altura máxima do reservatório em cm (deve ser configurada conforme o local de instalação)
extern float distance_max;           // distância máxima do sensor ultrassônico em cm (padrão 400cm para o JSN-SR04T)





#endif // CONSTANTS_H_ 


//PRETO!    #prestoalimentos