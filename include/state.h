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



#endif // STATE_H_