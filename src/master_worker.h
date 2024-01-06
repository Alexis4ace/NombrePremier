#ifndef MASTER_WORKER_H
#define MASTER_WORKER_H

#define MON_FICHIER "master_client.h"
// On peut mettre ici des éléments propres au couple master/worker :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (écriture dans un tube, ...)




typedef struct{
    int nombre;
    int tubeEntrant;
    int *tubeSortant;
    int master;
} ThreadData;


void action(ThreadData *pdata , int chiffre);

void first_worker(int lecture, int ecriture);

int lecture_ano(int lecture);
void ecrire_ano(int ecrire,int n);

void new_Worker(int premier , int lecture ,int ecriture);

#endif
