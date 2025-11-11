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
extern char* get_time_str(char* buffer, size_t bufferSize);


extern long idBatida; // Apenas declaração, sem inicialização
extern int qtd_batidas_intervalo;
extern int message_error_code;

// ============================================================================
// VARIÁVEIS DE ACUMULAÇÃO DE BATIDAS (BUFFER REDESENHADO)
// ============================================================================
extern int accumulated_batidas;           // Contador acumulado de batidas durante desconexão
extern unsigned long disconnection_start_time; // Timestamp da primeira batida durante desconexão
extern bool is_accumulating;   


#endif // STATE_H_


