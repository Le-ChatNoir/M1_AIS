                          /* population.c : la population d'anticorps */
#include <stdio.h>
#include <stdlib.h>       /* Pour qsort */
#include <unistd.h>       /* Pour sleep */

#include "params.h"
#include "anticorps.h"
#include "population.h"

/**********************************************************************/
/********* Une Population *********************************************/
/**********************************************************************/
/*
typedef struct { Ac* *individus;     // Tableau alloue dynamiquement d'Ac*
                 Ac* *clones;        // Pointeur vers la zone des clones
                 int nbIndividus;
                 int nbClones;
               }                             Population;
*/

/* Generation d'une Population */
void generePopulation(Population *population,
                      int nbIndividus,int nbClones)
{
 int i=0;
 int nbIndividus_Plus_nbClones; /* Par la suite, aura pour valeur:
                                   population->nbIndividus
                                             +
                                   population->nbClones
                                */
 if (nbIndividus<=0)
 {
  printf("**** Warning ! **** : nbIndividus <= 0 (generePopulation)\n");
  nbIndividus=0;
  exit(EXIT_FAILURE); /* Travailler avec une population vide ...? bof ! */
 }

 if (nbClones<=0)
 {
  printf("**** Warning ! **** : nbClones <= 0 (generePopulation)\n");
  nbClones=0;
 }

 if (nbClones>nbIndividus)
 {
  printf("**** Warning ! **** : nbClones > nbIndividus (generePopulation)\n");
  nbClones=nbIndividus;
 }

 population->nbIndividus=nbIndividus;
 population->nbClones=nbClones;

 nbIndividus_Plus_nbClones=population->nbIndividus+population->nbClones;

 if (nbIndividus_Plus_nbClones==0)
 {
   population->individus=NULL;
   population->clones=NULL;
 }
 else
 {
  /* Allocation pour les individus et pour les clones d'un seul coup */
  population->individus=(Ac**)malloc(nbIndividus_Plus_nbClones*sizeof(Ac*));
  if (population->individus==NULL) { printf("Erreur::generePopulation, "
                                            "probleme sur malloc Ac*\n");
                                     exit(EXIT_FAILURE);
  }
  /* Et on positionne le pointeur population->clones ou il faut */
  population->clones=population->individus+population->nbIndividus;
 }

 for(i=0;i<population->nbIndividus;i++)
 {
  Ac *ac = (Ac*)malloc(sizeof(Ac));
  if (ac==NULL) { printf("Erreur::generePopulation,"
                         "probleme sur malloc Ac-1\n");
                  exit(EXIT_FAILURE);
  }
  population->individus[i]=ac;
 }

 for(i=0;i<population->nbIndividus;i++)
 {
  genereAc(population->individus[i]);
 }

 for(i=0;i<population->nbClones;i++)
 {
  Ac *ac = (Ac*)malloc(sizeof(Ac));
  if (ac==NULL) { printf("Erreur::generePopulation,"
                         "probleme sur malloc Ac-2\n");
                  exit(EXIT_FAILURE);
  }
  population->clones[i]=ac;
 }

 /* Pas utile... sauf si la suite est mal utilisee */
 for(i=0;i<population->nbClones;i++)
 {
  genereAc(population->clones[i]);
 }
}

/* Destruction d'une population */
void supprimePopulation(Population *population)
{
 int i=0;

 if (population->individus) 
 {
  for(i=0;i<population->nbIndividus;i++)
  {
   free(population->individus[i]);
  }
 }

 if (population->clones) 
 {
  for(i=0;i<population->nbClones;i++)
  {
   free(population->clones[i]);
  }
 }

 if (population->individus) free(population->individus);
}

/*-------------*/
/* Pour qsort,                                                */
/* comparaison du cout de deux Anti-Corps (via les pointeurs) */
/* Avec qsort, tri avec un cout CROISSANT                     */
static int compareAcPtr(const void *ac1,const void *ac2)
{
 Ac **ac1P=(Ac**)ac1;
 Ac **ac2P=(Ac**)ac2;

 return compareAc(*ac1P,*ac2P);
}
/*---*/
/* Pour qsort,                                                */
/* comparaison du cout de deux Anti-Corps (via les pointeurs) */
/* Avec qsort, tri avec un cout DECROISSANT                   */
static int compareInvAcPtr(const void *ac1,const void *ac2)
{
 return - compareAcPtr(ac1,ac2);
}

/*-------------*/
/* Tri d'une Population et calcul du meilleur avec            */
/* un COUT DECROISSANT =>                                     */
/* les moins bons sont a gauche, les meilleurs a droite       */
static void triSousPopulation(Population *population,
                              int indice, int nbIndividusSousPopulation)
{
 if (nbIndividusSousPopulation>0)
 {
  qsort(population->individus+indice,nbIndividusSousPopulation,sizeof(Ac*),
        compareInvAcPtr);
 }
}
/*---*/
void triPopulation(Population *population)
{
 triSousPopulation(population,0,population->nbIndividus);
}

/*** Il faut un tri avant ***/
/* Recuperation du meilleur individu */
Ac  meilleurIndividu(Population *population)
{
 return *(population->individus[population->nbIndividus-1]);
}

/*** Il faut un tri avant (les meilleurs doivent etre a droite).     ***/
void clonageMeilleurs(Population *population)
{
 int i = 0;
 int nbClones=population->nbClones;

 for(i=0;i<nbClones;i++)
 {
  int indiceAc;
  int indiceClone;

  indiceAc=population->nbIndividus-nbClones+i;      /*  meilleur  */
  indiceClone=i;                                    /*   clone    */

  cloneAc(population->individus[indiceAc],
          population->clones[indiceClone]);
 }
}

/*** Il faut un clonage avant ....                                   ***/
void mutationClones(Population *population)
{
 /* Plus un clone est a droite, meilleur il est.                   */
 /* On peut ainsi faire varier le nombre de mutations a effectuer. */
 /* Dans un premier temps, le nombre de mutations peut etre fixe.  */

 int nbClones = population->nbClones;
 //On mute le plus mauvais clone 4x, puis le plus mauvais 1/3 3 fois, le 1/3 suivant 2x, et le meilleur 1/3 1x
 int mutaFactor = 4;
 int valChangement = nbClones/3;
 int i;
 
 for(i = 0; i < nbClones -1 ; i++){
 	muteAc(population->clones[i], mutaFactor);
 	if (i % valChangement == 0) {
 		mutaFactor--;
 	}
 }

}

/*** Les meilleurs et les clones sont comparares                     ***/
/*** Chaque couple (meilleur,clone) est evalue...                    ***/
/*** Et on garde le meilleur des deux                                ***/
void selectionMeilleursEtClonesMutes(Population *population)
{
 
 /* Les declarations de variables a mettre ici ! */
 //==========COMPARAISON 2 A 2 ==================//
 int nbClones = population->nbClones;
 int j;
 int indiceMeilleurs = population->nbIndividus -1;
 Ac* tmp;
 
 for(j = nbClones -1 ; j > -1; j--){
 	//Le clone mute est meilleur que l'anticorps de base
 	if (population->individus[indiceMeilleurs]->cout > population->clones[j]->cout) {
 		tmp = population->clones[j];
 		population->clones[j] = population->individus[indiceMeilleurs];
 		population->individus[indiceMeilleurs] = tmp;
 	}
 	//Pour synchro la place des meilleurs et des clones
 	indiceMeilleurs--;
 } 

}

/*** Les moins bons doivent etre a gauche (apres un tri par exemple) ***/
void remplacementMoinsBons(Population *population, int nbNouveaux)
{
 
 /* Les declarations de variables a mettre ici ! */
 //Regen nbNouveaux mauvais individus avec genereAc
 int i;
 
 for(i = 0; i< nbNouveaux; i++){
 	genereAc(population->individus[i]);
 }
 
 triPopulation(population);
 
}

/*** Les moins bons doivent etre a gauche (apres un tri par exemple) ***/
void mutationMoinsBons(Population *population)
{
 /* Plus un mauvais est a droite, meilleur il est!                 */
 /* On peut ainsi faire varier le nombre de mutations a effectuer. */
 /* Dans un premier temps, le nombre de mutations peut etre fixe.  */

 /* Les declarations de variables a mettre ici ! */

//On mute la population non retenue par un clone
 int nbIndividus = population->nbIndividus - population->nbClones;
 int mutaFactor = 4;
 int valChangement = nbIndividus/3;
 int i;
 
 for(i = 0; i < nbIndividus -1 ; i++){
 	muteAc(population->individus[i], mutaFactor);
 	if (i % valChangement == 0) {
 		mutaFactor--;
 	}
 }

}

/* Affichage des meilleurs individus de la Population  */
/* Pour afficher toute la population =>  nbIndividus=0 */
void printIndividusPopulation(Population *population, int nbIndividus)
{
 int i=0, borne=0;

 if (nbIndividus==0) borne=0;
 else borne=population->nbIndividus-nbIndividus;
 borne = (borne<0) ? 0: borne;

 if (borne==0) printf("Affichage de la population entiere\n");
 else printf("Affichage des %d meilleurs individus de la population\n",
              population->nbIndividus-borne);

 for(i=borne;i<population->nbIndividus;i++)
 {
  printAc(population->individus[i]);
 }
}

/* Dessin des meilleurs individus de la Population     */
/* Pour dessiner toute la population =>  nbIndividus=0 */
void dessineIndividusPopulation(FILE* flot,
                                Population *population, int nbIndividus)
{
 int i=0, borne=0;

 if (nbIndividus==0) borne=0;
 else borne=population->nbIndividus-nbIndividus;
 borne = (borne<0) ? 0: borne;

 if (borne==0) printf("Dessin de la population entiere\n");
 else printf("Dessin des %d meilleurs individus de la population\n",
              population->nbIndividus-borne);

 for(i=borne;i<population->nbIndividus;i++)
 {
  dessineAc(flot,population->individus[i]);
  sleep(1);
 }
}
