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
void onTimer();
void verifica_timer();
void show_time();
void show_ip ();
void verifica_batida_prensa(void);
void setup_batidas_prensa();
void verifica_interrupcao();
void try_send_buffered_batidas();
void buffer_batida(const char* nome, const char* timeStr, long id, const char* obs);
void define_length_max();
extern char* get_time_str(char* buffer, size_t bufferSize);


extern long idBatida; // Apenas declaração, sem inicialização


#endif // STATE_H_


