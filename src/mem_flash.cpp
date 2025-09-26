#include "mem_flash.h"
#include <Preferences.h>
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
    float value = prefs.getFloat(key, 0.0);
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
    String val = prefs.getString(key, "");
    prefs.end();
    strncpy(buffer, val.c_str(), maxLen);
    buffer[maxLen - 1] = '\0'; // Garante terminação
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
    int value = prefs.getInt(key, 0);
    prefs.end();
    return value;
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