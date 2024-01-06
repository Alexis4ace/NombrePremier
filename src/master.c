#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include "myassert.h"
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>

#include "master_client.h"
#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/
// BOTTIN alexis ROBERT hugo
// on peut ici définir une structure stockant tout ce dont le master
// a besoin
static int howMany = 0 ;
static int bigNumber = 0 ;

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(int client, int ecrire , int lire){
	
	bool marche = true;
    
    while(marche){  // boucle infinie :
    	int premier;
    	int ordre;
    	int mes ;	
    	
    	// - attente d'un ordre du client (via le tube nommé)
		vendre(client);	
 		
    	ordre = lecture_tube(P_C_T_M);      	
    	
    	printf("\n\n UN CLIENT ARRIVE %d \n", ordre);
   		
   		
   		if(ordre == ORDER_STOP)
   		{ 	
   			ecrire_ano(ecrire,ordre); 
   			ordre = lecture_ano(lire);
   			myassert(ordre == -1 , " stop eror in loop");							   // - si ORDER_STOP	
    		
    		ecrire_tube(P_M_T_C,ordre); 			// transmettre la réponse au client
    								//       . envoyer ordre de fin au premier worker et attendre sa fin
  		    marche = false ;		//       . envoyer un accusé de réception au client
    	}
    		
    	if(ordre == ORDER_COMPUTE_PRIME)
    	{  									// - si ORDER_COMPUTE_PRIME	
    						
			mes = lecture_tube(P_C_T_M);  	//       . récupérer le nombre N à tester provenant du client
									
			premier = mes;					//       . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
				
											//             il faut connaître le plus nombre (M) déjà enovoyé aux workers
			ecrire_ano(ecrire,mes);	  		//             on leur envoie tous les nombres entre M+1 et N-1
			mes = lecture_ano(lire);  
			
			if( mes == premier)
			{
				howMany++;	
				if( bigNumber < mes )
					bigNumber = mes ;
			}
			
											//        note : chaque envoie déclenche une réponse des workers
			ecrire_tube(P_M_T_C,mes);		//       . envoyer N dans le pipeline
											//       . récupérer la réponse
											//       . la transmettre au client
    	}
   		if(ordre == ORDER_HOW_MANY_PRIME){  // -si ORDER_HOW_MANY_PRIME
   				
   				
    			ecrire_tube(P_M_T_C,howMany);   //       . transmettre la réponse au client
    			
    	}
   
  
    	if(ordre == ORDER_HIGHEST_PRIME){  // - si ORDER_HIGHEST_PRIME
    		
    		
    		ecrire_tube(P_M_T_C,bigNumber); 			// transmettre la réponse au client
    		
    	}
    
    
    // - attendre ordre du client avant de continuer (sémaphore : précédence)
    // - revenir en début de boucle
    
    
    }
}
/************************************************************************
 * Fonction principale
 ************************************************************************/
int main(int argc, char * argv[])
{
    if (argc != 1){
        usage(argv[0], NULL);
	}

	wait(NULL);
	int test;
	
	// - création des sémaphores
	int semid = create_sem(1);
	
	
	// - création des tubes nommés
	create_tubes(P_M_T_C);
	create_tubes(P_C_T_M);
	
	// - création du premier worker
	int tube_M_W[2],tube_W_M[2];
	
	test = pipe(tube_M_W);
    myassert(test == 0,"error tube worker");
    test = pipe(tube_W_M);
    myassert(test == 0,"error tube maser");
    
    pid_t work = fork();
    
    if(work == 0){
    
    	close(tube_M_W[1]);
    	close(tube_W_M[0]);
		first_worker(tube_M_W[0],tube_W_M[1]);  // 0 lire  1 ecrire
		
	}
	
	else{

	close(tube_M_W[0]);
	close(tube_W_M[1]);

    printf("Master pret \n");
	loop(semid,tube_M_W[1],tube_W_M[0]); 	// boucle infinie
	
	close(tube_M_W[1]);
	close(tube_W_M[0]); // - fermer les tubes nommés
	
	destruction(); //destrction semaphore tube nommé
	printf("FIN  \n");
    return EXIT_SUCCESS;
    }
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
