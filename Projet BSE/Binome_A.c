//------------------------------------------------------------------------------------
// Binome_A.c
//------------------------------------------------------------------------------------
// DATE: 19/12/2018
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS: ALBERT, JANNIN
//
// Numéro du sujet: 1
//
//------------------------------------------------------------------------------------
// Definitions des variables globales
//------------------------------------------------------------------------------------
#include <c8051F020.h>

sbit LED = P1^6;
sfr16 TH4_TL4 = 0xF4;  // Timer 4: T4L 0xF4  T4H 0xF5

char Cpt_timer3 = 0;
unsigned int cp;
long sysclk = 22118400;  // Fréquence de l'horloge externe

// Servomoteur:
sbit Cde_Servo = P1^0;
float t_pos_min = 0.6;  // Durée de l'impulsion pour -90°
float t_pos_max = 2.3;  // Durée de l'impulsion pour 90°
float alpha;  // Durée d'impulsion par dégré d'angle


// LED:
char blink_led = 0;

//------------------------------------------------------------------------------------
// Configurations
//------------------------------------------------------------------------------------

/**
 * Configuration du Timer 3: gestion du clignotement de la LED
 * Registre modifiés:TRM3CN, TMR3RLL/TMR3RLH
 * @param void
 * @return void
 */
void cfg_timer3(){
	
	TMR3CN = 0x02;  // Horloge /12
	
	// Initialisation des registres de rechargement du Timer3:
	TMR3RLL = 0x00;  
	TMR3RLH = 0x00;
	
	TMR3CN |= 0x04; // Activation du Timer3
}


/**
 * Configuration du Timer 4: fin de l'impulsion
 * Registre modifiés: T4CON, RCAP4L/RCAP4H 
 * @param void
 * @return void
 */
void cfg_timer4(){

	// Configuration du Timer 4 en mode Auto-reload:
	//       RCLK1    TCLK1    CPRL4
	T4CON = (0<<5) + (0<<4) + (0<<0);
	
	// Timer configuré sur l'horloge du system:
	CKCON = (1<<6);
}


/**
 * Configuration du port de la LED(P1.6)
 * Registre modifiés:PMD1OUT, P1.6
 * @param void
 * @return void
 */
void cfg_LED(){
	
	P1MDOUT |= 0x40;  // Autorisation de la LED  
	LED = 0;  
}


/**
 * Configuration du port de sortie du signal Cde_Servo pour le servomoteur(P1.0)
 * Registre modifiés:PMD1OUT, P1.0
 * @param void
 * @return void
 */
void cfg_servo(){
	// Initialisation de la sortie à 0:
	Cde_Servo = 0;
	
	// Active la sortie en push-pull:
	P1MDOUT |= 1;
	
	// Durée d'impulsion par dégré d'angle:
	alpha = (t_pos_max - t_pos_min) / 180;
	
	
}


//------------------------------------------------------------------------------------
// Utilisations
//------------------------------------------------------------------------------------


/**
 * Interruption pour gestion clignotement LED
 * interruption utilisée: Timer 3 Overflow
 * @param void
 * @return void
 */
void int_timer3_LED() interrupt 14{
	TMR3CN &= ~(1<<7);  // RAZ du flag d'interruption Timer3
	Cpt_timer3++ ;		// Compte le nombre d'interruptions
	
	if(blink_led == 1 && Cpt_timer3 >= 56){
		LED = !LED;		// Changement d'etat de la LED
		Cpt_timer3 = 0;	// RAZ du compteur d'interruptions
	}	
}


/**
 * Interruption pour la fin de l'impulsion envoyé au servomoteur
 * interruption utilisée: Timer 4 Overflow
 * @param void
 * @return void
 */
void int_timer4_delay() interrupt 16{
	// Fin de l'impulsion du contrôle du servomoteur:
	Cde_Servo = 0;
	
	// Désactivation du timer 4:
	T4CON &= ~(1 << 2);
	
	// Désactive le flag du Timer 4:
	T4CON &= ~(1 << 7);
}


/**
 * Conversion d'un angle en la valeur à charger dans un Timer:
 * @param  int pos  angle demandée [-90°,90°]
 * @return int timer_load  valeur à intégrer au timer pour généré la bonne durée d'impulsion
 */
float t_pulse;
unsigned int timer_count;
unsigned int timer_load;
unsigned int pos2timer_count(int pos){
	pos += 90;  // pos = [0°, 180°]

	// Durée de l'impulsion:
	t_pulse = (t_pos_min + (pos * alpha));
	
	// Nombre de coût d'horloge pour généré l'impulsion:
	timer_count = t_pulse* 0.001 * (sysclk);
	
	// Valeur de rechargement du Timer 4:
	timer_load = 0xFFFF - timer_count;
	
	return timer_load;
}


/**
 * Commande de position envoyée au servomoteur
 * @param int pos: position angulaire [-90°, 90°]
 * @return void
 */
void servo_pos(int pos){
	// Conversion de l'angle en la valeur à charger dans le Timer:
	int timer_load = pos2timer_count(pos);
	
	// Envoie la commande plusieurs fois au servomoteur:
	int i, j, k;
	for(i = 0; i < 150; i++) {
		// Initialisation du Timer 4 à la valeur souhaitée:
		TH4_TL4 = timer_load;
		
		// Active la sortie contrôlant le servomoteur:
		Cde_Servo = 1; 
		
		// Activation du timer 4:
		T4CON |= (1<<2);
		
		//... La sortie contrôlant le servomoteur est désactivée dans l'interruption de Timer 4
			
		// Attend le flag:
		while(Cde_Servo == 1) ;
		
		// Attente supplémentaire:
		for(j=0; j<0xFF; j++) for(k=0; k<0x15; k++) ;
	}
}


/**
 * Active le clignotement de la LED:
 * @return void
 */
void start_blink_led() {
	blink_led = 1;
}

/**
 * Désactive le clignotement de la LED:
 * @return void
 */
void stop_blink_led() {
	blink_led = 0;
	LED = 0;
}
