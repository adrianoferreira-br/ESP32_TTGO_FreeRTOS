/*
 *
 */
#ifndef EXTERN_DATA_H_
#define EXTERN_DATA_H_



// ============ VARIÁVEIS GLOBAIS ============
extern float cpu_temperature;  // Temperatura interna do CPU em °C

// ============ DECLARAÇÕES DE FUNÇÕES ============
void firebase_setup(void);
void firebase_updateValues(void);
void send_data_firestore(void);
void update_cpu_temperature(void);

#endif   //EXTERN_DATA_H_