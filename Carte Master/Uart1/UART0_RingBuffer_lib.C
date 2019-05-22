//---------------------------------------------------------------------------------------
// ID_1.1    UART0_RingBuffer_lib.C -----------------------------------------------------
//---------------------------------------------------------------------------------------
// ID_1.2  Source originale des codes de buffer circulaire: 
//         Microcontroller Programming III	MP7-46 - 
//         Université d'Adelaide 
//
// ID_1.3 Adapté par F.JOLY - CPE Lyon
// ID_1.4 DATE: 08 Mars 2016 
// ID_1.5 Version 1.0 
// 
// ID_1.6 Objectifs ---------------------------------------------------------------------
// Gestion d'une UART en émission-réception par interruption et buffer circulaire
// 
// ID_1.7 Dépendances matérielles "cible" ----------------------------------------------- 
// Processeur cible: C8051F02x
// Périphérique UART0

// ID_1.8 Dépendances matérielles "extérieures" -----------------------------------------
// 

// ID_1.9 Dépendances de type communications/protocolaires ------------------------------
// La configuration de la communication UART est du type asynchrone - 8bits - 1 stop bit
// Pas de parité
//
// ID_1.10 Fonctionnement du code -------------------------------------------------------
// Le code contient:
// - les fonctions de configuration de l'UART0 et de son timer associé. 
// - Une fonction de création du buffer circulaire (initialisation de structures)
// - Les fonctions de remplissage du buffer de transmission et les fonctions de vidage 
//   du buffer de réception.
//  - La fonction d'interruption UART0 chargée d'émettre sur la liaison série le contenu 
//    du buffer de transmission et de stocker dans le buffer de réception les données 
//    reçues sur la liaison série.

// ID_1.11 Choix technologiques divers --------------------------------------------------
// Utilisation de L'UART0 et du Timer 1 comme source d'horloge de l'UART0.
// Pour fonctionner ce code a besoin des macros Define SYSCLK et BAUDRATE

// ID_1.12 Tests réalisés ---------------------------------------------------------------
// Validation sur plateforme 8051F020TB avec processeur 8051F020
// Vitesse de transmission: 115200 Baud
// Fréquence quartz: 22,1184 MHz
//
// ID_1.13 Chaine de compilation --------------------------------------------------------
// KEIL C51 6.03 / KEIL EVAL C51
//
// ID_1.14 Documentation de référence ---------------------------------------------------
// Datasheet 8051F020/1/2/3  Preliminary Rev. 1.4 12/03 
//
//ID_1.15 Commentaires sur les variables globales et les constantes ---------------------
// La taille des buffers de réception et de transmission est modifiable avec la 
// macro MAX_BUFLEN  

//*************************************************************************************************

#include <c8051f020.h>                    // SFR declarations
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <intrins.h>
#include "servomoteur.h"
#include "telemetre_ultrason.h"
#include "mesure_courant.h"
#include <math.h>
#include "servo_V.h"
#include "SPI_master.h"
#include "config.h"

#ifndef CFG_Globale
   #define CFG_Globale
   #include <CFG_Globale.h>
#endif


//*************************************************************************************************
// Paramètresd modifiables
//*************************************************************************************************
#define       MAX_BUFLEN 32 // Taille des buffers de données
xdata int stop=0;							// booleen pour l'arret d'urgence
xdata int epreuve=0;					// booleen de début ou fin d'épreuve

xdata char vitesse[3]="20";		// vitiesse initiale de notre robot

xdata int FLAG2 = 0;					// booleen de timer2 pour 100ms
xdata int FLAGTOURNE = 0;					// booleen de timer2 pour 100ms
xdata int counter_T2 =0;			// compteur pour que notre timer2 atteigne 100ms

xdata char angleMem[3]="000";	// angle du servomoteur mis en mémoire

xdata int courant_inst = 0;		// courant/energie/tension relevees et calculees pour la mesure de courant
xdata float energie_tot = 0;
xdata int tension1 = 0;

xdata int distance2 = 1000;		// distance associé à la detection d'obstacle
xdata int TestX = 0;					// distances associees à l'avance en detection d'obstacle
xdata int TestY = 0;
xdata int signeX = 0;					// booleens pour le signe des distance à parcourir suivant les axes
xdata int signeY = 0;
xdata int signe=2;

//*************************************************************************************************
// DEFINITION DES MACROS DE GESTION DE BUFFER CIRCULAIRE
//*************************************************************************************************

// Structure de gestion de buffer circulaire
	//rb_start: pointeur sur l'adresse de début du buffer 
	// rb_end: pointeur sur l'adresse de fin du buffer	
	// rb_in: pointeur sur la donnée à lire
	// rb_out: pointeur sur la case à écrire
		
#define RB_CREATE(rb, type) \
   struct { \
     type *rb_start; \	   
     type *rb_end; \	   
     type *rb_in; \
	   type *rb_out; \		
	  } rb

//Initialisation de la structure de pointeurs 
// rb: adresse de la structure
// start: adresse du premier élément du buffer 
// number: nombre d'éléments du buffer - 1	(le "-1" n'est à mon avis pas nécessaire)
#define RB_INIT(rb, start, number) \
         ( (rb)->rb_in = (rb)->rb_out= (rb)->rb_start= start, \
           (rb)->rb_end = &(rb)->rb_start[number] )

//Cette macro rend le buffer circulaire. Quand on atteint la fin, on retourne au début
#define RB_SLOT(rb, slot) \
         ( (slot)==(rb)->rb_end? (rb)->rb_start: (slot) )

// Test: Buffer vide? 
#define RB_EMPTY(rb) ( (rb)->rb_in==(rb)->rb_out )

// Test: Buffer plein?
#define RB_FULL(rb)  ( RB_SLOT(rb, (rb)->rb_in+1)==(rb)->rb_out )

// Incrementation du pointeur dur la case à écrire
#define RB_PUSHADVANCE(rb) ( (rb)->rb_in= RB_SLOT((rb), (rb)->rb_in+1) )

// Incrémentation du pointeur sur la case à lire
#define RB_POPADVANCE(rb)  ( (rb)->rb_out= RB_SLOT((rb), (rb)->rb_out+1) )

// Pointeur pour stocker une valeur dans le buffer
#define RB_PUSHSLOT(rb) ( (rb)->rb_in )

// pointeur pour lire une valeur dans le buffer
#define RB_POPSLOT(rb)  ( (rb)->rb_out )


//*************************************************************************************************


/* Transmission and stock Data Buffers */
static char  xdata outbuf[MAX_BUFLEN];     /* memory for transmission ring buffer #1 (TXD) */
static char  xdata inbuf [MAX_BUFLEN];     /* memory for stock ring buffer #2 (RXD) */
static  bit   TXactive = 0;             /* transmission status flag (off) */

/* define out (transmission)  and in (stock)  ring buffer control structures */
static RB_CREATE(out,unsigned char xdata);            /* static struct { ... } out; */
static RB_CREATE(in, unsigned char xdata);            /* static struct { ... } in; */

//**************************************************************************************************
//**************************************************************************************************

// **************************************************************************************************
// init_Serial_Buffer: Initialisation des structuresde gestion des buffers transmission et stock
// *************************************************************************************************
//**************************************************************************************************
void init_Serial_Buffer(void) {

    RB_INIT(&out, outbuf, MAX_BUFLEN-1);           /* set up TX ring buffer */
    RB_INIT(&in, inbuf,MAX_BUFLEN-1);             /* set up RX ring buffer */

}
// **************************************************************************************************
// SerOutchar: envoi d'un caractère dans le buffer de transmission de la liaison série
// *************************************************************************************************
unsigned char serOutchar(char c) {

  if(!RB_FULL(&out))  // si le buffer n'est pas plein, on place l'octet dans le buffer
  {                 
  	*RB_PUSHSLOT(&out) = c;               /* store data in the buffer */
  	RB_PUSHADVANCE(&out);                 /* adjust write position */

  	if(!TXactive) {
		TXactive = 1;                      /* indicate ongoing transmission */
 	  TI0 = 1;//   Placer le bit TI à 1 pour provoquer le déclenchement de l'interruption
  	}
	return 0;  // opération correctement réalisée 
  }
   else return 1; // opération échouée - Typiquement Buffer plein
	
	return 1;
}
// ************************************************************************************************
//  serInchar: 	lecture d'un caractère dans le buffer de réception de la liaison série
//  Fonction adaptée pour la réception de codes ASCII - La réception de la valeur binaire 0
//  n'est pas possible (conflit avec le code 0 retourné si il n'y a pas de caractère reçu)
// ************************************************************************************************
char serInchar(void) {
char c;

  if (!RB_EMPTY(&in))
  {                 /* wait for data */
		

  	c = *RB_POPSLOT(&in);                 /* get character off the buffer */
 	  RB_POPADVANCE(&in);                   /* adjust read position */
  	return c;
  }
  else return 0;
	
	return 0;
}
// ************************************************************************************************
//  serInchar_Bin: 	lecture d'un caractère dans le buffer de réception de la liaison série
//  Fonction adaptée pour la réception de codes Binaires - Cette fonction retourne un entier. 
//  L'octet de poids faible correspond au caractère reçu, l'octet de poids fort, s'il est nul indique 
//  qu'aucun caractère n'a été reçu.
//  
// ************************************************************************************************
unsigned int serInchar_Bin(void) {
char c;
unsigned int return_code = 0;
	 
  if (!RB_EMPTY(&in))
  {                
    // un caractère au moins est dans le buffer de réception
  	c = *RB_POPSLOT(&in);                 /* get character off the buffer */
 	  RB_POPADVANCE(&in);                   /* adjust read position */
  	return 0xFF00+c;
  }
	// pas de caractère dans le buffer de réception.
  else return return_code;
	
	return return_code;
}
// *************************************************************************************************
// serOutstring:  Envoi d'une chaine de caractère dans le buffer de transmission
// ************************************************************************************************
unsigned char serOutstring(char *buf) {
unsigned char len,code_err=0;

  for(len = 0; len < strlen(buf); len++)
     code_err +=serOutchar(buf[len]);
  return code_err;
}
//*************************************************************************************************
//  CONFIGURATION BAS NIVEAU de L'UART0
//*************************************************************************************************

//*****************************************************************************
#define Preload_Timer0 (SYSCLK/(BAUDRATE*16))
#if Preload_Timer0 > 255 
#error "Valeur Preload Timer0 HORS SPECIFICATIONS"
#endif 

//*****************************************************************************

		//FONCTION LECTURE/ECRITURE DE NOTRE UART1 (MASTER-SERIALIZER)

//*****************************************************************************
void putChar1(char carac){
	SBUF1=carac;
	while((SCON1|0xFD)==0xFD){
		}
	SCON1 &= 0xFD;
	}
char getChar1(){
	char reception=' ';
	while((SCON1|0xFE)==0xFE){
		}
	reception = SBUF1;
	SCON1 &= 0xFE;
	return reception;
	}
void putString1(char chaine[32]){
		xdata int i;
		for (i=0;i<strlen(chaine);i++){
			putChar1(chaine[i]);
		}
	}
	
void getString1(char* chaine){ 
	xdata int i = 0;
	for(i=0;i<5;i++){
		chaine[i]=getChar1();
	}
}
void getString2(char* chaine){ //fonction spéaciale pour la réception de la fonction : "getenc", car sa taille peut changer
	xdata int i = 0;
	for(i=0;i<7;i++){
		chaine[i]=getChar1();
	}
}
//*****************************************************************************

//	INTERRUPTIONS 

//*****************************************************************************


/* TIMER 2
		On veut plusieurs choses sur notre Timer 2:
						- gerer le calcule de distance entre notre objet et le projet obstacle, donc la detection d'obstacle
						- faire la mesure de courant
*/
void time2(void) interrupt 0x5{
	counter_T2++;			// compteur pour atteindre les 100ms par interval de detection d'objet
	if (counter_T2==2){
		RCAP2L = 0x00;					// pour obtenir 100 ms il faut 65536 + 65536 + 53248 (donc on change les valeurs de reload)
		RCAP2H = 0x30;
	}
	if (counter_T2==3){
		counter_T2 = 0;
		FLAG2 = 1;							/*On ne peut pas bloquer nos interruption, on se sert donc d'un flag pour faire
																	des opérations à l'exterieurs de notre interrupt*/
		RCAP2L = 0x00;					// pour obtenir 100 ms il faut 65536 + 65536 + 53248 (donc on change les valeurs de reload)
		RCAP2H = 0x00;
	}
	
	if (epreuve == 1){				// mesure de courant
			tension1 = conversion_ADC0(); //mV
			courant_inst = tension1*1000UL / (50UL*20UL);  //(Rshunt = 50mohm dou le facteur 1000) et gain de 20;
			energie_tot = energie_tot + (((float)courant_inst/1000UL)*((float)tension1/(1000*20)*0.1); //0.1s = base de temps
}
TF2=0;
}
/* UART0
		On veut plusieurs choses sur notre UART0:
						- transmission des messages de notre signal de commande à la carte MASTER via module XBEE
						- reception des message renvoyés par le robot via la carte MASTER et le module XBEE
*/
void UART0_ISR(void) interrupt 0x4 {
 
	
  if (TI0==1) // On peut envoyer une nouvelle donnée sur la liaison série
  { 
  	if(!RB_EMPTY(&out)) {
			if(*RB_POPSLOT(&out)=='Q'){
				putString1("stop\r");
			}
       SBUF0 = *RB_POPSLOT(&out);      /* start transmission of next byte */
       RB_POPADVANCE(&out);            /* remove the sent byte from buffer */
  	}
  	else TXactive = 0;                 /* TX finished, interface inactive */
	TI0 = 0;
  }
  else // RI0 à 1 - Donc une donnée a été reçue
  {
		if(!RB_FULL(&in)) {                   // si le buffer est plein, la donnée reçue est perdue
     	*RB_PUSHSLOT(&in) = SBUF0;        	/* store new data in the buffer */
		  RB_PUSHADVANCE(&in);               	/* next write location */
	 }
   RI0 = 0;
  }
}
//*****************************************************************************

//	FONCTION DE CONVERSION INT <=> ARRAY

//*****************************************************************************
void mon_itoa(int chiffre,char* distance_obs){ // basique
	xdata int i;
	xdata int a=100;
	for (i=0;i<3;i++){
		distance_obs[i]=0x30+floor(chiffre/a);
		chiffre = chiffre - (distance_obs[i]-0x30)*a;
		a = a/10;
	}
}
void mon_itoaObs(int chiffre,char* distance_obs){ // pour detection d'obstacle, cas special car on doit redimensioner distance_obs
		xdata int i;
		xdata int j;
		xdata int a=10;
		xdata int cpt=1;
			while(1){									// on vient mettre dans cpt la length de noptre chiffre (exp si 987 => cpt =3)
				if (chiffre/a>1){
					a=a*10;
					cpt+=1;
				}
				else{
					a=a/10;
					break;
				}
			}
		for(j=0;j<strlen(distance_obs)-cpt;j++){	// on met des 0 sur les premières cases mémoire
			distance_obs[j]='0';
		}
		for(i=strlen(distance_obs)-cpt;i<strlen(distance_obs);i++){
			distance_obs[i]=0x30+floor(chiffre/a);
			chiffre = chiffre - (distance_obs[i]-0x30)*a;
			a = a/10;
		}
	}
void mon_itoaD(int chiffre,char* distance_obs){ //si on veut changer une distance, cas spécial suivant positif ou negatif
		xdata int i=0;
		xdata int n=0;
		xdata int a=1000;
		if (chiffre<0){					// si la distance est négative(on "recul")
			chiffre=chiffre*(-1);	// on passe notre distance en positif 
			distance_obs[0]='-';	// on marque dans distance_obs qu'on va vers les axes negatifs
			i++;
		}
		n=i+3;
		for (i;i<=n;i++){
			distance_obs[i]=0x30+floor(chiffre/a);
			chiffre = chiffre - (distance_obs[i]-0x30)*a;
			a = a/10;
		}
	}
//*****************************************************************************
	
//	FONCTION COMMANDE EFFECTUEE
			// CETTE fonction permet que lorsque l'on veuille lancer une nouvelle commande aucune autre ne sois en cours.
					//Elle sert également à code l'arret d'urgence, la detection d'obstacle tous les 100ms.	
	
//*****************************************************************************	
void Commande_effectuee(){
	xdata char chaine[32]="";
	xdata int i = 0;
	xdata int test =0;
	while(test!=1){
		if (serInchar()=='Q'){
			putString1("stop\r");
			stop = 1;
			break;
		}
		putString1("pids\r");
		getString1(chaine);	
		for(i=0;i<strlen(chaine);i++){
			if (chaine[i]=='0'){
				test = 1;
			}
		}
		if ((FLAG2 == 1)&&(FLAGTOURNE==0)){
			distance2 = (int)(calc_dist());
			FLAG2 = 0;
			if ((distance2<30)&&(distance2>0)){
				putString1("stop\r");
				break;
			}
		}
	}
	delay(10);
}	

//*****************************************************************************

		// FONCTION AVANCE/TOURNE qui se sevent de la commande "mogo" ou "digo" 

//*****************************************************************************
void rotationG(int angle){
	xdata char dist[4];
	angle=angle*5.4;		// on à pris 5.4 de manière empirique, c'est le facteur de conversion entre notre angle en degrès et celui en increment qui lui équivaut
	mon_itoaD(angle,dist);
	putString1("digo 1:");
	putString1(dist);
	putString1(":10 2:-");
	putString1(dist);
	putString1(":10");
	putChar1('\r');
}

void rotationD(int angle){
	xdata char dist[4];
	angle=angle*5.4;		// on à pris 5.4 de manière empirique, c'est le facteur de conversion entre notre angle en degrès et celui en increment qui lui équivaut
	mon_itoaD(angle,dist);
	putString1("digo 1:-");
	putString1(dist);
	putString1(":10 2:");
	putString1(dist);
	putString1(":10");
	putChar1('\r');
}
void avanceX(char* distanceX){
		delay(20);
		FLAGTOURNE = 1;
		if (signeX==1){									// si on va vers les X négatifs
			rotationG(90);
		}
		else{														// si on va vers les X positifs
			rotationD(90);	
		}
		Commande_effectuee();						// on attend que la rotation soit finie avant d'avancer
		FLAGTOURNE = 0;
		if(stop==1){										// si on demande un arrêt d'urgence, on n'avance pas et on arrête tout							
		}
		else{							// pas d'arrêt d'urgence et rotation finie => on avance
			delay(20);
			putString1("digo 1:");
			putString1(distanceX);
			putString1(":20");
			putString1(" 2:");
			putString1(distanceX);
			putString1(":20");
			putChar1('\r');
		}
	}
void avanceY(char* distanceY){
	if(stop==1){											// si on demande un arrêt d'urgence, on n'avance pas et on arrête tout
	}
	else{
		FLAGTOURNE = 1;		
		delay(20);
		if (signeX==1){			
			if (signeY==1){								// si on va vers les X et Y négatifs
				rotationG(90);
			} 
			else{													// si on va vers les X négatif et Y positif
				rotationD(90);
			}
		}
		else{
			if (signeY==1){								// si on va vers les X positif et Y négatif
				rotationD(90);
			} 
			else{													// si on va vers les X positif et Y positif
				rotationG(90);
			}
		}
		Commande_effectuee();						// on attend que la rotation soit finie avant d'avancer		
		FLAGTOURNE = 0;
		if(stop==1){										// si on demande un arrêt d'urgence, on n'avance pas et on arrête tout
		}
		else{														// pas d'arrêt d'urgence et rotation finie => on avance
			delay(20);
			putString1("digo 1:");
			putString1(distanceY);
			putString1(":20");
			putString1(" 2:");
			putString1(distanceY);
			putString1(":20");
			putChar1('\r');
		}
	}
}
void avanceObstacle(char* distanceX){		// fonction d'avancé avec prise en compte des Obstacles
				xdata int i=0;
				xdata char chaine[15];
				xdata char inc[15];
				TestX=atoi(distanceX);						// TestX sert de variables qui prendra la valeurà parcourir
				putString1("getenc 1\r");					// On demande à la carte SERIALIZER de nous donnée le nombre d'increments realises
				getString2(chaine);
				for(i=3;i<strlen(chaine);i++){
					if (chaine[i]!='\r'){
						inc[i-3]=chaine[i];
					}
					else{
						break;
					}
						
				}																	// on vient mettre dans "inc", le nombre d'increments réalises
				if(atoi(inc)>=TestX){							/* si ce chiffre est superieur à la distance parcourue par le robot
																									(3790 au lieu de 3789, car il n'est pas 100% précis)
																						, on demande au robot de ne plus parcourir 1 seul cm, on met donc TestX à 0*/
					TestX=0;
				}
				else{
					TestX-=atoi(inc);								// si ce n'est pas le cas alors on soustrait à TestX la distance parcourue 								
				}
				mon_itoaObs(TestX,distanceX);			// On vient mettre à jour distanceX avec la nouvelle distance à parcourir
				putString1("clearenc 1 2\r");			// on vient clear le nombre d'incréments enregistré dans le robot
	}
	
void avance(char* vitesse){					// fonction avance banale avec les mogo
	putString1("mogo 1:");
	putString1(vitesse);
	putString1(" 2:");
	putString1(vitesse);
	putChar1('\r');
}

void recule(char* vitesse){					// fonction recule banale avec les mogo
	putString1("mogo 1:-");
	putString1(vitesse);
	putString1(" 2:-");
	putString1(vitesse);
	putChar1('\r');
}
//*****************************************************************************

		// FONCTION CONVERSION DISTANCE EN dm/INCREMENTS

//*****************************************************************************

void calcDistanceX(char* distanceX){
		xdata int dist1=0;
		dist1=atoi(distanceX);
		dist1=(1.67)*334*dist1/1.76;			/* les différents chiffres sont issue de la conversion donnée dans la datasheet
																				après on à ajuster de manière empirique pour avoir une meilleur précision	*/
		mon_itoaD(dist1,distanceX);	
	}
	
void calcDistanceY(char* distanceY){
		xdata int dist1=0;
		dist1=atoi(distanceY);
		dist1=(1.67)*334*dist1/1.76;			/* les différents chiffres sont issue de la conversion donnée dans la datasheet
																				après on à ajuster de manière empirique pour avoir une meilleur précision	*/
		mon_itoaD(dist1,distanceY);
	}


//*****************************************************************************

		// FONCTION QUI PERMET DE DISTINGUER LES DIFFERENTES COMMANDES ENVOYEES PAR L'UTILISATEUR

//*****************************************************************************

void transfert (char* stock) {
	xdata char vitesse2[3]="";				// si on précise une vitesse après la commande 'A' ou 'B'
	xdata char angle[3]="";						// angle demandé pour le servomoteur (besoin de garder en mémoire)
	xdata char angleRobot[3]="";			// angle demandé dans la commande 'G'
	xdata int angleRob=0;							// int associé au array precedent
	xdata char distanceX[6]="";				// array pour les distance à parcourir
	xdata char distanceY[6]="";
	xdata int i=0;										// variables de boucles "for"
	xdata int n=0;
	xdata int j = 0;
	xdata int machin = 0;							// int associé à l'array de l'angle servomoteur
	xdata int position_servo = 1;			// int de position du servomoteur
	xdata char distance_obs[3]="";		// array associé à l'entier du calcul de distance entre le robot et le prochain obstacle
	xdata int d=0;										// valeur de distance entre le robot et le prochain l'obstacle
	xdata char courant[4]="";					// array pour la mesure de courant
	serOutstring("\n\r");
	if(stock[0]=='D'){			// Début Epreuve
		serOutstring("******Epreuve 1******");
		epreuve=1;
		energie_tot = 0;
		}
	else if(stock[0]=='E'){						// Fin Epreuve
		epreuve=0;
		serOutstring("******Fin Epreuve******");
		}
	else{
		
		if (epreuve==1){
			if((stock[1]=='V')&&(stock[0]=='T')){								// TV vitesse
				vitesse[2]='\0';
				for (i=3;i<=strlen(stock);i++){
					vitesse[i-3]=stock[i];
					}
				if ((strlen(vitesse)==3)&&(atoi(vitesse)>100)){
					serOutchar('#');
					vitesse[0]='2';
					vitesse[1]='0';
					vitesse[2]='\0';
				}
				else{
					serOutchar('>');
					
			}
		}
			
		else if(((stock[0] == 'A')&&(stock[1] == 'S')&&(stock[2]== 'S'))||((stock[0] == 'S')&&(stock[1] == 'D'))||(stock[0] == 'L')||((stock[0] == 'P')&&(stock[1] == 'V'))){
				xdata char testSPI[32];
				sprintf(testSPI,"%s\r",stock);
				envoi_SPI(testSPI);
				
			}
			else if(stock[0]=='A'){			// Avancer	
				if (strlen(stock)==1){
					avance(vitesse);
				}
				else{
					for (i = 2;i<strlen(stock);i++){
							vitesse2[i-2]=stock[i];
						}
					avance(vitesse2);
					}
				
				}
			
			else if(stock[0]=='B'){							// Reculer										// Reculer
				if (strlen(stock)==1){
					recule(vitesse);
					}
				else{
					for (i = 2;i<strlen(stock);i++){
							vitesse2[i-2]=stock[i];		
					}
					recule(vitesse2);
					}
				
				}

			else if(stock[0]=='S'){												// STOP					// STOP
				putString1("stop\r");
				serOutstring("stop");
			}
			
			else if((stock[1]=='D')&&(stock[0]='R')){			// 
				rotationD(90);															//Rotation Droite 90°
				}																						//
			
			else if((stock[1]=='G')&&(stock[0]='R')){								// 
				rotationG(90);																				//Rotation Gauche 90°
				}																											//
			
			else if((stock[1]=='C')&&(stock[0]='R')){								// 
				if (stock[3]=='D'){																		//Rotation Complete 180°
					rotationD(180);																			//
					}
				else if(stock[3]=='G'){
					rotationG(180);
					}
				}
			else if((stock[1]=='A')&&(stock[0]='R')){								// Rotation 45°
				if ((stock[3]=='G')&&(stock[4]==':')&&(stock[5]=='4')&&(stock[6]=='5')){
					rotationG(45);
					}
				else if ((stock[3]=='D')&&(stock[4]==':')&&(stock[5]=='4')&&(stock[6]=='5')){
					rotationD(45);
					}
				}
			
				

			// Avancer jusqu'a un point
			else if(stock[0]=='G'){												// Commande G X: Y: A:
				
							/*
								On commence par mettre nos paramètres dans des variables
								*/
				
				j=0;
				//DistanceX
				while (stock[j]!=':'){
					j++;
				}
				j++;
				n=0;
				while (stock[j]!=' '){
					distanceX[n]=stock[j];
					j++;
					n++;
				}
				//DistanceY
				while (stock[j]!=':'){
					j++;
				}
				j++;
				n=0;
				while (stock[j]!=' '){
					distanceY[n]=stock[j];
					j++;
					n++;
				}
				//Angle
				while (stock[j]!=':'){
					j++;
				}
				j++;
				n=0;
				while (j<strlen(stock)){
					angleRobot[n]=stock[j];
					j++;
					n++;
				}
				// On vient ensuite convertir nos distance en increment
				calcDistanceX(distanceX);
				calcDistanceY(distanceY);
				if (distanceX[0]=='-'){
					signeX = 1;
					distanceX[0]='+';
				}
				if (distanceY[0]=='-'){
					signeY = 1;
					signe = 1;
					distanceY[0]='+';
				}
				putString1("clearenc 1 2\r");
				//on avance avec la detection d'obstacle
				while((atoi(distanceX)>=10)||(atoi(distanceY)>=10)){
					//AVANCE SELON X
					avanceX(distanceX);
					Commande_effectuee();								// on attend d'avoir finie l'avancee en X
					avanceObstacle(distanceX);					// check si il y a un obstacle ou non
					Commande_effectuee();								// on attend d'avoir finie l'avancee en X
					//AVANCE SELON Y
					avanceY(distanceY);
					Commande_effectuee();								// on attend d'avoir finie l'avancee en Y
					avanceObstacle(distanceY);					// check si il y a un obstacle ou non
					Commande_effectuee();								// on attend d'avoir finie l'avancee en Y
					if (signeY==1){											// on constate que à la rotation àprès obstacle, cette relation est toujours verifiée
						signeX = 1-signeX;
						signeY=0;
					}
					
				}
				if (signe==1){												// si on à tourné à 180° de notre position initiale dans les deplacements, on se remet à notre position initiale
					rotationG(180);
				}
				angleRob=atoi(angleRobot);						
			  Commande_effectuee();
				if(stop==1){													// si arret d'urgence
					stop=0;
				}
				else{
					if (angleRob>0){										// on tourne de notre angle final
						rotationG(angleRob);
					}
					else{
						angleRob=abs(angleRob);
						rotationD(angleRob);
					}
			serOutstring("\r\nB");									// on renvoit 'B' quand la commande est finie
			}
}	
			
			
			//servomoteur
			else if((stock[1]=='S')&&(stock[0]=='C')&&(stock[2]==' ')&&((stock[3]=='H')||(stock[3]=='V'))&&(stock[4]==' ')&&(stock[5]=='A')&&(stock[6]==':')){
				for (i=7;i<=strlen(stock);i++){
					angle[i-7]=stock[i];
				}
				if (stock[3]=='H'){
					strcpy(angleMem,angle);
				
					machin = atoi(angle);
					if (((machin)>= -90)&&(machin<=90)){
						position_servo *= machin;
						servo_pos(position_servo,stock[3]);
						serOutstring("AS H");
					}else{
						serOutchar('#');
					}
				} else if (stock[3] == 'V'){
					xdata char testSPI[32];
					sprintf(testSPI,"%s\r",stock);
					envoi_SPI(testSPI);
					serOutstring("AS V");
					/*
					machin = atoi(angle);
					if (((machin)>= -90)&&(machin<=90)){
						position_servo *= machin;
						chg_servo_pos_v(position_servo);
						
					}else{
						serOutchar('#');
					}
					*/
				}
			}
			
			else if ((stock[0] == 'M')&&(stock[1] == 'O') && (stock[2] == 'U')){
				d = (int)(calc_dist());
		
				mon_itoa(d,distance_obs);
				strcpy(angle,angleMem);
				if ((d<10) || (d>105)){	//Pas d'obstacles
					serOutstring(strcat(angle,": 0"));
				} else {	//Obstacle detecte
					serOutstring(strcat(strcat(angle,":"),distance_obs));
				}
		}
			else if ((stock[0] == 'M')&&(stock[1] == 'I')){
				mon_itoa(courant_inst,courant);
				serOutstring(courant);
			}
			else if ((stock[0] == 'M')&&(stock[1] == 'E')){
				mon_itoa((int)energie_tot,courant);
				serOutstring(courant);
			}
			else{
				serOutchar('#');
			}
			}
		else{
			serOutchar('#');
		}
	}
	serOutstring("\n\r");
	

}
	

//*****************************************************************************

		// FONCTION QUI PERMET AU CODE D'INSCRIRE CE QUE L'ON ECRIT SUR LE CLAVIER DANS
			// LES BUFFERS CIRCULAIRE. ELLE PERMET EGALEMENT DE RELEVER TOUTES LES 100 MS
			// LA DISTANCE ENTRE LE ROBOT ET LE PROCHAIN OBSTACLE

//*****************************************************************************
	

void ecoute (void){
	char c;
	int compteur = 0;
	char stock[32]="";	
	while (((c=serInchar())!='\r')){		// permet d'inscrire dans les buffer circulaire 
		if (c!=0){
			serOutchar(c);
			stock[compteur]=c;							// rempli la variable stock 
			compteur ++;
	  }
	if (FLAG2 == 1){										// FLAG2 = 1 donc on releve la distance entre le robot et le prochain obstacle
		distance2 = (int)(calc_dist());
		if ((distance2<30)&&(distance2>0)){
			putString1("stop\r");
		}
		FLAG2 = 0;
	}
	}
	transfert(stock);
}