/*
 *
 */
#ifndef CONSTANTS_H_
#define CONSTANTS_H_




// prefs
#define KEY_LEVEL_MAX    "level_max"
#define KEY_LEVEL_MIN    "level_min"
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


// topico
extern char topico[64];

// Informações referente a aplicação
extern char* NOME_EQUIPAMENTO;                    // Nome do equipamento   ex. "prensa_1"
extern char* VERSION;                             // Versão atual de uso. ex. "v25.4.14"
extern bool SENSOR_TEMPERATURE;                   // true | false
extern bool SENSOR_WATER_LEVEL;                   // true | false
extern bool SENSOR_BATIDA;                        // true | false
extern bool SENSOR_BATTERY_VOLTAGE;            // true | false

// Definições de constantes para o projeto
extern float level_max;
extern float level_min;


// Informações para acesso ao Firebase
extern char* FIREBASE_HOST;                       // Host do Firebase
extern char* FIREBASE_AUTH;                       // Chave de autenticação do Firebase   TODO: Criptografar a chave de autenticação

// Versão
extern char* VERSION;                             // Versão atual de uso. ex. "v25.4.15"



// informações do sensor ultrassônico
extern float percentual_reservatorio;   // percentual do reservatório
extern float altura_medida;                // altura máxima do reservatório em cm (deve ser configurada conforme o local de instalação)
extern float altura_reservatorio;  // distância máxima do sensor ultrassônico em cm (padrão 400cm para o JSN-SR04T)

// informações do sensor DHT22
extern float temperatura;           // temperatura em Celsius   
extern float humidade;               // umidade em %
// informações do sensor de batida
extern long idBatida;      // id da última batida registrada
// informações do sensor de tensão da bateria
extern float tensao_bateria;       // tensão da bateria em volts



// Informações do equipamento 
extern char* CLIENTE;
extern char* LOCAL;
extern char* LINHA;
extern char* FABRICANTE_MAQUINA;
extern char* NOME_EQUIPAMENTO;    
extern char* MODELO_MAQUINA;
extern char* ID_EQUIPAMENTO;
extern char* TIPO_EQUIPAMENTO;
// informação do sensor indx4
extern char* PLACA; // TTGO T-Display | Heltec WiFi Kit 32 | M5Stack Core2
extern char* MODELO_SENSOR; // Modelo do sensor:  "JSN-SR04T", "DHT22", "Batida", "TensãoBateria"
extern char* FABRICANTE_SENSOR; // Fabricante do sensor: "JSN", "Aosong", "Outros"
//extern char* SERIAL_SENSOR; // Número de série do sensor
extern char* DATA_INSTALACAO; // Data de instalação do sensor
extern char* TIPO_SENSOR;
extern char* DISPOSITIVO_ID;
extern char* VERSAO_HARDWARE;
// informações extras
extern int SAMPLE_INTERVAL; // em segundos - Intervalo de amostragem em segundos (padrão 5 minutos)
extern char* DATA_INSTALACAO; // Data de instalação do sensor
extern char* OBSERVACAO_READINGS; // Observação para as leituras enviadas via MQTT


// Informação do coletor de dados
extern char* OBSERVACAO_DEVICE_INFO; // Observação para as informações do dispositivo enviadas via MQTT



#endif // CONSTANTS_H_


