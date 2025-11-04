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
void onTimer();
void onTimerSendMqtt();
void verifica_timer();
void check_timer_interrupt_tosend_MqttDataReadings();
void show_time();
void show_ip ();
void verifica_batida_prensa(void);
void setup_batidas_prensa();
void verifica_interrupcao();
void try_send_buffered_batidas();
void buffer_batida(const char* nome, const char* timeStr, long id, const char* obs);
void set_reservatorio();
extern char* get_time_str(char* buffer, size_t bufferSize);


extern long idBatida; // Apenas declaração, sem inicialização
extern int qtd_batidas_intervalo;


#endif // STATE_H_


