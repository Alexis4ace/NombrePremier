#if defined HAVE_CONFIG_H
#include "config.h"
#endif


#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include "myassert.h"
#include <sys/sem.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include "myassert.h"

#include "master_worker.h"
// BOTTIN alexis ROBERT hugo
// fonctions éventuelles internes au fichier
//************************tubes******************
void ecrire_ano(int ecrire,int n){ 

	int nombres = n;
	int test = write(ecrire,&nombres,sizeof(int));
    myassert(test != -1 ,"ecriture dans tube pas ok");
    myassert(test == sizeof(int),"entier pas bien ecrit");

}


int lecture_ano(int lecture){

	int final ;
	int test = read(lecture,&final,sizeof(int));
    myassert(test != -1 || test != 0 ,"lecture dans tube pas ok");
    myassert(test == sizeof(int),"entier pas bien ecrit");
	return final;
}


//***************************creation worker******************************
void first_worker(int lecture, int ecriture){

	char entre[1000];
	sprintf(entre, "%d", lecture);
	char sortis[1000];
	sprintf(sortis, "%d", ecriture);
	
	char *argv[5];
	argv[0] = "worker";
	argv[1] = entre;
	argv[2] = sortis ;
	argv[3] = "2" ;
	argv[4] = NULL ;
	
	execv(argv[0],argv);  //  ./work tube_entrant_Master tube_sortant_master 2
}

void new_Worker(int premier , int lecture ,int ecriture){
	
	bool marche = true ;
	int chiffre = premier +1;
	while(marche){
		
		for(int i = 2 ; i < chiffre ; i++){ // trouve le prochain nombre premier
			
			if(chiffre % 2 == 0) // si divisible par 2 non premier
				break;
			
			if(chiffre % i == 0)  // si divisible par un chiffre autre que lui meme et sup a 2 alors non premier
				break;
			
			if(  i > (chiffre / 2 ) )
				marche =false;      // oui il est premier alors marche sera a false
		}
		
		if(marche==true)  // si on est sortis avec break alors on incremente chiffre avec un
			chiffre++;
	}  // sinon marche sera a false donc on sort du while
	
	char entre[1000];
	sprintf(entre, "%d", lecture);
	char sortis[1000];
	sprintf(sortis, "%d", ecriture);
	char numero[1000];
	sprintf(numero, "%d", chiffre);
	
	char *argv[5];
	argv[0] = "worker";
	argv[1] = entre;
	argv[2] = sortis ;
	argv[3] = numero;
	argv[4] = NULL;
	
	execv(argv[0],argv);  //./worker tube_entrant tube_sortant 14
}

// fonctions éventuelles proposées dans le .h
