//------------------------------------------------------------------------------------
// Projet_BSE.c
//------------------------------------------------------------------------------------
// DATE: 19/12/18
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS: Albert, Avinain, Bru, Jannin
//
// Num�ro du sujet: 1
// Bin�me : B (Avinain, Bru)
//
//------------------------------------------------------------------------------------
#include "c8051F020.h"
#include "Binome_A.h"
#include "Binome_B.h"

// UART
char UART_buff;
char UART_busy = 0;

// TIMER2
sfr16 T2 = 0xcc;
sfr16 RCAP2 = 0xCA;

// CAN
sfr16 ADC0 = 0xBE;
float CSG_min = 0.0f;
float CSG_max = 30.0f;

//------------------------------------------------------------------------------------
// CONFIGURATION
//------------------------------------------------------------------------------------


/**
 * Configuration de la fonction Delay attente de x ms
 * @param int j, j est le nobte de ms � attendre
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
 * Configuration de l'horloge externe � 22,1184 MHz, pour assurer une bonne transmission/reception s�rie
 * @param void
 * @return void
 * Registres modifi�s : OSCICN , OSCXCN
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
 * Registres modifi�s : T2CON, CKCON , RCAP2, IE
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
	
	// D�marrage TIMER2
	TR2 = 1;
}

/**
 * Configuration de l'UART
 * @param void
 * @return void
 * Registres modifi�s : SCON0, P0MDOUT, XBR2, XBR0
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
 * Registres modifi�s : IE
 */
void cfg_interrupt (void) {
	EA = 1; // enable all interupts
}

/**
 * Configuration du convertisseur analogique/num�rique
 * @param void
 * @return void
 * Registres modifi�s : ADC0CN, AMX0CF, AMX0SL, REF0CN
 */
void cfg_CAN (void) {
	REF0CN = 0x03;
	AMX0CF = 0x0;
	AMX0SL = 0x0; // AIN0 par d�faut
	AD0CM0 = 0; //conversion pour AD0BUSY = 1
	AD0CM1 = 0; 
	AD0TM = 0;
	AD0LJST = 0; //alignement � droite
	AD0EN = 1; // activation du  CAN
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
	cfg_CAN();
	cfg_interrupt();
	
	XBR2 |= 1<<6;	//Activation du Crossbar
	
	cfg_LED();
	cfg_timer3();
	cfg_timer4();
	cfg_servo();
	
	EIE2 = 0x05;  // Activation des interruptions Timers 3 et 4
	EIP2 = 0x03;  // Priorit� haute Timers 3 et 4
}

/**
 * Conversion int to string
 * @param int v , valeur � convertir
 * @return string str2, chaine de caract�res associ�e
 */
char *itos(unsigned int v) {
	char str1[20] = "", str2[20] = "";
	int unite = 0;
	int indice = 0, indice2 = 0;
	while(v != 0){
		unite = v%10; // Unit�
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
 * Envoi d'un caract�re � l'UART
 * @param char c , caract�re � envoyer
 * @return void
 * Registres modifi�s : SBUF0
 */
void UART_send (char c) {
	while(UART_busy == 1) {}
	UART_busy = 1;
	SBUF0 = c;
}

/**
 * Envoi d'une chaine de caract�res � l'UART
 * @param char* s , chaine de caract�res � envoyer
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
 * Envoi d'un retour � la ligne � l'UART
 * @param void
 * @return void 
 */
void UART_sendCRLF (void) {
	UART_send(0x0D);
	UART_send(0x0A);
}

/**
 * Envoi d'un entier � l'UART
 * @param char* prefix, pr�fixe du message
 * @param int a, entier � envoyer
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
 * R�ception d'un caract�re sur l'UART (fait appel � des fonctions d�velopp�es par le binome A)
 * @param void
 * @return void
 */
void UART_receive (void) {
	char invalid_cmd = 0;
	if(UART_buff == 0x00)
		return;
	
	switch (UART_buff) {
		// Mode automatique
		case 'a':
			auto_mode();
			break;
		
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
		
		// Lecture de la distance MIN
		case 'L':
			UART_sendi("Distance min: ", get_CSG_min(), "cm");
			break;
		
		// Lecture de la distance MAX
		case 'H':
			UART_sendi("Distance max: ", get_CSG_max(), "cm");
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
 * Registres modifi�s : SCON0, SBUF0
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


/**
 * Lecture de consigne distance min
 * @param void
 * @return int CSG min
 * Registres modifi�s : AMX0SL, AD0INT, AD0BUSY
 */
int get_CSG_min(void) {
	AMX0SL = 0x01; // AIN1
	AD0INT = 0;
	AD0BUSY = 1;
	while(AD0INT != 1);
	
	CSG_min = (50.0f*((ADC0*2.75f)/4095.0f));
	return CSG_min;
}

/**
 * Lecture de consigne distance max
 * @param void
 * @return int CSG max
 * Registres modifi�s : AMX0SL, AD0INT, AD0BUSY
 */
int get_CSG_max(void) {
	AMX0SL = 0x02; // AIN2
	AD0INT = 0;
	AD0BUSY = 1;
	while(AD0INT != 1);
	
	CSG_max = (50.0f*((ADC0*2.75f)/4095.0f));
	return CSG_max;
}

/**
 * Activation du mode "Balayage automatique"
 * @param void
 * @return void
 * Registres modifi�s : 
 */
void auto_mode() {
	float angle = -90.0f;
	
	UART_sends("AUTOMATIC MODE ACTIVATED!");
	UART_sendCRLF();
	
	while(angle <= 90.0f) {
		run(angle);
		angle += 10.0f;
	}
}

/**
 * Affichage d'un diagramme de positionnement
 * @param void
 * @return void
 * Registres modifi�s : 
 */
void show_diagram(float d) {
	int pos;
	char diag[22] = " |                  | ";
	
	// Trop pr�s
	if(d < CSG_min) {
		diag[0] = 'X';
		
	// Trop loin
	} else if(d > CSG_max) {
		diag[21] = 'X';
		
	// Dans l'intervalle
	} else {
		pos = (int) (20.0f * (d - CSG_min) / (CSG_max - CSG_min));
		diag[pos+1] = 'v';
	}
	
	UART_sends(diag);
	UART_sendCRLF();
}

//------------------------------------------------------------------------------------
// APPLICATION
//------------------------------------------------------------------------------------

/**
 * Envoi du message de d�marrage
 * @param void
 * @return void
 */
void Welcome() {
	UART_sendCRLF();
	UART_sendCRLF();
	UART_sends("********** CONTROLE DE DISTANCE **********");
	UART_sendCRLF();
	UART_sends("Bienvenue :)");
	UART_sendCRLF();
}

/**
 * Effectue une mesure � l'angle de rotation actuel
 * @param void
 * @return void
 */
float mesure (int angle) {
	float V, d;
	
	AMX0SL = 0x0; // AIN0
	AD0INT = 0;
	AD0BUSY = 1;
	while(AD0INT != 1);
	
	V = ((ADC0*2.75f)/4095.0f);
	if(V < 2.0f)
		d = (1.0f/(0.0169f*V-0.0008f));
	else
		d = (1.0f/(0.0382f*V-0.0454f));
	
	if(angle >= 0)
		UART_sendi("Mesure (", angle, "deg): ");
	else
		UART_sendi("Mesure (-", -angle, "deg): ");
	UART_sendi("", (int) d, "cm");
	show_diagram(d);
	return d;
}

/**
 * Effectue une mesure � un angle donn�
 * @param int angle, angle de mesure (en degr�s)
 * @return void
 */
void run (int angle) {
	float d;
	
	// Lecture des consignes
	get_CSG_min();
	get_CSG_max();
	
	// Rotation � l'angle souhait�
	servo_pos(angle);
	
	// R�alisation de la mesure
	d = mesure(angle);
	
	// V�rification
	if((d >= CSG_min) && (d <= CSG_max))
		start_blink_led();
	else
		stop_blink_led();
}
