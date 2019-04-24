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
#include <string.h>
#include "UART0.h"

#define SYSCLK 22118400UL //approximate SYSCLK frequency in Hz
#define BAUDRATE  19200UL          // Baud rate of UART in bps
#define Preload_Timer0 (SYSCLK/(BAUDRATE*16))

// UART
char UART_buff;
char UART_busy = 0;
int fin = 0;
int fin2 = 0;
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

void cfg_Clock_UART(void){
	CKCON |= 0x10;      // T1M: Timer 1 use the system clock.
	TMOD |= 0x20;       //  Timer1 CLK = system clock
	TMOD &= 0x2f;			  // Timer1 configuré en timer 8 bit avec auto-reload	
	TF1 = 0;				  // Flag Timer effacé
	
	TH1 = -(Preload_Timer0);
	ET1 = 0;				   // Interruption Timer 1 dévalidée
	TR1 = 1;				   // Timer1 démarré

}

void cfg_UART0_mode1(void){
	RCLK0 = 0;     // Source clock Timer 1
	TCLK0 = 0;
	PCON  |= 0x80; //SMOD0: UART0 Baud Rate Doubler Disabled.
	PCON &= 0xBF;  // SSTAT0=0
	SCON0 = 0x70;   // Mode 1 - Check Stop bit - stock validée
	
	XBR0 |= 0x04;
	XBR2 |= 0x40;
	P0MDOUT |= 0x01;
	
    ES0 = 1;        // interruption UART0 autorisée	

}

void putChar0(char carac){
	SBUF0=carac;
	while((SCON0 & 0x02)== 0){}
	SCON0 &= 0xFD;
}
	
void putString0(char chaine[32]){
	int i;
	for (i=0;i<strlen(chaine);i++){
		putChar0(chaine[i]);
	}
}

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
	cfg_clk();
	cfg_interrupt();
	cfg_Clock_UART();
	cfg_UART0_mode1();
	cfg_PWM();
	
	XBR2 |= 1<<6;	//Activation du Crossbar
}



/**
 * Envoi d'un retour à la ligne à l'UART
 * @param void
 * @return void 
 */
void UART_sendCRLF (void) {
	putChar0('\r');
	putChar0('\n');
}
/**
 * Réception d'un caractère sur l'UART (fait appel à des fonctions développées par le binome A)
 * @param void
 * @return void
 */
void UART_receive (void) {
	char invalid_cmd = 0;
	fin = 0;
	if(UART_buff == 0)
		return;


	switch (UART_buff) {
		case 'b':
			putChar0('b');
			UART_buff = 0x00;
			fin = 1;
			if (fin2 == 0){
			start_blink_led(3,3,5,25);};
		//start_blink_led(1,1,2,25);
		//start_blink_led(0.5,0.5,8,100);
		//start_blink_led(0.1,1,4,10);
		//start_blink_led(0.1,1,50,10);
			break;
		
		case 'e': 
			putChar0('e');
			led_off();
			break;
		
		case 'a':
			putChar0('a');
			led_on();
			break;
		
		default:
			invalid_cmd = 1;
			break;
	}
			
	putString0(invalid_cmd ? "Invalid command !" : "OK");
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
	}}
	

void pointer_interrupt() interrupt 9{
	PCA0CN &= 0x7E;
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
	putString0("********** POINTEUR LUMINEUX **********");
	UART_sendCRLF();
	putString0("Bienvenue :)");
	UART_sendCRLF();
}

/**
 * Active le clignotement de la LED:
 * @return void
 */
int p;
void start_blink_led(int t_up, int t_down, int n_period, int intensity) {
	for (p = 0; p < n_period; p++){
		float intensite = (-(((float)intensity * 655.36f)-65536.0f));
		unsigned int pca = (int) intensite;
		int pca1 = (pca/256);
		int pca2 = (pca%256);
		PCA0CPL0 = pca2;
		PCA0CPH0 = pca1;
		PCA0CN |= 0x40;
		if (fin == 1 & fin2  ==0){
			UART_receive();}
		if (fin ==1){fin2=1; break;};
			delay(t_up * 1000);
		if (fin == 1 & fin2  ==0){
			UART_receive();}
			if (fin ==1){fin2=1; break;};
				PCA0CN = 0x00;
				delay (t_down * 1000);
		fin2 = 0;
	}
}

void led_on (){
	PCA0CPL0 = 0x00;
	PCA0CPH0 = 0x00;
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

int main() {
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	init();
	Welcome();
	while(1){
		UART_receive();
	};
}
