#include <c8051F020.h> 
#include "servo_V.h"
#include "UART0.h"

sbit Cde_Servo_V = P1^3;
//long SYSCLK = 22118400;  // Fr?quence de l'horloge externe 
/**/
xdata int cpt_servo_V = 0;
xdata float t_pulse_v; 
xdata unsigned int timer_count_v; 
xdata unsigned int timer_load_v;
xdata float t_pos_min_v = 0.5;  // Dur?e de l'impulsion en ms pour -90? 
xdata float t_pos_max_v = 2.4;  // Dur?e de l'impulsion en ms pour 90? 
xdata float alpha_v;  // Dur?e d'impulsion par d?gr? d'angle 
//int  position= 0;
unsigned int timer_loade;
/*
void delay(int j){ 
 	xdata unsigned int n, cp; 
 	for(n=0;n<j;n++){ 
 		for(cp=0;cp<2500;cp++){ 
 		}; 
 	} 
 }
 */
//Configuration initiale du PCA
void config_PCA(){
	//Configuration de la clock du PCA
	PCA0MD = 0x09;
	//Configuration de la comparaison (+autorisation interruption quand comparaison ok
	PCA0CPM1 = 0x49;
	//Activation des interruptions
	EIE1 |= 0x08;
}

  
/*
void cfg_clk_ext(void){
	//Enable external oscillator with f=22MHz
	OSCXCN = 0x67;

	delay(1);

	OSCICN = 0x08;	
}
*/
void config_servo_v(){
	xdata int temporaire= -90;
	P1MDOUT |= 0x08;
	alpha_v = (t_pos_max_v - t_pos_min_v) / 180;
	chg_servo_pos_v(temporaire);
	PCA0CN |= 0x40;
}

//Interruption liée au PCA
void interPCA() interrupt 9 {
	if (PCA0CN >= 0x80){
		PCA0CN &=0x7F; //Clear le flag 
		cpt_servo_V+=1;
		if (cpt_servo_V == 12){
			Cde_Servo_V = 1; // Activation de la commande
			cpt_servo_V = 0;
		}
	} else if (PCA0CN && 0x02 == 0x02){
		PCA0CN &= 0xFD;	//Clear le flag
		Cde_Servo_V = 0; //Desactivation de la commande
	}
	else if (PCA0CN && 0x01 == 0x01){
		PCA0CN &= 0xFE;
	}
}



unsigned int pos2timer_count_v(int pos){   // int pos
	if (pos >= 70){
		pos = 180;  // pos = [0?, 180?] 
	} else {
		pos+=110;
	}
	
// Dur?e de l'impulsion: 
	 t_pulse_v = (t_pos_min_v + (pos * alpha_v))*12; 

// Nombre de co?t d'horloge pour g?n?r? l'impulsion: 
	timer_count_v = t_pulse_v* 0.001 * (SYSCLK/12); 

// Valeur de rechargement du Timer 4: 
 	timer_load_v = 0xFFFF - timer_count_v; 

	return timer_load_v; 
 }  


//Modification de la position du servomoteur vertical
void chg_servo_pos_v(int pos){
	timer_loade = 0xFFFF-pos2timer_count_v(pos);

	Cde_Servo_V = 1;

	//PCA0CN &= 0xBF; //Désactivation du PCA
	//Détermination de la valeur à charger pour la comparaison du PCA
	
	//Changement de la valeur de référence
	PCA0CPL1 = timer_loade%256;
	PCA0CPH1 = timer_loade>>8;
	
	//Activation du PCA
	PCA0CN |= 0x40;
	
	//Activation de la sortie
	Cde_Servo_V = 1;
}

/*
void main(){
	
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	XBR2 |= 0x40;
	EA = 1;
	config_servo_v();
	cfg_clk_ext();
	config_PCA();
	
	chg_servo_pos_v(position);
	while(1);
	
*/