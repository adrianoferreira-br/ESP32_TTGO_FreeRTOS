#ifndef MEM_FLASH_H
#define MEM_FLASH_H

#include <EEPROM.h>

void setup_mem_flash();
void save_flash_length_max(float length_max);
float read_flash_length_max();



#endif // MEM_FLASH_H