// Groupe: B2
//
//------------------------------------------------------------------------------------
#include <c8051F020.h> 
  
 
sbit signal_entre = P1^1;
long sysclk = 22118400;  // Fréquence de l'horloge externe 
int front;
int temps;
float distance;
int test = 1;
// delay de 10x us
void delay_10u(int j){ 
 unsigned int n, cp; 
 for(n=0;n<j;n++){ 
 	for(cp=0;cp<20;cp++){ 
 	}; 
 } 
} 

//void cfg_clk_ext_2(){
//	//Enable external oscillator with f=22MHz
//	OSCXCN = 0x67;
//	
//	delay_10u(100);
//	 
//	OSCICN = 0x08;	
//}

//------------------------------------------------------------------------------------ 
 // Configurations 
 //------------------------------------------------------------------------------------ 
 
//void cfg_crossbar_2(){
//	XBR2 = 0x40; 
//}

 void conf_port_sortie(){ 	 
// Active la sortie en push-pull: 
 	P1MDOUT = 0xFF; 
 } 
 
 void activation_interrupt_6(){ 
	 P3IF |= 0x04; // active en front montant
	 front = 1;
	 EIE2 |= 0x10; // activer interruption
 }
 
 void cfg_timer0(){
	 TH0 = 0x00;
	 TL0 = 0x00;
	 TR0 = 0; // desactivation timer
	 TMOD |= 0x01; // mode 1 timer0
	 TMOD &= 0xF1;
	 CKCON &= 0xF7; // sysclk divided by 12
 }
 
 void config_telem_ultrason(){
	EA = 1;
	signal_entre = 0;
//	cfg_clk_ext_2();
//	cfg_crossbar_2();
	conf_port_sortie();
	cfg_timer0();
	activation_interrupt_6();
}	
 
void int6() interrupt 18 {
	P3IF &= 0xBF; // flag a 0
	if( front == 1){
		front =0;
		P3IF &= 0xFB; // active en front descendant
		TR0 = 1; // activation timer0
	}
	else{
		TR0 = 0;
		front = 1;
		P3IF |= 0x04; // active en front montant
		temps = (TH0*256)+TL0;
		
		
		TH0 = 0;
		TL0 = 0;		
	}
}


 
float calc_dist(){
	 int j = 0;
	 int i=0;
	 float distance;
	 while(j<8){
		if (i<3){
			signal_entre = 1;
			delay_10u(1);
			signal_entre = 0;
			delay_10u(1000);
			i++;
	
		}
		if(temps != 0){
			distance = ((temps*12.0)/sysclk)*34000.0/2.0; // distance en cm
			temps = 0;
		}	
	j++;}
	return distance;
}
 
 
 
 
 
 /*
void main(){
	int i=0;
	int j = 0;
	float d;
	//desactivation du watchdog
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	signal_entre = 0;
	config();
	
	d = calc_dist();
	
	while(1){
	}
}
	 
/*
if ((stock[0] == "M")&&(stock[1] == "O") && (stock[2] == "U"){
	d = calc_dist();
	itoa(d,distance,10);
	if ((d<10) || (d>105)){	//Pas d'obstacles
		serOutstring(strcat(angle,": 0"));
	} else {	//Obstacle detecte
		serOutstring(strcat(strcat(angle,":"),distance));
	}
}
*/


















