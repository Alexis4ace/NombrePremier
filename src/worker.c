#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

 #include <sys/types.h>
 #include <unistd.h>


#include "myassert.h"

#include "master_worker.h"

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <n> <fdIn> <fdToMaster>\n", exeName);
    fprintf(stderr, "   <n> : nombre premier géré par le worker\n");
    fprintf(stderr, "   <fdIn> : canal d'entrée pour tester un nombre\n");
    fprintf(stderr, "   <fdToMaster> : canal de sortie pour indiquer si un nombre est premier ou non\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static void parseArgs(int argc, char * argv[] , ThreadData *pdata)
{
    if (argc != 4)
        usage(argv[0], "Nombre d'arguments incorrect");
	else {
  // 0 lecteur // 1 ecrire  ici //1
		pdata->nombre = atoi(argv[3]);
		pdata->tubeEntrant = atoi(argv[1]);
		pdata->master = atoi(argv[2]);
		pdata->tubeSortant = NULL ;
	}
    // remplir la structure
}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(ThreadData *pdata)
{
    // boucle infinie :
    bool marche = true ; //condition d'arret du worker
    
    int mes,res,test;  // mes : chiffre recu dans le tube    res : pour le fork   test : pour les retours de fonctions avec assert 
    
    int tube[2];   // nouveau tube si jamais faut créer un worker ( oui le dernier worker aura un tube qu'il n'utilisera pas ) 
    test = pipe(tube);
    myassert( test == 0 , " error tube dans loop dans worker" );
    
    int ecrire = tube[1];   
    
    while ( marche ){  
    	mes = lecture_ano(pdata->tubeEntrant); 						//    attendre l'arrivée d'un nombre à tester
    	
    	if( mes == -1	){     										//    si ordre d'arrêt
    		printf(" worker %d a recu la destruction \n" , pdata->nombre);
    		
    		if(pdata->tubeSortant != NULL) 							// si il y a un worker suivant, transmettre l'ordre et attendre sa fin
    			ecrire_ano(*(pdata->tubeSortant),mes);
    		else
    			ecrire_ano(pdata->master,mes);
    		marche = false; 										//       sortir de la boucle
    	}
    	
    	//    sinon c'est un nombre à tester, 4 possibilités :
    	
    	if( mes % pdata->nombre != 0  && marche ){
    	
    		if(pdata->tubeSortant != NULL){  											//           - s'il y a un worker suivant lui transmettre le nombre
    			
    			printf("J'ecris dans le tube existant %d ( je suis le worker %d ) \n",mes , pdata->nombre );   //           - le nombre n'est pas premier
    			ecrire_ano(*(pdata->tubeSortant),mes);
    		}
    		
    		if( pdata->tubeSortant == NULL){   			//           - le nombre n'est pas premier		
				
				printf(" Je suis le workeur %d et je creer un nouveau \n",pdata->nombre);
				(pdata->tubeSortant) = &ecrire ; 
				res = fork();
				
				if( res == 0){    
					
					close(tube[1]);
					new_Worker(pdata->nombre,tube[0],pdata->master);     //           - s'il n'y a pas de worker suivant, le créer
				}
				else{
					close(tube[0]);
					ecrire_ano(tube[1],mes);                              // lui transmet le message 
				}
			}
		}
    	if(mes % pdata->nombre == 0){  							 				//           - le nombre est premier
    		printf("Le workeur %d a trouver la reponse %d \n",pdata->nombre , mes );
    		ecrire_ano(pdata->master,pdata->nombre);
    	}
    	
    
    }
    close(tube[1]);// libérer les ressources : fermeture des files descriptors par exemple
   
}
/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
    
    ThreadData pdata; // structure 
  
  
    parseArgs(argc, argv ,&pdata);  // ./work  tube_entrant tube_sortant 2
	printf(" \n  WORKER DU NUMERO : %d        ", atoi(argv[3]) );
    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier
    loop(&pdata);

    return EXIT_SUCCESS;
}
