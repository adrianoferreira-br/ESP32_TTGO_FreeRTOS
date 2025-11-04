#include "mem_flash.h"
#include <Preferences.h>
#include <string.h>
#include "constants.h"

Preferences prefs;

/*
 * Função de configuração da memória flash (Preferences)
 */ 
void setup_mem_flash() { 
    // Não é necessário inicializar Preferences aqui, pois cada função abre/fecha o namespace
}

/*
 * Salva um float
 */
void save_flash_float(const char* key, float value) {
    prefs.begin("flash", false);
    prefs.putFloat(key, value);
    prefs.end();
    Serial.println("Float salvo na flash: " + String(value));
}

/*
 * Lê um float
 */
float read_flash_float(const char* key) {
    prefs.begin("flash", true);
    
    // Verifica se a chave existe antes de tentar ler para evitar mensagens de erro
    float value = 0.0;
    if (prefs.isKey(key)) {
        value = prefs.getFloat(key, 0.0);
    }
    
    prefs.end();
    return value;
}

/*
 * Salva uma string
 */
void save_flash_string(const char* key, const char* value) {
    prefs.begin("flash", false);
    prefs.putString(key, value);
    prefs.end();
    Serial.println("String salva na flash: " + String(value));
}

/*
 * Lê uma string
 */
void read_flash_string(const char* key, char* buffer, int maxLen) {
    prefs.begin("flash", true);
    
    // Verifica se a chave existe antes de tentar ler para evitar mensagens de erro
    if (prefs.isKey(key)) {
        String val = prefs.getString(key, "");
        strncpy(buffer, val.c_str(), maxLen);
        buffer[maxLen - 1] = '\0'; // Garante terminação
    } else {
        // Chave não existe, retorna string vazia
        buffer[0] = '\0';
    }
    
    prefs.end();
}

/*
 * Salva um int
 */
void save_flash_int(const char* key, int value) {
    prefs.begin("flash", false);
    prefs.putInt(key, value);
    prefs.end();
    Serial.println("Int salvo na flash: " + String(value));
}

/*
 * Lê um int
 */
int read_flash_int(const char* key) {
    prefs.begin("flash", true);
    
    // Verifica se a chave existe antes de tentar ler para evitar mensagens de erro
    int value = 0;
    if (prefs.isKey(key)) {
        value = prefs.getInt(key, 0);
    }
    
    prefs.end();
    return value;
}




/*
 * Carrega todas as configurações básicas da flash
 * Deve ser chamada no início do programa (setup)
 */
void load_all_settings_from_flash() {
    Serial.println("📂 Carregando configurações da flash...");
    
    // ============================================ CONFIGURAÇÕES DO RESERVATÓRIO ===
    level_max = read_flash_float(KEY_LEVEL_MAX);
    level_min = read_flash_float(KEY_LEVEL_MIN);
    SAMPLE_INTERVAL = read_flash_int(KEY_SAMPLE_TIME_S);
    
    // Validação e valores padrão para reservatório
    if (level_max <= 0.0 || level_max > 400 || isnan(level_max)) {
        level_max = 20.0; // padrão
        save_flash_float(KEY_LEVEL_MAX, level_max);
        Serial.println("⚠️ Level_max inválido, usando padrão: " + String(level_max) + " cm");
    } else {
        Serial.println("✅ Level_max: " + String(level_max) + " cm");
    }
    
    if (level_min <= 0.0 || level_min > 400 || isnan(level_min)) {
        level_min = 400.0; // padrão
        save_flash_float(KEY_LEVEL_MIN, level_min);
        Serial.println("⚠️ Level_min inválido, usando padrão: " + String(level_min) + " cm");
    } else {
        Serial.println("✅ Level_min: " + String(level_min) + " cm");
    }
    
    if (SAMPLE_INTERVAL <= 0 || SAMPLE_INTERVAL > 3600) {
        SAMPLE_INTERVAL = 300; // padrão 300 segundos (5 minutos)
        save_flash_int(KEY_SAMPLE_TIME_S, SAMPLE_INTERVAL);
        Serial.println("⚠️ Sample_interval inválido, usando padrão: " + String(SAMPLE_INTERVAL) + " segundos");
    } else {
        Serial.println("✅ Sample_interval: " + String(SAMPLE_INTERVAL) + " segundos");
    }
    
    // ▶ FILTER THRESHOLD
    filter_threshold = read_flash_float(KEY_FILTER_THRESHOLD);
    if (filter_threshold <= 0.0 || filter_threshold > 50.0 || isnan(filter_threshold)) {
        filter_threshold = 10.0; // padrão 10%
        save_flash_float(KEY_FILTER_THRESHOLD, filter_threshold);
        Serial.println("⚠️ Filter_threshold inválido, usando padrão: " + String(filter_threshold) + "%");
    } else {
        Serial.println("✅ Filter_threshold: " + String(filter_threshold) + "%");
    }
    
    // ================================== CONFIGURAÇÕES DO DISPOSITIVO ===

    
    // CLIENTE
    char client_tmp[32];
    read_flash_string(KEY_CLIENTE, client_tmp, sizeof(client_tmp));
    if (strlen(client_tmp) > 0) {
        // ✅ ATUALIZA a variável global com o valor da flash
        strcpy(CLIENTE, client_tmp);  
        Serial.println("✅ CLIENTE carregado da flash: " + String(CLIENTE));
    } else {
        // ✅ Salva o valor padrão na flash para próximas vezes
        save_flash_string(KEY_CLIENTE, CLIENTE);
        Serial.println("⚠️ CLIENTE vazio, usando padrão: " + String(CLIENTE));
    }

    // LOCALIZAÇÃO
    char location_tmp[32];
    read_flash_string(KEY_LOCAL, location_tmp, sizeof(location_tmp));
    if (strlen(location_tmp) > 0) {
        // ✅ ATUALIZA a variável global com o valor da flash
        strcpy(LOCAL, location_tmp);
        Serial.println("✅ LOCAL carregado da flash: " + String(LOCAL));
    } else {
        // ✅ Salva o valor padrão na flash para próximas vezes
        save_flash_string(KEY_LOCAL, LOCAL);
        Serial.println("⚠️ LOCAL vazio, usando padrão: " + String(LOCAL));
    }

    // TIPO DO DISPOSITIVO
    char device_type_tmp[32];
    read_flash_string(KEY_TIPO_EQUIP, device_type_tmp, sizeof(device_type_tmp));
    if (strlen(device_type_tmp) > 0) {
        // ✅ ATUALIZA a variável global com o valor da flash
        strcpy(TIPO_EQUIPAMENTO, device_type_tmp);
        Serial.println("✅ TIPO_EQUIPAMENTO carregado da flash: " + String(TIPO_EQUIPAMENTO));
    } else {
        // ✅ Salva o valor padrão na flash para próximas vezes
        save_flash_string(KEY_TIPO_EQUIP, TIPO_EQUIPAMENTO);
        Serial.println("⚠️ TIPO_EQUIPAMENTO vazio, usando padrão: " + String(TIPO_EQUIPAMENTO));
    }
    
    // ID DO EQUIPAMENTO
    char equipment_id_tmp[32];
    read_flash_string(KEY_ID_EQUIP, equipment_id_tmp, sizeof(equipment_id_tmp));
    if (strlen(equipment_id_tmp) > 0) {
        // ✅ ATUALIZA a variável global com o valor da flash
        strcpy(ID_EQUIPAMENTO, equipment_id_tmp);
        Serial.println("✅ ID_EQUIPAMENTO carregado da flash: " + String(ID_EQUIPAMENTO));
    } else {
        // ✅ Salva o valor padrão na flash para próximas vezes
        save_flash_string(KEY_ID_EQUIP, ID_EQUIPAMENTO);
        Serial.println("⚠️ ID_EQUIPAMENTO vazio, usando padrão: " + String(ID_EQUIPAMENTO));
    }

    //NOME DO DISPOSITIVO
    char device_name_tmp[32];
    read_flash_string(KEY_NOME_EQUIP, device_name_tmp, sizeof(device_name_tmp));
    if (strlen(device_name_tmp) > 0) {
        // ✅ ATUALIZA a variável global com o valor da flash
        strcpy(NOME_EQUIPAMENTO, device_name_tmp);
        Serial.println("✅ NOME_EQUIPAMENTO carregado da flash: " + String(NOME_EQUIPAMENTO));
    } else {
        // ✅ Salva o valor padrão na flash para próximas vezes
        save_flash_string(KEY_NOME_EQUIP, NOME_EQUIPAMENTO);
        Serial.println("⚠️ NOME_EQUIPAMENTO vazio, usando padrão: " + String(NOME_EQUIPAMENTO));
    }



    // ============================================================== CONFIGURAÇÕES DE REDE ===
    char ssid_tmp[32], password_tmp[64];
    read_flash_string(KEY_WIFI_SSID, ssid_tmp, sizeof(ssid_tmp));
    read_flash_string(KEY_WIFI_PASS, password_tmp, sizeof(password_tmp));
    
    if (strlen(ssid_tmp) > 0) {
        Serial.println("✅ WiFi SSID carregado: " + String(ssid_tmp));
    } else {
        Serial.println("⚠️ WiFi SSID vazio, usando padrão do constants.h");
    }
    
    // ======================================================== CONFIGURAÇÕES MQTT ===

    char mqtt_server_tmp[32], mqtt_user_tmp[32], mqtt_pass_tmp[32];
    read_flash_string(KEY_MQTT_SERVER, mqtt_server_tmp, sizeof(mqtt_server_tmp));
    read_flash_string(KEY_MQTT_USER, mqtt_user_tmp, sizeof(mqtt_user_tmp));
    read_flash_string(KEY_MQTT_PASS, mqtt_pass_tmp, sizeof(mqtt_pass_tmp));
    int mqtt_port_tmp = read_flash_int(KEY_MQTT_PORT);    

    if (strlen(mqtt_server_tmp) > 0) {
        strncpy(MQTT_SERVER, mqtt_server_tmp, sizeof(MQTT_SERVER) - 1);
        MQTT_SERVER[sizeof(MQTT_SERVER) - 1] = '\0';
        Serial.println("✅ MQTT Server carregado: " + String(MQTT_SERVER));
    } else {
        // ✅ APLICA o valor padrão que já está em MQTT_SERVER (constants.h)
        Serial.println("⚠️ MQTT Server vazio, usando padrão: " + String(MQTT_SERVER));
    }
    
    if (mqtt_port_tmp > 0) {
        PORT_MQTT = mqtt_port_tmp;
        Serial.println("✅ MQTT Port carregado: " + String(PORT_MQTT));
    } else {
        // ✅ APLICA o valor padrão que já está em PORT_MQTT (constants.h)
        Serial.println("⚠️ MQTT Port inválido, usando padrão: " + String(PORT_MQTT));
    }
    
    if (strlen(mqtt_user_tmp) > 0) {
        strncpy(MQTT_USERNAME, mqtt_user_tmp, sizeof(MQTT_USERNAME) - 1);
        MQTT_USERNAME[sizeof(MQTT_USERNAME) - 1] = '\0';
        Serial.println("✅ MQTT User carregado: " + String(MQTT_USERNAME));
    } else {
        // ✅ APLICA o valor padrão que já está em MQTT_USERNAME (constants.h)
        Serial.println("⚠️ MQTT User vazio, usando padrão: " + String(MQTT_USERNAME));
    }
    
    if (strlen(mqtt_pass_tmp) > 0) {
        strncpy(MQTT_PASSWORD, mqtt_pass_tmp, sizeof(MQTT_PASSWORD) - 1);
        MQTT_PASSWORD[sizeof(MQTT_PASSWORD) - 1] = '\0';
        Serial.println("✅ MQTT Password carregado: [HIDDEN]");
    } else {
        // ✅ APLICA o valor padrão que já está em MQTT_PASSWORD (constants.h)
        Serial.println("⚠️ MQTT Password vazio, usando padrão: [HIDDEN]");
    }
    
    // ============================================================== CONFIGURAÇÕES DE DISPOSITIVO ===
    // DISPOSITIVO_ID
    char dispositivo_id_tmp[64];
    read_flash_string(KEY_DISPOSITIVO_ID, dispositivo_id_tmp, sizeof(dispositivo_id_tmp));
    if (strlen(dispositivo_id_tmp) > 0) {
        strcpy(DISPOSITIVO_ID, dispositivo_id_tmp);
        Serial.println("✅ DISPOSITIVO_ID carregado da flash: " + String(DISPOSITIVO_ID));
    } else {
        save_flash_string(KEY_DISPOSITIVO_ID, DISPOSITIVO_ID);
        Serial.println("⚠️ DISPOSITIVO_ID vazio, usando padrão: " + String(DISPOSITIVO_ID));
    }

    // FABRICANTE_MAQUINA
    char fabricante_maquina_tmp[64];
    read_flash_string(KEY_FABRICANTE_MAQUINA, fabricante_maquina_tmp, sizeof(fabricante_maquina_tmp));
    if (strlen(fabricante_maquina_tmp) > 0) {
        strcpy(FABRICANTE_MAQUINA, fabricante_maquina_tmp);
        Serial.println("✅ FABRICANTE_MAQUINA carregado da flash: " + String(FABRICANTE_MAQUINA));
    } else {
        save_flash_string(KEY_FABRICANTE_MAQUINA, FABRICANTE_MAQUINA);
        Serial.println("⚠️ FABRICANTE_MAQUINA vazio, usando padrão: " + String(FABRICANTE_MAQUINA));
    }

    // MODELO_MAQUINA
    char modelo_maquina_tmp[64];
    read_flash_string(KEY_MODELO_MAQUINA, modelo_maquina_tmp, sizeof(modelo_maquina_tmp));
    if (strlen(modelo_maquina_tmp) > 0) {
        strcpy(MODELO_MAQUINA, modelo_maquina_tmp);
        Serial.println("✅ MODELO_MAQUINA carregado da flash: " + String(MODELO_MAQUINA));
    } else {
        save_flash_string(KEY_MODELO_MAQUINA, MODELO_MAQUINA);
        Serial.println("⚠️ MODELO_MAQUINA vazio, usando padrão: " + String(MODELO_MAQUINA));
    }

    // TIPO_SENSOR
    char tipo_sensor_tmp[32];
    read_flash_string(KEY_TIPO_SENSOR, tipo_sensor_tmp, sizeof(tipo_sensor_tmp));
    if (strlen(tipo_sensor_tmp) > 0) {
        strcpy(TIPO_SENSOR, tipo_sensor_tmp);
        Serial.println("✅ TIPO_SENSOR carregado da flash: " + String(TIPO_SENSOR));
    } else {
        save_flash_string(KEY_TIPO_SENSOR, TIPO_SENSOR);
        Serial.println("⚠️ TIPO_SENSOR vazio, usando padrão: " + String(TIPO_SENSOR));
    }

    // OBSERVACAO_DEVICE_INFO -> KEY_DEVICE_INFO
    char observacao_device_info_tmp[64];
    read_flash_string(KEY_DEVICE_INFO, observacao_device_info_tmp, sizeof(observacao_device_info_tmp));
    if (strlen(observacao_device_info_tmp) > 0) {
        strcpy(OBSERVACAO_DEVICE_INFO, observacao_device_info_tmp);
        Serial.println("✅ OBSERVACAO_DEVICE_INFO carregado da flash: " + String(OBSERVACAO_DEVICE_INFO));
    } else {
        save_flash_string(KEY_DEVICE_INFO, OBSERVACAO_DEVICE_INFO);
        Serial.println("⚠️ OBSERVACAO_DEVICE_INFO vazio, usando padrão: " + String(OBSERVACAO_DEVICE_INFO));
    }

    // OBSERVACAO_SETTINGS
    char observacao_settings_tmp[64];
    read_flash_string(KEY_OBSERVACAO_SETTINGS, observacao_settings_tmp, sizeof(observacao_settings_tmp));
    if (strlen(observacao_settings_tmp) > 0) {
        strcpy(OBSERVACAO_SETTINGS, observacao_settings_tmp);
        Serial.println("✅ OBSERVACAO_SETTINGS carregado da flash: " + String(OBSERVACAO_SETTINGS));
    } else {
        save_flash_string(KEY_OBSERVACAO_SETTINGS, OBSERVACAO_SETTINGS);
        Serial.println("⚠️ OBSERVACAO_SETTINGS vazio, usando padrão: " + String(OBSERVACAO_SETTINGS));
    }

    // OBSERVACAO_READINGS
    char observacao_readings_tmp[64];
    read_flash_string(KEY_OBSERVACAO_READINGS, observacao_readings_tmp, sizeof(observacao_readings_tmp));
    if (strlen(observacao_readings_tmp) > 0) {
        strcpy(OBSERVACAO_READINGS, observacao_readings_tmp);
        Serial.println("✅ OBSERVACAO_READINGS carregado da flash: " + String(OBSERVACAO_READINGS));
    } else {
        save_flash_string(KEY_OBSERVACAO_READINGS, OBSERVACAO_READINGS);
        Serial.println("⚠️ OBSERVACAO_READINGS vazio, usando padrão: " + String(OBSERVACAO_READINGS));
    }

    // LINHA
    char linha_tmp[32];
    read_flash_string(KEY_LINHA, linha_tmp, sizeof(linha_tmp));
    if (strlen(linha_tmp) > 0) {
        strcpy(LINHA, linha_tmp);
        Serial.println("✅ LINHA carregado da flash: " + String(LINHA));
    } else {
        save_flash_string(KEY_LINHA, LINHA);
        Serial.println("⚠️ LINHA vazio, usando padrão: " + String(LINHA));
    }

    // PLACA_SOC
    char placa_soc_tmp[64];
    read_flash_string(KEY_PLACA_SOC, placa_soc_tmp, sizeof(placa_soc_tmp));
    if (strlen(placa_soc_tmp) > 0) {
        strcpy(PLACA_SOC, placa_soc_tmp);
        Serial.println("✅ PLACA_SOC carregado da flash: " + String(PLACA_SOC));
    } else {
        save_flash_string(KEY_PLACA_SOC, PLACA_SOC);
        Serial.println("⚠️ PLACA_SOC vazio, usando padrão: " + String(PLACA_SOC));
    }

    // FABRICANTE_SENSOR
    char fabricante_sensor_tmp[32];
    read_flash_string(KEY_FABRICANTE_SENSOR, fabricante_sensor_tmp, sizeof(fabricante_sensor_tmp));
    if (strlen(fabricante_sensor_tmp) > 0) {
        strcpy(FABRICANTE_SENSOR, fabricante_sensor_tmp);
        Serial.println("✅ FABRICANTE_SENSOR carregado da flash: " + String(FABRICANTE_SENSOR));
    } else {
        save_flash_string(KEY_FABRICANTE_SENSOR, FABRICANTE_SENSOR);
        Serial.println("⚠️ FABRICANTE_SENSOR vazio, usando padrão: " + String(FABRICANTE_SENSOR));
    }

    // MODELO_SENSOR
    char modelo_sensor_tmp[32];
    read_flash_string(KEY_MODELO_SENSOR, modelo_sensor_tmp, sizeof(modelo_sensor_tmp));
    if (strlen(modelo_sensor_tmp) > 0) {
        strcpy(MODELO_SENSOR, modelo_sensor_tmp);
        Serial.println("✅ MODELO_SENSOR carregado da flash: " + String(MODELO_SENSOR));
    } else {
        save_flash_string(KEY_MODELO_SENSOR, MODELO_SENSOR);
        Serial.println("⚠️ MODELO_SENSOR vazio, usando padrão: " + String(MODELO_SENSOR));
    }

    // VERSAO_HARDWARE
    char versao_hardware_tmp[32];
    read_flash_string(KEY_VERSAO_HARDWARE, versao_hardware_tmp, sizeof(versao_hardware_tmp));
    if (strlen(versao_hardware_tmp) > 0) {
        strcpy(VERSAO_HARDWARE, versao_hardware_tmp);
        Serial.println("✅ VERSAO_HARDWARE carregado da flash: " + String(VERSAO_HARDWARE));
    } else {
        save_flash_string(KEY_VERSAO_HARDWARE, VERSAO_HARDWARE);
        Serial.println("⚠️ VERSAO_HARDWARE vazio, usando padrão: " + String(VERSAO_HARDWARE));
    }

    // DATA_INSTALACAO
    char data_instalacao_tmp[32];
    read_flash_string(KEY_DATA_INSTALACAO, data_instalacao_tmp, sizeof(data_instalacao_tmp));
    if (strlen(data_instalacao_tmp) > 0) {
        strcpy(DATA_INSTALACAO, data_instalacao_tmp);
        Serial.println("✅ DATA_INSTALACAO carregado da flash: " + String(DATA_INSTALACAO));
    } else {
        save_flash_string(KEY_DATA_INSTALACAO, DATA_INSTALACAO);
        Serial.println("⚠️ DATA_INSTALACAO vazio, usando padrão: " + String(DATA_INSTALACAO));
    }
    
    Serial.println("📂 Carregamento de configurações concluído!");
}
/*
// Exemplo de uso:

save_flash_float("length_max", 123.45);
save_flash_string("ip", "192.168.0.100");
save_flash_string("wifi_ssid", "MinhaRede");
save_flash_string("wifi_pass", "Senha123");
save_flash_string("mqtt_server", "mqtt.example.com");
save_flash_int("mqtt_port", 1883);
save_flash_string("mqtt_user", "usuario");
save_flash_string("mqtt_pass", "senha");

float length_max = read_flash_float("length_max");
char ip[16], ssid[32], pass[32], mqtt_server[32], mqtt_user[32], mqtt_pass[32];
read_flash_string("ip", ip, 16);
read_flash_string("wifi_ssid", ssid, 32);
read_flash_string("wifi_pass", pass, 32);
read_flash_string("mqtt_server", mqtt_server, 32);
int mqtt_port = read_flash_int("mqtt_port");
read_flash_string("mqtt_user", mqtt_user, 32);
read_flash_string("mqtt_pass", mqtt_pass, 32);
*/