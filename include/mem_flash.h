#ifndef MEM_FLASH_H
#define MEM_FLASH_H

#include <EEPROM.h>

void setup_mem_flash();


//flash
void save_flash_float(int addr, float value);
float read_flash_float(int addr);
//string
void save_flash_string(int addr, const char* value, int maxLen);
void read_flash_string(int addr, char* buffer, int maxLen);
//int
void save_flash_int(int addr, int value);
int read_flash_int(int addr);



void save_flash_length_max(float length_max);
float read_flash_length_max();


#endif // MEM_FLASH_H