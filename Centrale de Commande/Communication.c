//------------------------------------------------------------------------------------
// Projet_Transversal
//------------------------------------------------------------------------------------
// DATE: 27/02/19
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS: Blasco, Bru, Renotton
//
// Groupe: B2
//
//------------------------------------------------------------------------------------
#include "c8051F020.h"
#include "Communication.h"


// UART
char UART_buff;
char UART_busy = 0;

// TIMER2
sfr16 T2 = 0xcc;
sfr16 RCAP2 = 0xCA;


//------------------------------------------------------------------------------------
// CONFIGURATION
//------------------------------------------------------------------------------------


/**
 * Configuration de la fonction Delay attente de x ms
 * @param int j, j est le nobte de ms à attendre
 * @return void
 */

void delay(int j){
	unsigned int n, cp;
	for(n=0;n<j;n++){
		for(cp=0;cp<2500;cp++){
		};
	}
}

/**
 * Configuration de l'horloge externe à 22,1184 MHz, pour assurer une bonne transmission/reception série
 * @param void
 * @return void
 * Registres modifiés : OSCICN , OSCXCN
 */

void cfg_clk (void){
	// Enable the external oscillator
	OSCXCN = 0x67;

	// Wait at least 1ms
	delay(2);

	// Switch the system clk to the external oscillator
	OSCICN = 0x1c;
}

/**
 * Configuration du Timer 2 pour la gestion de l'UART
 * @param void
 * @return void
 * Registres modifiés : T2CON, CKCON , RCAP2, IE
 */
void cfg_timer2 (void) {
	RCLK0 = 1;
	TCLK0 = 1;
	CPRL2 = 0;
	CKCON |= 0x20;
	RCAP2 = 0xFFB8; // 9600b
	
	// Interruptions
	ES0 = 1;
	ET2 = 0x00; // Overflow
	
	// Démarrage TIMER2
	TR2 = 1;
}

/**
 * Configuration de l'UART
 * @param void
 * @return void
 * Registres modifiés : SCON0, P0MDOUT, XBR2, XBR0
 */
void cfg_UART (void) {
	SCON0 = 0x50;
	P0MDOUT = 0xFF;
	XBR2 |= 0x40;
	XBR0 |= 0x04;
}
	
/**
 * Activation des interruptions
 * @param void
 * @return void
 * Registres modifiés : IE
 */
void cfg_interrupt (void) {
	EA = 1; // enable all interupts
}



//------------------------------------------------------------------------------------
// UTILISATION
//------------------------------------------------------------------------------------


/**
 * Initialisation de la configuration
 * @param void
 * @return void
 */
void init (void) {
	cfg_clk();
	cfg_UART();
	cfg_timer2();
	cfg_interrupt();
	
	XBR2 |= 1<<6;	//Activation du Crossbar
	
}

/**
 * Conversion int to string
 * @param int v , valeur à convertir
 * @return string str2, chaine de caractères associée
 */
char *itos(unsigned int v) {
	char str1[20] = "", str2[20] = "";
	int unite = 0;
	int indice = 0, indice2 = 0;
	while(v != 0){
		unite = v%10; // Unité
		str1[indice] = 0x30 + unite;
		indice++;
		v = (v - unite)/10;
	}
	
	if(indice == 0) {
		indice = 1;
		str1[0] = '0';
	}
	
	// Reverse de string
	for(;indice > 0;indice--) {
		str2[indice-1] = str1[indice2];
		indice2++;
	}
	str2[indice2+1] = '\0';
	
	return str2;
}

/**
 * Envoi d'un caractère à l'UART
 * @param char c , caractère à envoyer
 * @return void
 * Registres modifiés : SBUF0
 */
void UART_send (char c) {
	while(UART_busy == 1) {}
	UART_busy = 1;
	SBUF0 = c;
}

/**
 * Envoi d'une chaine de caractères à l'UART
 * @param char* s , chaine de caractères à envoyer
 * @return void
 */
void UART_sends (char* s) {
	int i = 0;
	while(s[i] != '\0') {
		UART_send(s[i]);
		i++;
	}
}


/**
 * Envoi d'un retour à la ligne à l'UART
 * @param void
 * @return void 
 */
void UART_sendCRLF (void) {
	UART_send(0x0D);
	UART_send(0x0A);
}

/**
 * Envoi d'un entier à l'UART
 * @param char* prefix, préfixe du message
 * @param int a, entier à envoyer
 * @param char* suffix, suffixe du message
 * @return void
 */
void UART_sendi (char *prefix, int a, char *suffix) {
	UART_sends(prefix);
	UART_sends(itos(a));
	UART_sends(suffix);
	UART_sendCRLF();
}


/**
 * Réception d'un caractère sur l'UART 
 * @param void
 * @return void
 */
void UART_receive (void) {
	char invalid_cmd = 0;
	if(UART_buff == 0x00)
		return;
	
	switch (UART_buff) {
		
		case '0' : 
			run(0);
			break;
		
		case 'D' : 
			run(-90);
			break;
		
		case 'G' :
			run(90);
			break;
		
		case 'd':
			run(-45);
			break;
		
		case 'g' :
			run(45);
			break;
		
		default:
			invalid_cmd = 1;
			break;
	}
			
	UART_sends(invalid_cmd ? "Invalid command !" : "OK");
	UART_sendCRLF();
	UART_buff = 0x00;
}

/**
 * Interruption de l'UART
 * @param void
 * @return void
 * Registres modifiés : SCON0, SBUF0
 */
void UART() interrupt 4 {
	// Reception
	if(RI0 == 1){
		UART_buff = SBUF0;
		RI0 = 0;
	}
	
	// Transmission
	if(TI0 == 1) {
		UART_busy = 0;
		TI0 = 0;
	}
}



//------------------------------------------------------------------------------------
// APPLICATION
//------------------------------------------------------------------------------------

/**
 * Envoi du message de démarrage
 * @param void
 * @return void
 */
void Welcome() {
	UART_sendCRLF();
	UART_sendCRLF();
	UART_sends("********** CENTRALE DE COMMANDE **********");
	UART_sendCRLF();
	UART_sends("Bienvenue :)");
	UART_sendCRLF();
}

/**
 * Envoi de la consigne de rotation à la carte master
 * @param int angle, angle de rotation (en degrès)
 * @return void
 */
void run (int angle) {

	AMX0SL = 0x0; // AIN0
	AD0INT = 0;
	AD0BUSY = 1;
	while(AD0INT != 1);
	UART_sends(itos(angle));
	
}

