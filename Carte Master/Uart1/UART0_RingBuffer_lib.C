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
#include <math.h>


#ifndef CFG_Globale
   #define CFG_Globale
   #include <CFG_Globale.h>
#endif


//*************************************************************************************************
// Paramètresd modifiables
//*************************************************************************************************
#define       MAX_BUFLEN 32 // Taille des buffers de données
xdata char vitesse[3]="20";
xdata int epreuve=0;
xdata char angleMem[3]="000";

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
void UART0_ISR(void) interrupt 0x4 {
 
//	static unsigned int cp_tx = 0;
//  static unsigned int cp_rx = 0;
	
  if (TI0==1) // On peut envoyer une nouvelle donnée sur la liaison série
  { 
  	if(!RB_EMPTY(&out)) {
       SBUF0 = *RB_POPSLOT(&out);      /* start transmission of next byte */
       RB_POPADVANCE(&out);            /* remove the sent byte from buffer */
//			 cp_tx++;
  	}
  	else TXactive = 0;                 /* TX finished, interface inactive */
	TI0 = 0;
  }
  else // RI0 à 1 - Donc une donnée a été reçue
  {
		if(!RB_FULL(&in)) {                   // si le buffer est plein, la donnée reçue est perdue
     	*RB_PUSHSLOT(&in) = SBUF0;        /* store new data in the buffer */
		  RB_PUSHADVANCE(&in);               /* next write location */
//		  cp_rx++;
	 }
   RI0 = 0;
  }
}
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
//cfg_Clock_UART
//	Utilisation du Timer 1
//*****************************************************************************	 
void cfg_Clock_UART(void)
{
  CKCON |= 0x10;      // T1M: Timer 1 use the system clock.
  TMOD |= 0x20;       //  Timer1 CLK = system clock
	TMOD &= 0x2f;			  // Timer1 configuré en timer 8 bit avec auto-reload	
	TF1 = 0;				  // Flag Timer effacé
	
	TH1 = -(Preload_Timer0);
	ET1 = 0;				   // Interruption Timer 1 dévalidée
	TR1 = 1;				   // Timer1 démarré

}
/*
void time2 interrupt 5(void){
	T2 &= 0x7F;
}
*/
 

//*****************************************************************************	 
//CFG_uart0_mode1
//
//*****************************************************************************	 
void cfg_UART0_mode1(void)
{
		RCLK0 = 0;     // Source clock Timer 1
		TCLK0 = 0;
		PCON  |= 0x80; //SMOD0: UART0 Baud Rate Doubler Disabled.
		PCON &= 0xBF;  // SSTAT0=0
		SCON0 = 0x70;   // Mode 1 - Check Stop bit - stock validée
		TI0 = 1;        // Transmission: octet transmis (prêt à recevoir un char)
					          // pour transmettre			
    ES0 = 1;        // interruption UART0 autorisée	
}

void cfg_UART1_mode1(void)
{
		PCON  |= 0x10; //SMOD0: UART1 Baud Rate Doubler Disabled.
		PCON &= 0xF7;  // SSTAT1=0
		SCON1 = 0x60;   // Mode 1 - Check Stop bit - stock validée
	
    //EIE2 |= 0x40;        // interruption UART0 autorisée	ES1=1
}
void putChar1(char carac){
	SBUF1=carac;
	while((SCON1|0xFD)==0xFD){
		}
	SCON1 &= 0xFD;
	}
	
void putString1(char chaine[32]){
		xdata int i;
		for (i=0;i<strlen(chaine);i++){
			putChar1(chaine[i]);
		}
	}
	
char getChar1(){
	char reception=' ';
	while((SCON1|0xFE)==0xFE){
		}
	reception = SBUF1;
	SCON1 &= 0xFE;
	serOutchar(reception);
	return reception;
	}
	
	
void Commande_effectuee(char c){
	while(getChar1()!=c){
	}
}
	void mon_itoa(int chiffre,char* distance_obs){
		xdata int i;
		xdata int a=100;
		for (i=0;i<3;i++){
			distance_obs[i]=0x30+floor(chiffre/a);
			chiffre = chiffre - (distance_obs[i]-0x30)*a;
			a = a/10;
		}
	}
		void mon_itoaD(int chiffre,char* distance_obs){
		xdata int i=0;
		xdata int n=0;
		xdata int a=1000;
		if (chiffre<0){
			chiffre=chiffre*(-1);
			distance_obs[0]='-';
			i++;
		}
		n=i+3;
		for (i;i<=n;i++){
			distance_obs[i]=0x30+floor(chiffre/a);
			chiffre = chiffre - (distance_obs[i]-0x30)*a;
			a = a/10;
		}
	}
	
	
	void avance(char* vitesse){
		putString1("mogo 1:");
		putString1(vitesse);
		putString1(" 2:");
		putString1(vitesse);
		putChar1('\r');
	}
	
	void calcDistanceX(char* distanceX){
		xdata int dist1=0;
		dist1=atoi(distanceX);
		dist1=(1.67)*334*dist1/1.76;
		mon_itoaD(dist1,distanceX);
		
	}
	
	void calcDistanceY(char* distanceY){
		xdata int dist1=0;
		dist1=atoi(distanceY);
		dist1=(1.67)*334*dist1/1.76;
		mon_itoaD(dist1,distanceY);
	}
	
	void rotationG(int angle){
		xdata char dist[4];
		angle=angle*5.4;
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
		angle=angle*5.4;
		mon_itoaD(angle,dist);
		putString1("digo 1:-");
		putString1(dist);
		putString1(":10 2:");
		putString1(dist);
		putString1(":10");
		putChar1('\r');
	}
	
	void avanceX(char* distanceX){
		if (distanceX[0]=='-'){
			rotationG(90);
		}
		else{
			rotationD(90);
		}
		Commande_effectuee('>');

		//delay(1500); /////////////////////// A changer plus tard ///////////////////////////////////////////////////
		putString1("digo 1:");
		putString1(distanceX);
		putString1(":20");
		putString1(" 2:");
		putString1(distanceX);
		putString1(":20");
		putChar1('\r');
	}
	
	void avanceY(char* distanceY,char* distanceX){
		if (distanceX[0]=='-'){
			if (distanceY[0]=='-'){
				rotationG(90);
			} 
			else{
				rotationD(90);
			}
		}
		else{
			if (distanceY[0]=='-'){
				rotationD(90);
			} 
			else{
				rotationG(90);
			}
		}
	  Commande_effectuee('>');
		//delay(1500); /////////////////////// A changer plus tard ///////////////////////////////////////////////////
		putString1("digo 1:");
		putString1(distanceY);
		putString1(":20");
		putString1(" 2:");
		putString1(distanceY);
		putString1(":20");
		putChar1('\r');
	}
		
	
	
	void recule(char* vitesse){
		putString1("mogo 1:-");
		putString1(vitesse);
		putString1(" 2:-");
		putString1(vitesse);
		putChar1('\r');
	}
	
	
	
	
	
	
	void transfert (char* stock) {
	xdata char vitesse2[3]="";
	xdata char angle[3]="";
	xdata char angleRobot[3]="";
	xdata int angleRob=0;
	xdata char distanceX[6]="";
	xdata char distanceY[6]="";
	xdata int i=0;
	xdata int n=0;
	xdata int j = 0;
	xdata int machin = 0;
	xdata int position_servo = 1;
	xdata char distance_obs[3]="";
	xdata int d=0;
		
	if(stock[0]=='D'){			// Début Epreuve
		serOutstring("******Epreuve 1******");
		epreuve=1;
		}
	else if(stock[0]=='E'){						// Fin Epreuve
		epreuve=0;
		serOutstring("******Fin Epreuve******");
		}
	else{
		if (epreuve==1){
			if((stock[1]=='V')&&(stock[0]='T')){								// TV vitesse
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
			
			else if(stock[0]=='B'){																	// Reculer
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

			else if(stock[0]=='S'){																	// STOP
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
			else if(stock[0]=='G'){
				j=0;
				//distanceX
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
				//distanceY
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
				calcDistanceX(distanceX);
				calcDistanceY(distanceY);				
				avanceX(distanceX);
        Commande_effectuee('>');
				
				//delay(1500); /////////////////////// A changer plus tard ///////////////////////////////////////////////////
				avanceY(distanceY,distanceX);
				Commande_effectuee('>');

				//delay(1500); /////////////////////// A changer plus tard ///////////////////////////////////////////////////
				if (distanceY[0]=='-'){
					rotationG(180);
				}
				angleRob=atoi(angleRobot);
			  Commande_effectuee('>');

				//delay(1500); /////////////////////// A changer plus tard ///////////////////////////////////////////////////
				if (angleRob>0){
					rotationG(angleRob);
				}
				else{
					angleRob=abs(angleRob);
					rotationD(angleRob);
				}
			serOutstring("\r\nB");
			}	
			
			
			//servomoteur
			else if((stock[1]=='S')&&(stock[0]='C')&&(stock[2]==' ')&&(stock[3]='H')&&(stock[4]==' ')&&(stock[5]='A')&&(stock[6]=':')){
				
				for (i=7;i<=strlen(stock);i++){
					angle[i-7]=stock[i];
				}
				if (stock[3]=='H'){
					strcpy(angleMem,angle);
				}
				machin = atoi(angle);
				if (((machin)>= -90)&&(machin<=90)){
					position_servo *= machin;
					servo_pos(position_servo);
					serOutstring("AS H");
				}else{
					serOutchar('#');
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
	

void ecoute (void){
	char c;
	int compteur = 0;
	char stock[32]="";
	
	while (((c=serInchar())!='\r')){
		if (c!=0){
			serOutchar(c);
			stock[compteur]=c;
			compteur ++;
	  }
	}
	transfert(stock);
}
