#include "mem_flash.h"
#include <EEPROM.h>
#include "constants.h"



/*
 * Função de configuração da memória flash (EEPROM)
 */ 
void setup_mem_flash() { 
 EEPROM.begin(EEPROM_SIZE); // Inicializa EEPROM

}

/*
 * 
 */
void save_flash_float(int addr, float value) {
      EEPROM.writeFloat(ADDR_LENGTH_MAX, value);
      EEPROM.commit();    
}

/*
 * 
 */
float read_flash_float(int addr) {
      return EEPROM.readFloat(addr);              
}

void save_flash_string(int addr, const char* value, int maxLen) {
    for (int i = 0; i < maxLen; i++) {
        EEPROM.write(addr + i, value[i]);
        if (value[i] == '\0') break;
    }
    EEPROM.commit();
}

void read_flash_string(int addr, char* buffer, int maxLen) {
    for (int i = 0; i < maxLen; i++) {
        buffer[i] = EEPROM.read(addr + i);
        if (buffer[i] == '\0') break;
    }
    buffer[maxLen - 1] = '\0'; // Garante terminação
}

void save_flash_int(int addr, int value) {
    EEPROM.writeInt(addr, value);
    EEPROM.commit();
}

int read_flash_int(int addr) {
    return EEPROM.readInt(addr);
}






/*
// Gravar
save_flash_length_max(123.45);
save_flash_string(ADDR_IP, "192.168.0.100", 16);
save_flash_string(ADDR_WIFI_SSID, "MinhaRede", 32);
save_flash_string(ADDR_WIFI_PASS, "Senha123", 32);
save_flash_string(ADDR_MQTT_SERVER, "mqtt.example.com", 32);
save_flash_int(ADDR_MQTT_PORT, 1883);
save_flash_string(ADDR_MQTT_USER, "usuario", 32);
save_flash_string(ADDR_MQTT_PASS, "senha", 32);

// Ler
float length_max = read_flash_length_max();
char ip[16], ssid[32], pass[32], mqtt_server[32], mqtt_user[32], mqtt_pass[32];
read_flash_string(ADDR_IP, ip, 16);
read_flash_string(ADDR_WIFI_SSID, ssid, 32);
read_flash_string(ADDR_WIFI_PASS, pass, 32);
read_flash_string(ADDR_MQTT_SERVER, mqtt_server, 32);
int mqtt_port = read_flash_int(ADDR_MQTT_PORT);
read_flash_string(ADDR_MQTT_USER, mqtt_user, 32);
read_flash_string(ADDR_MQTT_PASS, mqtt_pass, 32);

*/