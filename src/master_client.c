#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#define _XOPEN_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include "myassert.h"
#include <sys/sem.h>
#include <unistd.h>
#include <pthread.h>
#include "myassert.h"
#include <stdbool.h>
#include <math.h>
#include <sys/wait.h>
#include "master_client.h"

// BOTTIN alexis ROBERT hugo
// fonctions éventuelles internes au fichier

//**************************destruction***************************
void destruction(){

	int test = unlink(P_M_T_C);   											
	myassert( test ==0, "pipe unlink" );  //destruction des tubes nommes
	
	test = unlink(P_C_T_M);
	myassert( test == 0 , "pipe unlink" );
	
	pid_t res = fork(); 
	if(res == 0){
		detruire_sem_all(); // destruction semaphore
	}	
	wait(NULL);

}


void detruire_sem_all(){    // lance programme destruction sem
	
	char *s[2];
	s[0] = "rmsempipe.sh";
	s[1] = NULL;
	execv(s[0],s);
}
// ************************************** SEMAPHORE ******************************************
int create_sem(int cle){   // creation semaphore avec une cle
    
   	key_t key = ftok(MON_FICHIER, cle);
    myassert(key != -1,"  erreur generation de cle");

	int semId = semget(key, 1, IPC_CREAT | IPC_EXCL | 0641);
	myassert(semId != -1 , " erreur generation de sem dans create_sem");
	
	int ret = semctl(semId, 0, SETVAL, 0);  // on les inits a 0 
    myassert(ret != -1," error init semaphore");

	return semId;  
}

int create_sem_client(int cle){  // on recupere le semaphore cree par le client 
    
    key_t key;
    int semId;
	
    key = ftok(MON_FICHIER, cle);
    myassert(key != -1,"erreur de generation de la cle");
    
    semId = semget(key, 1, 0);
    myassert(semId != -1,"ereur de recuperation de semapore ");

    return semId;
}

void prendre(int semId)  // decremente le semaphore de 1 
{
    struct sembuf op = {0, -1, 0};
    int ret = semop(semId, &op, 1);
    myassert(ret != -1,"erreur decrementation du semapore");
}

void vendre(int semId) // incremente le semaphore de 1
{
    struct sembuf op = {0, +1, 0};
    int ret = semop(semId, &op, 1);
    myassert(ret != -1,"erreur incrementation du semaphore");
}


//****************************tubes*********************************
void ecrire_tube(const char *name,int n){  
	int nombres = n;
	int fd = open(name, O_WRONLY);  // ouverture 
	
	int test = write(fd,&nombres,sizeof(int));  // ecriture
    myassert(test != -1 ,"ecriture dans tube pas ok");
    myassert(test == sizeof(int),"entier pas bien ecrit");
    
    close(fd); // fermeture
}


int lecture_tube(const char *name){
	int final ;
	int fd = open(name, O_RDONLY); // ouverture
	
	int test = read(fd,&final,sizeof(int)); // ecriture
    myassert(test != -1 || test != 0 ,"lecture dans tube pas ok");
    myassert(test == sizeof(int),"entier pas bien ecrit");
    
    close(fd); // fermeture
	return final; // renvoie le chiffre du tube
}


void create_tubes(const char *name){  // creation tube nommé
	
	int ret = mkfifo(name,0644);
	myassert(ret == 0 ,"erreur creation thepipe dans creation_tubes");

}

//*********************************local compute***********************************
bool *init_tab_bool(int n){
	
	bool *tab_bool = malloc(sizeof(bool) * (n+1) )	;  // malloc  le free se situe dans la fonction multi_thread 
	
	for(int k = 2 ; k < (n+1) ; k++ ){  // initialise toute les case a true
		tab_bool[k] = true ;
	}
	
	tab_bool[0] = false ;  // passe 0 a false car non premier
	
	return tab_bool; // retourne l'adresse du tableau
}


void init_struct(Thread *pdata , int n, pthread_mutex_t *m , bool *p){ // init la structure 
	
	pdata->taille = n+1;  
	(pdata->b) = p;
	pdata->mutex = m;
}


void * codeThread(void * arg)
{
    myassert(arg != NULL," erreur lecture argument dans codethreads");  
    Thread * pdata = (Thread *) arg;
    
    pthread_mutex_lock( (pdata->mutex) );  // rentre en section critique
    
 	
 	remplir_tab(pdata->b , pdata->nombre , pdata->taille); // passe toutes les cases du tableau qui sont multiple du chiffre pdata->nombre
	pthread_mutex_unlock( (pdata->mutex) ); // sort de la section critique 
	   
    return NULL;
}


void remplir_tab(bool *p, int nombre , int taille ){ // passes les multiples a false 
	
	for(int k = 2 ; k < taille ; k++){
		
		if( k != nombre && k % nombre == 0 )
			p[k] = false ;
	}
}


void resultat_tab(bool *p , int taille ){ // pour toutes les cases a true afficher la valeur de sa position 
	
	printf(" nombre premier : \n");
	int espace = 1;
	
	for(int k = 1 ; k < taille ; k++){
		
		if( p[k]){
			printf(" %d  ",k);
			espace++;
		}
		
		if(espace % 6 == 0){
			printf(" \n");
			espace = 1;
		}
	}
	printf(" \n ");
}


void multi_thread(int n){ // lancement des threads

    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; // init mutex

    pthread_t tabId[n-1]; // nombre de threads
    Thread datas[n-1]; // nombre de data pour chaque thread
    
    int taille = n+1;
    n = (int) sqrtf ( (float) n );
    
	bool *p = init_tab_bool(n); // pointeur sur le tableau 

    for (int i = 0; i < n-1; i++)
		init_struct(&datas[i],n,&mutex,p); //init les structures
    
    

    for (int i = 0; i < n-1; i++)
    {
    	datas[i].nombre = i + 2;
    
        int ret = pthread_create(&(tabId[i]), NULL, codeThread, &(datas[i])); //lance les threads
        myassert(ret == 0,"erreur lancement threads");
    }

  
    for (int i = 0; i < n-1 ; i++)
    {
        int ret = pthread_join(tabId[i], NULL); //attend les threads
        myassert(ret == 0,"erreur d'attente des threads");
    }
	
	resultat_tab(p, taille); // affiche resultat
    
    int ret = pthread_mutex_destroy(&mutex);  // detruit mutex
    myassert(ret == 0,"error destruction mutex"); 

	free(p); // libere le tableau
	
}




// fonctions éventuelles proposées dans le .h

