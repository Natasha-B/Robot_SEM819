//---------------------------------------------------------------------------------------
// ID_1.1    UART0_RingBuffer_lib.C -----------------------------------------------------
//---------------------------------------------------------------------------------------
// ID_1.2  Source originale des codes de buffer circulaire: 
//         Microcontroller Programming III	MP7-46 - 
//         Universit� d'Adelaide 
//
// ID_1.3 Adapt� par F.JOLY - CPE Lyon
// ID_1.4 DATE: 08 Mars 2016 
// ID_1.5 Version 1.0 
// 
// ID_1.6 Objectifs ---------------------------------------------------------------------
// Gestion d'une UART en �mission-r�ception par interruption et buffer circulaire
// 
// ID_1.7 D�pendances mat�rielles "cible" ----------------------------------------------- 
// Processeur cible: C8051F02x
// P�riph�rique UART0

// ID_1.8 D�pendances mat�rielles "ext�rieures" -----------------------------------------
// 

// ID_1.9 D�pendances de type communications/protocolaires ------------------------------
// La configuration de la communication UART est du type asynchrone - 8bits - 1 stop bit
// Pas de parit�
//
// ID_1.10 Fonctionnement du code -------------------------------------------------------
// Le code contient:
// - les fonctions de configuration de l'UART0 et de son timer associ�. 
// - Une fonction de cr�ation du buffer circulaire (initialisation de structures)
// - Les fonctions de remplissage du buffer de transmission et les fonctions de vidage 
//   du buffer de r�ception.
//  - La fonction d'interruption UART0 charg�e d'�mettre sur la liaison s�rie le contenu 
//    du buffer de transmission et de stocker dans le buffer de r�ception les donn�es 
//    re�ues sur la liaison s�rie.

// ID_1.11 Choix technologiques divers --------------------------------------------------
// Utilisation de L'UART0 et du Timer 1 comme source d'horloge de l'UART0.
// Pour fonctionner ce code a besoin des macros Define SYSCLK et BAUDRATE

// ID_1.12 Tests r�alis�s ---------------------------------------------------------------
// Validation sur plateforme 8051F020TB avec processeur 8051F020
// Vitesse de transmission: 115200 Baud
// Fr�quence quartz: 22,1184 MHz
//
// ID_1.13 Chaine de compilation --------------------------------------------------------
// KEIL C51 6.03 / KEIL EVAL C51
//
// ID_1.14 Documentation de r�f�rence ---------------------------------------------------
// Datasheet 8051F020/1/2/3  Preliminary Rev. 1.4 12/03 
//
//ID_1.15 Commentaires sur les variables globales et les constantes ---------------------
// La taille des buffers de r�ception et de transmission est modifiable avec la 
// macro MAX_BUFLEN  

//*************************************************************************************************
#include <c8051f020.h>                    // SFR declarations
#include <stdio.h>
#include <intrins.h>
#include <string.h>
#ifndef CFG_Globale
   #define CFG_Globale
   #include <CFG_Globale.h>
#endif


//*************************************************************************************************
// Param�tresd modifiables
//*************************************************************************************************
#define       MAX_BUFLEN 32 // Taille des buffers de donn�es

//*************************************************************************************************
// DEFINITION DES MACROS DE GESTION DE BUFFER CIRCULAIRE
//*************************************************************************************************

// Structure de gestion de buffer circulaire
	//rb_start: pointeur sur l'adresse de d�but du buffer 
	// rb_end: pointeur sur l'adresse de fin du buffer	
	// rb_in: pointeur sur la donn�e � lire
	// rb_out: pointeur sur la case � �crire
		
#define RB_CREATE(rb, type) \
   struct { \
     type *rb_start; \	   
     type *rb_end; \	   
     type *rb_in; \
	   type *rb_out; \		
	  } rb

//Initialisation de la structure de pointeurs 
// rb: adresse de la structure
// start: adresse du premier �l�ment du buffer 
// number: nombre d'�l�ments du buffer - 1	(le "-1" n'est � mon avis pas n�cessaire)
#define RB_INIT(rb, start, number) \
         ( (rb)->rb_in = (rb)->rb_out= (rb)->rb_start= start, \
           (rb)->rb_end = &(rb)->rb_start[number] )

//Cette macro rend le buffer circulaire. Quand on atteint la fin, on retourne au d�but
#define RB_SLOT(rb, slot) \
         ( (slot)==(rb)->rb_end? (rb)->rb_start: (slot) )

// Test: Buffer vide? 
#define RB_EMPTY(rb) ( (rb)->rb_in==(rb)->rb_out )

// Test: Buffer plein?
#define RB_FULL(rb)  ( RB_SLOT(rb, (rb)->rb_in+1)==(rb)->rb_out )

// Incrementation du pointeur dur la case � �crire
#define RB_PUSHADVANCE(rb) ( (rb)->rb_in= RB_SLOT((rb), (rb)->rb_in+1) )

// Incr�mentation du pointeur sur la case � lire
#define RB_POPADVANCE(rb)  ( (rb)->rb_out= RB_SLOT((rb), (rb)->rb_out+1) )

// Pointeur pour stocker une valeur dans le buffer
#define RB_PUSHSLOT(rb) ( (rb)->rb_in )

// pointeur pour lire une valeur dans le buffer
#define RB_POPSLOT(rb)  ( (rb)->rb_out )


//*************************************************************************************************


/* Transmission and Reception Data Buffers */
static char  xdata outbuf[MAX_BUFLEN];     /* memory for transmission ring buffer #1 (TXD) */
static char  xdata inbuf [MAX_BUFLEN];     /* memory for reception ring buffer #2 (RXD) */
static  bit   TXactive = 0;             /* transmission status flag (off) */

/* define out (transmission)  and in (reception)  ring buffer control structures */
static RB_CREATE(out,unsigned char xdata);            /* static struct { ... } out; */
static RB_CREATE(in, unsigned char xdata);            /* static struct { ... } in; */

//**************************************************************************************************
//**************************************************************************************************
void UART0_ISR(void) interrupt 0x4 {
 
//	static unsigned int cp_tx = 0;
//  static unsigned int cp_rx = 0;
	
  if (TI0==1) // On peut envoyer une nouvelle donn�e sur la liaison s�rie
  { 
  	if(!RB_EMPTY(&out)) {
       SBUF0 = *RB_POPSLOT(&out);      /* start transmission of next byte */
       RB_POPADVANCE(&out);            /* remove the sent byte from buffer */
//			 cp_tx++;
  	}
  	else TXactive = 0;                 /* TX finished, interface inactive */
	TI0 = 0;
  }
  else // RI0 � 1 - Donc une donn�e a �t� re�ue
  {
		if(!RB_FULL(&in)) {                   // si le buffer est plein, la donn�e re�ue est perdue
     	*RB_PUSHSLOT(&in) = SBUF0;        /* store new data in the buffer */
		  RB_PUSHADVANCE(&in);               /* next write location */
//		  cp_rx++;
	 }
   RI0 = 0;
  }
}
// **************************************************************************************************
// init_Serial_Buffer: Initialisation des structuresde gestion des buffers transmission et reception
// *************************************************************************************************
//**************************************************************************************************
void init_Serial_Buffer(void) {

    RB_INIT(&out, outbuf, MAX_BUFLEN-1);           /* set up TX ring buffer */
    RB_INIT(&in, inbuf,MAX_BUFLEN-1);             /* set up RX ring buffer */

}
// **************************************************************************************************
// SerOutchar: envoi d'un caract�re dans le buffer de transmission de la liaison s�rie
// *************************************************************************************************
unsigned char serOutchar(char c) {

  if(!RB_FULL(&out))  // si le buffer n'est pas plein, on place l'octet dans le buffer
  {                 
  	*RB_PUSHSLOT(&out) = c;               /* store data in the buffer */
  	RB_PUSHADVANCE(&out);                 /* adjust write position */

  	if(!TXactive) {
		TXactive = 1;                      /* indicate ongoing transmission */
 	  TI0 = 1;//   Placer le bit TI � 1 pour provoquer le d�clenchement de l'interruption
  	}
	return 0;  // op�ration correctement r�alis�e 
  }
   else return 1; // op�ration �chou�e - Typiquement Buffer plein
}
// ************************************************************************************************
//  serInchar: 	lecture d'un caract�re dans le buffer de r�ception de la liaison s�rie
//  Fonction adapt�e pour la r�ception de codes ASCII - La r�ception de la valeur binaire 0
//  n'est pas possible (conflit avec le code 0 retourn� si il n'y a pas de caract�re re�u)
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
//  serInchar_Bin: 	lecture d'un caract�re dans le buffer de r�ception de la liaison s�rie
//  Fonction adapt�e pour la r�ception de codes Binaires - Cette fonction retourne un entier. 
//  L'octet de poids faible correspond au caract�re re�u, l'octet de poids fort, s'il est nul indique 
//  qu'aucun caract�re n'a �t� re�u.
//  
// ************************************************************************************************
unsigned int serInchar_Bin(void) {
char c;
unsigned int return_code = 0;
	 
  if (!RB_EMPTY(&in))
  {                
    // un caract�re au moins est dans le buffer de r�ception
  	c = *RB_POPSLOT(&in);                 /* get character off the buffer */
 	  RB_POPADVANCE(&in);                   /* adjust read position */
  	return 0xFF00+c;
  }
	// pas de caract�re dans le buffer de r�ception.
  else return return_code;
}
// *************************************************************************************************
// serOutstring:  Envoi d'une chaine de caract�re dans le buffer de transmission
// ************************************************************************************************
unsigned char serOutstring(char *buf) {
unsigned char len,code_err=0;
  for(len = 0; len < strlen(buf); len++)
			while(buf[len]!='\r')
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
	TMOD &= 0x2f;			  // Timer1 configur� en timer 8 bit avec auto-reload	
	TF1 = 0;				  	// Flag Timer effac�

	TH1 = -(Preload_Timer0);
	ET1 = 0;				   // Interruption Timer 1 d�valid�e
	TR1 = 1;				   // Timer1 d�marr�
}
 
//*****************************************************************************	 
//CFG_uart0_mode1
//
//*****************************************************************************	 
void cfg_UART0_mode1(void)
{
		RCLK0 = 0;     	// Source clock Timer 1
		TCLK0 = 0;
		PCON  |= 0x80; 	//SMOD0: UART0 Baud Rate Doubler Disabled.
		PCON &= 0xBF;  	// SSTAT0=0
		SCON0 = 0x70;   // Mode 1 - Check Stop bit - Reception valid�e
		TI0 = 1;        // Transmission: octet transmis (pr�t � recevoir un char
					          // pour transmettre			
    ES0 = 1;        // interruption UART0 autoris�e	
}

//*****************************************************************************	 
//CFG_uart1_mode1
//
//*****************************************************************************	 
void cfg_UART1_mode1(void)
{
		RCLK0 = 0;     		// Source clock Timer 1
		TCLK0 = 0;
		PCON  |= 0x10; 		//SMOD0: UART0 Baud Rate Doubler Disabled.
		PCON &= 0xF7;  		// SSTAT0=0
		SCON1 |= 0x72;   	// Mode 1 - Check Stop bit - Reception valid�e et TI1 activ�
		EIE2 |= 0x40;
	
    ES1 = 1;        	// interruption UART0 autoris�e	
}