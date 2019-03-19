
//------------------------------------------------------------------------------------
// Projet_Transversal - Servomoteur.c
//------------------------------------------------------------------------------------
// DATE: 
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS: 
//
// Groupe: B2
//
//------------------------------------------------------------------------------------
#include <c8051F020.h> 
#include "servomoteur.h"
#include "CFG_Globale.h"
sfr16 TH4_TL4 = 0xF4;  // Timer 4: T4L 0xF4  T4H 0xF5 

unsigned int cp; 
//long sysclk = 22118400;  // Fr?quence de l'horloge externe 


 // Servomoteur HS-422: +
sbit Cde_Servo = P1^0; 
float t_pos_min = 0.5;  // Dur?e de l'impulsion en ms pour -90? 
float t_pos_max = 2.4;  // Dur?e de l'impulsion en ms pour 90? 
float alpha;  // Dur?e d'impulsion par d?gr? d'angle 

 void delay(int j){ 
 	unsigned int n, cp; 
 	for(n=0;n<j;n++){ 
 		for(cp=0;cp<2500;cp++){ 
 		}; 
 	} 
 } 


 /*void cfg_clk_ext(void){
	//Enable external oscillator with f=22MHz
	OSCXCN = 0x67;

	delay(1);

	OSCICN = 0x08;	
}*/

 //------------------------------------------------------------------------------------ 
 // Configurations 
 //------------------------------------------------------------------------------------ 


 /** 
  * Configuration du Timer 4: fin de l'impulsion 
  * Registre modifi?s: T4CON, RCAP4L/RCAP4H  
  * @param void 
  * @return void 
  */ 
void cfg_timer4(){ 

// Configuration du Timer 4 en mode Auto-reload: 
//       RCLK1    TCLK1    CPRL4  
 	 // Activation du timer 4: 
	//T4CON |= 0x04; 
		EIE2 |= 0x04;

// Timer configur? sur l'horloge du system: 
 	CKCON = 0x40; 
 } 


/** 
 * Configuration du port de sortie du signal Cde_Servo pour le servomoteur(P1.0) 
 * Registre modifi?s:PMD1OUT, P1.0 
 * @param void 
 * @return void 
 */ 
 void cfg_servo(){ 
// Initialisation de la sortie ? 0: 
 	Cde_Servo = 0; 

// Active la sortie en push-pull: 
 	P1MDOUT |= 1; 

// Dur?e d'impulsion par d?gr? d'angle: 
 alpha = (t_pos_max - t_pos_min) / 180;  	 	 
} 
/*
void cfg_crossbar(){
	XBR2 = 0x40; 
}*/


//------------------------------------------------------------------------------------ 
// Utilisations 
//------------------------------------------------------------------------------------ 


 /** 
 * Interruption pour la fin de l'impulsion envoy? au servomoteur 
 * interruption utilis?e: Timer 4 Overflow 
  * @param void 
  * @return void 
  */ 
 void int_timer4_delay() interrupt 16{ 
// Fin de l'impulsion du contr?le du servomoteur: 
 	Cde_Servo = 0; 

// D?sactivation du timer 4: 
 	T4CON &= 0xFB; 

// D?sactive le flag du Timer 4: 
 	T4CON &= 0x7F; 
 } 


 /** 
  * Conversion d'un angle en la valeur ? charger dans un Timer: 
  * @param  int pos  angle demand?e [-90?,90?] 
  * @return int timer_load  valeur ? int?grer au timer pour g?n?r? la bonne dur?e d'impulsion 
  */ 
 float t_pulse; 
 unsigned int timer_count; 
 unsigned int timer_load; 
 unsigned int pos2timer_count(int pos){   // int pos
	pos += 90;  // pos = [0?, 180?] 

// Dur?e de l'impulsion: 
	 t_pulse = (t_pos_min + (pos * alpha)); 

// Nombre de co?t d'horloge pour g?n?r? l'impulsion: 
	timer_count = t_pulse* 0.001 * (SYSCLK); 

// Valeur de rechargement du Timer 4: 
 	timer_load = 0xFFFF - timer_count; 

	return timer_load; 
 }  

 /** 
  * Commande de position envoy?e au servomoteur 
  * @param int pos: position angulaire [-90?, 90?] que l'on r?cup?re via une liaison UART que l'on convertit en int  
  * @return void 
  */ 
 void servo_pos(int pos){ // int pos
// Conversion de l'angle en la valeur ? charger dans le Timer: 
 	int timer_load = pos2timer_count(pos); // pos

 	// Envoie la commande plusieurs fois au servomoteur: 
 	int i,j,k;
 	for(i = 0; i < 1000; i++) { 
		// Initialisation du Timer 4 ? la valeur souhait?e: 
 		TH4_TL4 = timer_load; 

		// Activation du timer 4: 
	  T4CON |= 0x04; 



		// Active la sortie contr?lant le servomoteur: 
 		Cde_Servo = 1;  

		//... La sortie contr?lant le servomoteur est d?sactiv?e dans l'interruption de Timer 4 

		// Attend le flag: 
 		while(Cde_Servo == 1); 

 		// Attente suppl?mentaire: 
		for(j=0; j<0xFF; j++) for(k=0; k<0x15; k++) ; 
 	} 
 } 

void config_servo(){
	//Activation des interruptions
	
	cfg_timer4();
	cfg_servo();	
}

/*void main(){
	//desactivation du watchdog
	WDTCN = 0xDE;
	WDTCN = 0xAD;

	config();

	servo_pos();

	while(1){

	}
}*/


