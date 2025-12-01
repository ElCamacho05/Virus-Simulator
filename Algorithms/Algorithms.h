#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "../DAO_General.h"

unsigned int hashFunction(int key, int size);

//PUNTO 2 (brotes aleatorios)
void establish_initial_outbreak(BIO_SIM_DATA *data, int num_brotes, int cepa_id);

//PUNTO 3(Simulacion de los contagios diarios)
void run_daily_simulation(BIO_SIM_DATA *data, int dia_simulation);

#endif