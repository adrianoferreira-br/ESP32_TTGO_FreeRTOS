#include "mem_flash.h"


#define EEPROM_SIZE 4 // Tamanho necessário para armazenar um float (4 bytes)
#define EEPROM_ADDRESS 0 // Endereço inicial para armazenar o float


 
 
void setup_mem_flash() { 
 EEPROM.begin(EEPROM_SIZE); // Inicializa EEPROM

}


void save_flash_length_max(float length_max) {
      EEPROM.writeFloat(0, length_max);
      EEPROM.commit();    
}



float read_flash_length_max() {
      float length_max = EEPROM.readFloat(0);
      Serial.println("Altura máxima do reservatório (lido da EEPROM): " + String(length_max) + " cm");
      return length_max;
}
