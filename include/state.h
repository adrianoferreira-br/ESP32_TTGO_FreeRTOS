/*

*/
#ifndef STATE_H_
#define STATE_H_



void loop_state(void);
void init_state(void);
void InterruptionPino35(void);
void InterruptionPino38(void);
void InterruptionPino12(void);
void show_time();
void show_ip ();
void verifica_batida_prensa(void);
void setup_batidas_prensa();
void verifica_interrupcao();
void try_send_buffered_batidas();
void buffer_batida(const char* nome, const char* timeStr, long id, const char* obs);
void define_length_max();



#endif // STATE_H_


