#ifndef DATAREPOSITORY_H
#define DATAREPOSITORY_H

#include "DAO_General.h"

extern BIO_SIM_DATA *GlobalData; 
extern int pause;
extern int simulation_day;
extern int elapsedTime;
extern double secondsPerDay;

void updateTime();

#endif
