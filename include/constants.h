/*
 *
 */
#ifndef CONSTANTS_H_
#define CONSTANTS_H_

// ============================================================================
// CONFIGURAÇÃO DO CLIENTE E EQUIPAMENTO
// ============================================================================
#define EQUIP_PRENSA // EQUIP_PRENSA | EQUIP_PROCESSAMENTO | EQUIP_LINEA | EQUIP_RESERVATORIO | EQUIP_OUTRO

// ============================================================================
// ATIVAÇÃO AUTOMÁTICA DOS SENSORES BASEADO NO TIPO DE EQUIPAMENTO
// ============================================================================
#if defined(EQUIP_PRENSA) || defined(EQUIP_PROCESSAMENTO) || defined(EQUIP_LINEA)
  #define SENSOR_BATIDA    
#elif defined(EQUIP_RESERVATORIO)
  #define SENSOR_TEMPERATURE
  #define SENSOR_WATER_LEVEL     
#elif defined(EQUIP_OUTRO)
  #define SENSOR_TEMPERATURE
  #define SENSOR_BATTERY_VOLTAGE
#else
  // Configuração padrão quando nenhum equipamento específico é definido
  #define SENSOR_BATTERY_VOLTAGE
#endif

// Sensor de temperatura infravermelho MLX90614
// #define SENSOR_MLX90614  // Descomente para ativar o sensor MLX90614


// prefs - settings_network 
#define KEY_IP           "ip"
#define KEY_WIFI_SSID    "ssid"
#define KEY_WIFI_PASS    "wifipass"
#define KEY_MQTT_SERVER  "mqtt_server"
#define KEY_MQTT_PORT    "mqtt_port"
#define KEY_MQTT_USER    "mqtt_user"
#define KEY_MQTT_PASS    "mqtt_pass"

// prefs - settings_device
#define KEY_CLIENTE      "client"
#define KEY_LOCAL        "location"
#define KEY_LINHA        "line"
#define KEY_TIPO_EQUIP   "type_equip"
#define KEY_ID_EQUIP     "equip_id"
#define KEY_NOME_EQUIP   "name_equip"
#define KEY_DISPOSITIVO_ID  "device_id"
#define KEY_PLACA_SOC       "soc_board"
#define KEY_FABRICANTE_MAQUINA "manufacturer_machine"
#define KEY_MODELO_MAQUINA  "model_machine"
#define KEY_SERIAL_MAQUINA  "serial_machine"
#define KEY_TIPO_SENSOR     "type_sensor"
#define KEY_DEVICE_INFO     "notes_device_info"
#define KEY_OBSERVACAO_SETTINGS "notes_settings"
#define KEY_OBSERVACAO_READINGS "notes_readings"
#define KEY_FABRICANTE_SENSOR "manufacturer_sensor"
#define KEY_MODELO_SENSOR   "model_sensor"
#define KEY_VERSAO_HARDWARE "hardware_version"
#define KEY_DATA_INSTALACAO "installation_date"

// prefs - settings_sensor
#define KEY_LEVEL_MAX     "level_max"
#define KEY_LEVEL_MIN     "level_min"
#define KEY_SAMPLE_TIME_S "sample_time_s"
#define KEY_SAMPLE_INTERVAL_BATCH "sample_interval_batch"
#define KEY_FILTER_THRESHOLD "filter_threshold"


// tipos de sensores
#define EQUIP_TIPO_PRENSA           1
#define EQUIP_TIPO_PROCESSAMENTO    2
#define EQUIP_TIPO_LINEA            3
#define EQUIP_TIPO_RESERVATORIO     4
#define EQUIP_TIPO_OUTRO            99


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

// Definições de constantes para o projeto
extern float level_max;
extern float level_min;
extern float filter_threshold; // Threshold do filtro em % (padrão 10%)
extern int SAMPLE_INTERVAL; // em segundos - Intervalo de amostragem em segundos (padrão 5 minutos)
extern int sample_interval_batch; // em segundos - Intervalo de envio batch_time via MQTT (padrão 60 segundos)

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
extern char CLIENTE[32];
extern char LOCAL[32];
extern char LINHA[32];
extern char FABRICANTE_MAQUINA[64];
extern char NOME_EQUIPAMENTO[32];    
extern char MODELO_MAQUINA[64];
extern char SERIAL_MAQUINA[64];
extern char ID_EQUIPAMENTO[32];
extern char TIPO_EQUIPAMENTO[32];
// informação do sensor indx4
extern char PLACA_SOC[64]; // TTGO T-Display | Heltec WiFi Kit 32 | M5Stack Core2 | LILYGO S3 T-display
extern char MODELO_SENSOR[32]; // Modelo do sensor:  "JSN-SR04T", "DHT22", "Batida", "TensãoBateria"
extern char FABRICANTE_SENSOR[32]; // Fabricante do sensor: "JSN", "Aosong", "Outros"
//extern char* SERIAL_SENSOR; // Número de série do sensor
extern char DATA_INSTALACAO[32]; // Data de instalação do sensor
extern char TIPO_SENSOR[32];
extern char DISPOSITIVO_ID[64];
extern char VERSAO_HARDWARE[32];

// Informações referente a aplicação e versão
extern char* VERSION;                             // Versão atual de uso. ex. "v25.4.15"

// Informações para acesso ao Firebase
extern char* FIREBASE_HOST;                       // Host do Firebase
extern char* FIREBASE_AUTH;                       // Chave de autenticação do Firebase   TODO: Criptografar a chave de autenticação

// informações extras
extern char OBSERVACAO_READINGS[64]; // Observação para as leituras enviadas via MQTT
// Informação do coletor de dados
extern char OBSERVACAO_DEVICE_INFO[64]; // Observação para as informações do dispositivo enviadas via MQTT
extern char OBSERVACAO_SETTINGS[64]; // Observação para as configurações do dispositivo enviadas via MQTT






#endif // CONSTANTS_H_


