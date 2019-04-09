#include <c8051F020.h> 
#include "servo_V.h"

sbit Cde_Servo_V = P1^3;
long SYSCLK = 22118400;  // Fr?quence de l'horloge externe 
/**/
float t_pulse; 
unsigned int timer_count; 
unsigned int timer_load;
xdata float t_pos_min = 0.5;  // Dur?e de l'impulsion en ms pour -90? 
xdata float t_pos_max = 2.4;  // Dur?e de l'impulsion en ms pour 90? 
xdata float alpha;  // Dur?e d'impulsion par d?gr? d'angle 

/**/
void delay(int j){ 
 	xdata unsigned int n, cp; 
 	for(n=0;n<j;n++){ 
 		for(cp=0;cp<2500;cp++){ 
 		}; 
 	} 
 }
 
//Configuration initiale du PCA
void config_PCA(){
	//Configuration de la clock du PCA
	PCA0MD = 0x09;
	//Configuration de la comparaison (+autorisation interruption quand comparaison ok
	PCA0CPM0 = 0x49;
	//Activation des interruptions
	EIE1 |= 0x08;
}

  
/**/
void cfg_clk_ext(void){
	//Enable external oscillator with f=22MHz
	OSCXCN = 0x67;

	delay(1);

	OSCICN = 0x08;	
}

//Interruption liée au PCA
void interPCA() interrupt 9 {
	if (PCA0CN >= 0x80){
		PCA0CN &=0x7F; //Clear le flag 
 		Cde_Servo_V = 1; // Activation de la commande
	} else if (PCA0CN & 0x01 == 0x01){
		PCA0CN &= 0xFE;	//Clear le flag
		Cde_Servo_V = 0; //Desactivation de la commande
	}
}


/**/
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


//Modification de la position du servomoteur vertical
void chg_servo_pos_v(int pos){
	unsigned int timer_loade = 0xFF-pos2timer_count(pos);

	Cde_Servo_V = 1;
	PCA0CN &= 0xBF; //Désactivation du PCA
	//Détermination de la valeur à charger pour la comparaison du PCA
	
	//Changement de la valeur de référence
	PCA0CPL0 = timer_loade%256;
	PCA0CPH0 = timer_loade>>4;
	
	//Activation du PCA
	PCA0CN |= 0x40;
	
	//Activation de la sortie
	Cde_Servo_V = 1;
}

/**/
void main(){
	int position = 0;
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	XBR2 |= 0x40;
	EA = 1;
	P1MDOUT = 0xFF;
	cfg_clk_ext();
	config_PCA();
	
	chg_servo_pos_v(position);
	while(1);
	
}