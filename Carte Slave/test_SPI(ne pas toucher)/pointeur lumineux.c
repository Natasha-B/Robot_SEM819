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
#include "pointeur.h"


/*// UART
char UART_buff;
char UART_busy = 0;
*/


//------------------------------------------------------------------------------------
// CONFIGURATION
//------------------------------------------------------------------------------------


/**
 * Configuration de la fonction Delay attente de x ms
 * @param int j, j est le nobte de ms à attendre
 * @return void
 */

void delay(int j){
	xdata unsigned int n, cp;
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

/*
void cfg_clk (void){
	// Enable the external oscillator
	OSCXCN = 0x67;

	// Wait at least 1ms
	delay(2);

	// Switch the system clk to the external oscillator
	OSCICN = 0x1c;
}
*/


/**
 * Configuration du Timer 2 pour la gestion de l'UART
 * @param void
 * @return void
 * Registres modifiés : T2CON, CKCON , RCAP2, IE
 */
 /*
void cfg_timer2 (void) {
	RCLK0 = 1;
	TCLK0 = 1;
	CPRL2 = 0;
	CKCON |= 0x20;
	RCAP2L = 0xB8; // 9600b
	RCAP2H = 0xFF;
	
	// Interruptions
	ES0 = 1;
	ET2 = 0x00; // Overflow
	
	// Démarrage TIMER2
	TR2 = 1;
}
*/

/**
 * Configuration de l'UART
 * @param void
 * @return void
 * Registres modifiés : SCON0, P0MDOUT, XBR2, XBR0
 */
 /*
void cfg_UART (void) {
	SCON0 = 0x50;
	P0MDOUT = 0xFF;
	XBR2 |= 0x40;
	XBR0 |= 0x04;
}
	*/
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
	PCA0CN = 0x00;
	PCA0MD |= 0x01;
	PCA0CPM0 |= 0xc3;
	XBR0 |= 0x08;
	P0MDOUT |= 0xff;
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
	//cfg_clk();
	//cfg_UART();
	//cfg_timer2();
	cfg_interrupt();
	cfg_PWM();
	
	XBR2 |= 1<<6;	//Activation du Crossbar
}


/**
 * Conversion int to string
 * @param int v , valeur à convertir
 * @return string str2, chaine de caractères associée
 */
/*
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
 /*
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
 /*
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
 /*
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
 /*
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
/* 
void UART_receive (void) {
	char invalid_cmd = 0;
	if(UART_buff == 0)
		return;


	switch (UART_buff) {
		case 'C':
			UART_sends('C');
			start_blink_led(3,3,5,25);
			break;
		
		case 'E': 
			led_off();
			break;
		
		case 'A':
			led_on();
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
 /*
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
*/


//TODO A decommenter si fonction seule
/*
void pointer_interrupt() interrupt 9{
	PCA0CN &= 0x7E;
} 
*/

//------------------------------------------------------------------------------------
// APPLICATION
//------------------------------------------------------------------------------------

/**
 * Active le clignotement de la LED:
 * @return void
 */
xdata int p;
void start_blink_led(int t_up, int t_down, int n_period, int intensity) {
	for (p = 0; p < n_period; p++){
		xdata float intensite = (-(((float)intensity * 655.36f)-65536.0f));
		xdata unsigned int pca = (int) intensite;
		xdata int pca1 = (pca/256);
		xdata int pca2 = (pca%256);
		PCA0CPL0 = pca2;
		PCA0CPH0 = pca1;
		PCA0CN |= 0x40;
		delay(t_up * 1000);
		PCA0CN = 0x00;
		delay (t_down * 1000);
	}
}

void led_on (void){
	PCA0CPL0 = 0x00;
	PCA0CPH0 = 0x70;
	PCA0CN |= 0x40;
}


/**
 * Désactive le clignotement de la LED:
 * @return void
 */
void led_off() {
	PCA0CPL0 = 0xFF;
	PCA0CPH0 = 0xFF;
	PCA0CN |= 0x40;
	
}


//------------------------------------------------------------------------------------
// TEST
//------------------------------------------------------------------------------------


