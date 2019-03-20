//------------------------------------------------------------------------------------
// pointeur lumineux.c
//------------------------------------------------------------------------------------
// DATE: 11/03/19
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
// DEVLOPPE PAR : Bru Natacha, Brouse Léa 

//
//------------------------------------------------------------------------------------
#include "c8051F020.h"


// UART
char UART_buff [50];
char UART_busy = 0;


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
	EIE1 |= 0x08;
	EIP1 |= 0x08;
}


/**
 * Configuration de la PWM
 * @param void
 * @return void
 * Registres modifiés : 
 */

void cfg_PWM(void){
	PCA0CN |= 0x00;
	PCA0MD |= 0x09;
	PCA0CPM0 |= 0x42;
	PCA0CPH0 = 0x00; // initialisé à 0
	XBR0 |= 0x08;
	P2MDOUT |= 0x01;
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
	cfg_PWM();
	
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
 * Réception d'un caractère sur l'UART (fait appel à des fonctions développées par le binome A)
 * @param void
 * @return void
 */
void UART_receive (void) {
	char invalid_cmd = 0;
	if(UART_buff == 0)
		return;


/*	
	switch (UART_buff) {
		// Mode automatique
		case 'UART_buff[0] == A':
			start_blink_led(UART_buff[1], UART_buff[2], UART_buff[3], UART_buff[4]);
			break;
		
		case 'UART_buff[0] == E' : 
			stop_blink_led();
			break;
		
		default:
			invalid_cmd = 1;
			break;
	}
			
	UART_sends(invalid_cmd ? "Invalid command !" : "OK");*/
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

void pointer_interrupt() interrupt 9{
	PCA0CN &= 0x7F;
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
	UART_sends("********** POINTEUR LUMINEUX **********");
	UART_sendCRLF();
	UART_sends("Bienvenue :)");
	UART_sendCRLF();
}


/**
 * Active le clignotement de la LED:
 * @return void
 */
void start_blink_led(int t_up, int t_down, int n_period, int intensity) {
	for (int p = 0; p < n_period; p++){
		float intensite = (-(((float)intensity * 2.56f)-256.0f));
		int pca = (int) intensite
		PCA0CPH0 = pca;
		PCA0CN |= 0x40;
		delay(t_up * 1000);
		PCA0CN |= 0x00;
		delay (t_down * 1000);
	}
}

void led_on (void){
	PCA0CPH0 = 0x00;
	PCA0CN |= 0x40;
}


/**
 * Désactive le clignotement de la LED:
 * @return void
 */
void led_off() {
	PCA0CPH0 = 0xFF;
	PCA0CN |= 0x40;
	
}


//------------------------------------------------------------------------------------
// TEST
//------------------------------------------------------------------------------------

int main() {
	init();
	Welcome();
	start_blink_led(3,3,5,50);
	led_on();
	delay(5000);
	led_off();
	while(1){
		UART_receive();
	};
}
