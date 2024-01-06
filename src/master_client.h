#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE

// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)
#define P_M_T_C "Pipe_Master_To_Client"
#define P_C_T_M "Pipe_Client_To_Master"
#include <pthread.h>
#define MON_FICHIER "master_client.h"
// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master

typedef struct
{
    int taille;
    bool *b;
    int nombre;
    pthread_mutex_t *mutex;
}Thread;

void multi_thread(int n);
void resultat_tab(bool *p , int taille );

void init_struct(Thread *pdata , int n, pthread_mutex_t *m , bool *p);
bool *init_tab_bool(int n);
void * codeThread(void * arg);
void remplir_tab(bool *p, int nombre , int taille);

void destruction();

void ecrire_tube(const char *name,int n);
int lecture_tube(const char *name);
void create_tubes(const char *name);
int create_sem(int key);
int create_sem_client(int key);
void prendre(int semid );
void vendre(int semid );
void detruire_sem_all(); //fork  avant
// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation


#endif
