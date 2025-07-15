/*

*/
#ifndef STATE_H_
#define STATE_H_



void loop_state(void);
void init_state(void);
void InterruptionPino35(void);
void InterruptionPino38(void);
void InterruptionPino12(void);
void sensor_corrente(void);
void show_time();
void show_ip ();
void calcula_corrente();
void calcula_tensao();
void calcula_fluxo();
void pulseCounter();
void verifica_batida_prensa(void);
void setup_batidas_prensa();
void verifica_interrupcao();
void try_send_buffered_batidas();
void buffer_batida(const char* nome, const char* timeStr, long id, const char* obs);



#endif // STATE_H_