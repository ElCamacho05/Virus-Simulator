// Person.c

#include "Person.h"
#include "Regions.h"
#include "Virus.h"

#include <stdlib.h>
#include <string.h>

// PERSON functions
PERSON *createPerson(int id, char *name, int territorio_id, double initialDegree, double riesgo_inicial, int daysInfected) {
    PERSON *nP =(PERSON*) malloc(sizeof(PERSON));
    if (nP == NULL) return NULL;
    
    nP->id = id;
    strcpy(nP->name, name);
    
    // Asignación de IDs y campos corregidos
    nP->territorio_id = territorio_id; 
    nP->initialDegree = initialDegree;
    nP->riesgo_inicial = riesgo_inicial; // Usar el campo corregido
    nP->daysInfected = daysInfected;

    // Inicialización del estado de simulación (CLAVE para la Tarea 2 y 3)
    nP->estado = SANO;
    nP->cepa_actual_id = 0; 


    return nP;
};