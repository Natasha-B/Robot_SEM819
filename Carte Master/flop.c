//------------------------------------------------------------------------------------
// PROJET TRANSVERSALE
//------------------------------------------------------------------------------------
// DATE: 
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS: TRANSVERSALE
//
//------------------------------------------------------------------------------------
#include <c8051f020.h>                    // SFR declarations
#include <stdio.h>
#include <intrins.h>
#include <string.h>
#ifndef CFG_Globale
   #define CFG_Globale
   #include <CFG_Globale.h>
#endif

//*************************************************************************************************
// Paramètresd modifiables
//*************************************************************************************************
#define       MAX_BUFLEN 32 // Taille des buffers de données
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

/* Transmission and Reception Data Buffers */
static char  xdata outbuf[MAX_BUFLEN];     /* memory for transmission ring buffer #1 (TXD) */
static char  xdata inbuf [MAX_BUFLEN];     /* memory for reception ring buffer #2 (RXD) */
static  bit   TXactive = 0;             /* transmission status flag (off) */

/* define out (transmission)  and in (reception)  ring buffer control structures */
static RB_CREATE(out,unsigned char xdata);            /* static struct { ... } out; */
static RB_CREATE(in, unsigned char xdata);            /* static struct { ... } in; */
//*************************************************************************************************

//**************************************************************************************************
//**************************************************************************************************
void UART0_ISR(void) interrupt 0x4 {
 
  static unsigned int cp_rx = 0;
	
  if (RI0 ==1) // RI0 à 1 - Donc une donnée a été reçue
  {
		if(!RB_FULL(&in)) {                   // si le buffer est plein, la donnée reçue est perdue
     	*RB_PUSHSLOT(&in) = SBUF0;        /* store new data in the buffer */
		  RB_PUSHADVANCE(&in);               /* next write location */
		  cp_rx++;
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
// *************************************************************************************************

//*****************************************************************************	 
//cfg_Clock_UART
//	Utilisation du Timer 1
//*****************************************************************************	 
void cfg_Clock_UART(void)
{
	CKCON |= 0x10;      // T1M: Timer 1 use the system clock.
  TMOD |= 0x20;       //  Timer1 CLK = system clock
	TMOD &= 0x2f;			  // Timer1 configuré en timer 8 bit avec auto-reload	
	TF1 = 0;				  	// Flag Timer effacé

	TH1 = 0xDD;
	ET1 = 0;				   	// Interruption Timer 1 dévalidée
	TR1 = 1;				   	// Timer1 démarré
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
		SCON0 = 0x70;   // Mode 1 - Check Stop bit - Reception validée
		TI0 = 1;        // Transmission: octet transmis (prêt à recevoir un char
					          // pour transmettre			
    ES0 = 1;        // interruption UART0 autorisée	
}
