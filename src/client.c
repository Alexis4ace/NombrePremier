#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include "myassert.h"

#include "master_client.h"

// chaines possibles pour le premier paramètre de la ligne de commande
#define TK_STOP      "stop"
#define TK_COMPUTE   "compute"
#define TK_HOW_MANY  "howmany"
#define TK_HIGHEST   "highest"
#define TK_LOCAL     "local"

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/
// BOTTIN alexis ROBERT hugo
static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <ordre> [<nombre>]\n", exeName);
    fprintf(stderr, "   ordre \"" TK_STOP  "\" : arrêt master\n");
    fprintf(stderr, "   ordre \"" TK_COMPUTE  "\" : calcul de nombre premier\n");
    fprintf(stderr, "                       <nombre> doit être fourni\n");
    fprintf(stderr, "   ordre \"" TK_HOW_MANY "\" : combien de nombres premiers calculés\n");
    fprintf(stderr, "   ordre \"" TK_HIGHEST "\" : quel est le plus grand nombre premier calculé\n");
    fprintf(stderr, "   ordre \"" TK_LOCAL  "\" : calcul de nombres premiers en local\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static int parseArgs(int argc, char * argv[], int *number)
{
    int order = ORDER_NONE;

    if ((argc != 2) && (argc != 3))
        usage(argv[0], "Nombre d'arguments incorrect");

    if (strcmp(argv[1], TK_STOP) == 0)
        order = ORDER_STOP;
    else if (strcmp(argv[1], TK_COMPUTE) == 0)
        order = ORDER_COMPUTE_PRIME;
    else if (strcmp(argv[1], TK_HOW_MANY) == 0)
        order = ORDER_HOW_MANY_PRIME;
    else if (strcmp(argv[1], TK_HIGHEST) == 0)
        order = ORDER_HIGHEST_PRIME;
    else if (strcmp(argv[1], TK_LOCAL) == 0)
        order = ORDER_COMPUTE_PRIME_LOCAL;
    
    if (order == ORDER_NONE)
        usage(argv[0], "ordre incorrect");
    if ((order == ORDER_STOP) && (argc != 2))
        usage(argv[0], TK_STOP" : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME) && (argc != 3))
        usage(argv[0], TK_COMPUTE " : il faut le second argument");
    if ((order == ORDER_HOW_MANY_PRIME) && (argc != 2))
        usage(argv[0], TK_HOW_MANY" : il ne faut pas de second argument");
    if ((order == ORDER_HIGHEST_PRIME) && (argc != 2))
        usage(argv[0], TK_HIGHEST " : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME_LOCAL) && (argc != 3))
        usage(argv[0], TK_LOCAL " : il faut le second argument");
    if ((order == ORDER_COMPUTE_PRIME) || (order == ORDER_COMPUTE_PRIME_LOCAL))
    {
        *number = strtol(argv[2], NULL, 10);
        if (*number < 2)
             usage(argv[0], "le nombre doit être >= 2");
    }       
    
    return order;
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{	
    int mes,number;
    
    int client = create_sem_client(1);
													//           . le mutex est déjà créé par le master
    int order = parseArgs(argc, argv, &number);  // order peut valoir 5 valeurs (cf. master_client.h) :
    printf("%d\n", order); 						// pour éviter le warning
    
    
    
    
    
    prendre(client);
    
    										//    - entrer en section critique :
    ecrire_tube(P_C_T_M,order); 						//    - ouvrir les tubes nommés (ils sont déjà créés par le master) j'ouvre et je ferme à chaque lecture ou ecriture voir master_client.c
    										
	
    if(order == ORDER_STOP){ 															//      - ORDER_STOP
    	
    																			
    	mes = lecture_tube(P_M_T_C);														//    - attendre la réponse sur le second tube
    																			//             le master ouvre les tubes dans le même ordre
    	if(mes == -1)
    		printf("STOP \n");
    }
    
    else if( order == ORDER_HOW_MANY_PRIME  || order == ORDER_HIGHEST_PRIME){			//      - ORDER_HIGHEST_PRIME					//      - ORDER_HOW_MANY_PRIME
    	
    	
    	mes = lecture_tube(P_M_T_C);                                                   //    - attendre la réponse sur le second tube
    	
    	
    	if( order == ORDER_HIGHEST_PRIME)
    		printf(" HIGHEST  : %d (if nothing then return 0)\n",mes);
    	else
    		printf(" HOW MANY : %d \n",mes);
    }
    
    else if( order == ORDER_COMPUTE_PRIME){												 //      - ORDER_COMPUTE_PRIME

    	
		ecrire_tube(P_C_T_M, number );
		
		
		
    	mes = lecture_tube(P_M_T_C);                                    //    - attendre la réponse sur le second tube
    	
    	
    	if( number == mes)
    		printf("IT'S PRIME %d  \n", number);
    	else
    		printf("IT'S NOT PRIME %d  \n", number);
    }
    else{   																	//      - ORDER_COMPUTE_PRIME_LOCAL
																				// si c'est ORDER_COMPUTE_PRIME_LOCAL	
		int n = number ;
		
    	multi_thread(n);  																//    alors c'est un code complètement à part multi-thread
    	
    }
	//    - sortir de la section critique
  
    //    - libérer les ressources (fermeture des tubes, ...)    mes tubes se ferme tout seul à chaque fois rien et dans multi_thread ( 5 ligne plus haut ) je fais un malloc et un free à la fin
   	//    - débloquer le master grâce à un second sémaphore (cf. ci-dessous)
    return EXIT_SUCCESS;
}
