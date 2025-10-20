#ifndef MEM_FLASH_H
#define MEM_FLASH_H

#include <Preferences.h>

void setup_mem_flash();

// Carrega todas as configurações da flash
void load_all_settings_from_flash();

//float
void save_flash_float(const char* key, float value);
float read_flash_float(const char* key);
//string
void save_flash_string(const char* key, const char* value);
void read_flash_string(const char* key, char* buffer, int maxLen);
//int
void save_flash_int(const char* key, int value);
int read_flash_int(const char* key);

// Funções específicas para configurações do dispositivo
void save_dispositivo_id(const char* value);
void save_fabricante_maquina(const char* value);
void save_modelo_maquina(const char* value);
void save_tipo_sensor(const char* value);
void save_observacao_device_info(const char* value);
void save_observacao_settings(const char* value);
void save_observacao_readings(const char* value);

void read_dispositivo_id(char* buffer, int maxLen);
void read_fabricante_maquina(char* buffer, int maxLen);
void read_modelo_maquina(char* buffer, int maxLen);
void read_tipo_sensor(char* buffer, int maxLen);
void read_observacao_device_info(char* buffer, int maxLen);
void read_observacao_settings(char* buffer, int maxLen);
void read_observacao_readings(char* buffer, int maxLen);


#endif // MEM_FLASH_H