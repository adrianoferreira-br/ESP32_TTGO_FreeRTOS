/*

*/
#ifndef STATE_H_
#define STATE_H_

#include "Arduino.h"

void loop_state(void);
void init_state(void);
void InterruptionPino35(void);
void InterruptionPino38(void);
void InterruptionPino12(void);
void setup_timer();
void setup_timer_send_takt_time();
void reconfigure_batch_timer(int new_interval);
void onTimer();
void onTimerSendMqtt();
void verifica_timer();
void check_timer_interrupt_tosend_MqttDataReadings();
void show_time();
void show_ip ();
void verifica_batida_prensa(void);
void setup_batidas_prensa();
void verifica_interrupcao();
void set_reservatorio();
void verifica_batida_prensa_sensor2();
void InterruptionPino13();
extern char* get_time_str(char* buffer, size_t bufferSize);
extern String get_formatted_time();


extern long idBatida;
extern int qtd_batidas_intervalo;
extern long idBatida_sensor2;
extern int qtd_batidas_intervalo_sensor2;
extern int message_error_code;

// ============================================================================
// VARIÁVEIS DE ACUMULAÇÃO DE BATIDAS - SENSOR 1
// ============================================================================
extern int accumulated_batidas;
extern unsigned long disconnection_start_time;
extern bool is_accumulating;

// ============================================================================
// VARIÁVEIS DE ACUMULAÇÃO DE BATIDAS - SENSOR 2
// ============================================================================
extern int accumulated_batidas_sensor2;
extern unsigned long disconnection_start_time_sensor2;
extern bool is_accumulating_sensor2;   


#endif // STATE_H_


