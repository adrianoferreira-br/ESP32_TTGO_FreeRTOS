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


#endif // MEM_FLASH_H