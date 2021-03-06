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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

sfr16 TH4_TL4 = 0xF4;  // Timer 4: T4L 0xF4  T4H 0xF5 
 
unsigned int cp; 
long sysclk = 22118400;  // Fr�quence de l'horloge externe 
int epreuve = 0; 	//Booleen indiquant si l'epreuve est en cours ou non
char reception[10];
char message[10]; 
char recup[2];
int encours = 0; 	//Booleen indiquant si une transmission est en cours ou non
 
 // Servomoteur HS-422: +
sbit Cde_Servo = P1^0; 
float t_pos_min = 0.5;  // Dur�e de l'impulsion en ms pour -90� 
float t_pos_max = 2.4;  // Dur�e de l'impulsion en ms pour 90� 
float alpha;  // Dur�e d'impulsion par d�gr� d'angle 
int pos = 1;
 
 void delay(int j){ 
 	unsigned int n, cp; 
 	for(n=0;n<j;n++){ 
 		for(cp=0;cp<2500;cp++){ 
 		}; 
 	} 
 } 


 void cfg_clk_ext(void){
	//Enable external oscillator with f=22MHz
	OSCXCN = 0x67;
	
	delay(1);
	 
	OSCICN = 0x08;	
}
 
 //------------------------------------------------------------------------------------ 
 // Configurations 
 //------------------------------------------------------------------------------------ 
 
 
 /** 
  * Configuration du Timer 4: fin de l'impulsion 
  * Registre modifi�s: T4CON, RCAP4L/RCAP4H  
  * @param void 
  * @return void 
  */ 
void cfg_timer4(){ 

// Configuration du Timer 4 en mode Auto-reload: 
//       RCLK1    TCLK1    CPRL4  
 	 // Activation du timer 4: 
	//T4CON |= 0x04; 
// Timer configur� sur l'horloge du system: 
 	CKCON |= 0x40; 
 } 
 
 
/** 
 * Configuration du port de sortie du signal Cde_Servo pour le servomoteur(P1.0) 
 * Registre modifi�s:PMD1OUT, P1.0 
 * @param void 
 * @return void 
 */ 
 void cfg_servo(){ 
// Initialisation de la sortie � 0: 
 	Cde_Servo = 0; 
 	 
// Active la sortie en push-pull: 
 	P1MDOUT |= 1; 
 	 
// Dur�e d'impulsion par d�gr� d'angle: 
 alpha = (t_pos_max - t_pos_min) / 180;  	 	 
} 

void cfg_crossbar(){
	XBR2 = 0x40; 
}
 
//Va permettre la reception et la transmission de donnees
void configUART0(){
	PCON |= 0xC0;	//Activation du bade rate doubler
	SM00 = 0;	//Selection du mode 1 de l'UART0
	SM10 = 1;
	SM20 = 1;	//Bit de stop � 1;
	REN0 = 1; 	//Autorisation des receptions
	TI0 = 0;	//Clear le flag de transmission
	RI0 = 0; 	//Clear leDDDDDDD flag de reception
	ES0 = 1; 	//Autorisation de l'interrutpion de reception et transmission
	T2CON &= 0xCF;	//Selection du timer 1 comme generateur de bade rate
}

//Utilise pour la clock de l'UART0
void configT1(){
	CKCON |= 0x10; //Utilisation de la fr�quence normale du quartz 
	TL1 = 0xB8;	//Initialisation du timer1
	TH1 = 0xB8;	//Initialisation du chargement du timer1
	TMOD |= 0x20;	//Activation du mode autoreload
	// ET1 = 1;	//Autorisation de l'interruption (� voir si necessaire ou non)	
	TR1 = 1;	//Lancement du timer1
}



//------------------------------------------------------------------------------------ 
// Utilisations 
//------------------------------------------------------------------------------------ 
 
 
 /** 
 * Interruption pour la fin de l'impulsion envoy� au servomoteur 
 * interruption utilis�e: Timer 4 Overflow 
  * @param void 
  * @return void 
  */ 
 void int_timer4_delay() interrupt 16{ 
// Fin de l'impulsion du contr�le du servomoteur: 
 	Cde_Servo = 0; 
 	 
// D�sactivation du timer 4: 
 	T4CON &= 0xFB; 
 	 
// D�sactive le flag du Timer 4: 
 	T4CON &= 0x7F; 
 } 

//Fonction d'interruption li�e � l'UART0
void int_UART0() interrupt 4 {
	if (TI0 == 1) {		// interruption due a une fin de transmission
		TI0 = 0;
		encours = 0;	
	} else if (RI0 == 1) {	//interruption due a une fin de reception
		RI0 = 0;
		*recup = SBUF0;
		strcat(reception, recup);
	}
} 

 
 /** 
  * Conversion d'un angle en la valeur � charger dans un Timer: 
  * @param  int pos  angle demand�e [-90�,90�] 
  * @return int timer_load  valeur � int�grer au timer pour g�n�r� la bonne dur�e d'impulsion 
  */ 
 float t_pulse; 
 unsigned int timer_count; 
 unsigned int timer_load; 

 unsigned int pos2timer_count(){   // int pos
	pos += 90;  // pos = [0�, 180�] 
	
	// Dur�e de l'impulsion: 
	t_pulse = (t_pos_min + (pos * alpha)); 
 	 
	// Nombre de co�t d'horloge pour g�n�r� l'impulsion: 
	timer_count = t_pulse* 0.001 * (sysclk); 
 	 
	// Valeur de rechargement du Timer 4: 
 	timer_load = 0xFFFF - timer_count; 
	return timer_load;
	
 }  
 
 /** 
  * Commande de position envoy�e au servomoteur 
  * @param int pos: position angulaire [-90�, 90�] que l'on r�cup�re via une liaison UART que l'on convertit en int  
  * @return void 
  */ 
 void servo_pos(){ // int pos
// Conversion de l'angle en la valeur � charger dans le Timer: 
 	int timer_load = pos2timer_count(); // pos
 	// Envoie la commande plusieurs fois au servomoteur: 
 	int i,j,k;
 	for(i = 0; i < 1000; i++) { 
		// Initialisation du Timer 4 � la valeur souhait�e: 
 		TH4_TL4 = timer_load; 
		
		// Activation du timer 4: 
	  T4CON |= 0x04; 
		
		
		
		// Active la sortie contr�lant le servomoteur: 
 		Cde_Servo = 1;  
 		 
		//... La sortie contr�lant le servomoteur est d�sactiv�e dans l'interruption de Timer 4 
			 
		// Attend le flag: 
 		while(Cde_Servo == 1); 
 		 
 		// Attente suppl�mentaire: 
		for(j=0; j<0xFF; j++) for(k=0; k<0x15; k++) ; 
 	} 
 } 

void Transmit(){
	int i = 0;
	REN0 = 0;
	//Desactivation de la reception durant la transmission
	
	while ( i < strlen(message)){
		if(encours == 0){
			SBUF0 = message[i];
			i++;
			encours = 1;
		}
	}
	*message = '\0';
	REN0 = 1;	//Activation de la reception d=suite � la transmission
}

void test_reception(){
	char testD = 'D';
	if ((reception == 'D') && (epreuve == 0)) {
		epreuve = 1;	//Signalement du d�but de l'epreuve
		strcpy(message,"l......."); //Message de bon demarrage de l'epreuve
		Transmit();	//Transmission du message
		*reception = '\0';	//Clear de reception et attente de nouvelles instructions
	} else if ((epreuve == 0) && (reception == '\0')) {
		*message = '#';	//Message d'erreur
		*reception = '\0';	//Clear la reception
		Transmit();	//Transmet le message
	} else if (epreuve == 1) {
		char comp[10] = "CS H A:";
		if (reception == 'E') {
			epreuve = 0;	//Fin d'epreuve
			*reception = '\0';
		} else if ((reception == 'C') || (reception != '\0')) {
			*message = '#';
			Transmit();
			*reception = '\0';
		} else {
			int i = 0;
			int messageOK = 1;	//Permet de d�tecter si le message est bien celui demande
			if ((strcmp(reception, comp) != 0)) {
				if (strlen(reception) >= strlen(comp)){
					*message = '#';
					Transmit();
					*reception = '\0';
					messageOK = 0;
				} else {
					messageOK = 0;
				}
			}
			if (messageOK){
				delay(5000); //delai pour taper l'angle : 5s
				REN0 = 0;	//Empeche de taper de nouvelles commandes durant le traitement
				if (reception[strlen(comp)] == '-'){
					pos = -1;
					strcat(comp, '-');
				}
				if (strlen(reception) == strlen(comp)+2){ //Cas o� on a des dizaines
					if((reception[strlen(comp)]>0x29) && (reception[strlen(comp)] < 0x40) && (reception[strlen(comp)+1] > 0x29) && (reception[strlen(comp)+1] < 0x40)){
						char diz = reception[strlen(comp)];
						char unit = reception[strlen(comp)+1];
						pos = pos*(atoi(&diz)*10 + atoi(&unit));
						if ((pos >= -90) && (pos <= 90)){
							servo_pos();
							strcpy(message, "AS H");
							Transmit();
						} else {
							*message = '#';
							Transmit();
							*reception = '\0';	
						}
					} else {
						*message = '#';
						Transmit();
						*reception = '\0';	
					}
				} else if (strlen(reception) == i+2){
					if((reception[strlen(comp)]>0x29) && (reception[strlen(comp)] < 0x40)){
						pos *= atoi(&reception[strlen(comp)]);
						servo_pos();
						strcpy(message, "AS H");
						Transmit();
					} else {
						*message = '#';
						Transmit();
						*reception = '\0';	
					}
				}
				REN0 = 1; 	//Permet de taper de nouvelles commandes
				*reception = '\0'; 
			}
		}
	}
}

void config(){
	//Activation des interruptions
	EA = 1;
	EIE2 = 0x04;
	
	cfg_clk_ext();
	configUART0();
	configT1();
	cfg_timer4();
	cfg_crossbar();
	cfg_servo();	
}

void main(){
	//desactivation du watchdog
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	config();
			
	//servo_pos();
	
	while(1){
		test_reception();
	}
}
 
