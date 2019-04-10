// Groupe: B2
//
//------------------------------------------------------------------------------------
#include <c8051F020.h> 
#include "mesure_courant.h"
#include "CFG_Globale.h"

sfr16 ADC0 = 0xBE;

//VREF = 2.43V
xdata char autorisation = 0;
xdata int convert;
xdata int tension;
xdata int courant_conso;
//int Rshunt = 50; //mohm
//int Gain_ampli = 20; // chosir un gain pour avoir au max 2.43V

/*
void delay3_10u(int j){ 
 xdata unsigned int n, cp; 
 for(n=0;n<j;n++){ 
 	for(cp=0;cp<20;cp++){ 
 	}; 
 } 
} 
*/
/*
void cfg_clk_ext_3(){
	//Enable external oscillator with f=22MHz
	OSCXCN = 0x67;
	delay2_10u(100);
	OSCICN = 0x08;	
}
*/

void config_adc0(){
	AMX0CF &= 0xFE; // AIN0 single ended input
	AMX0SL &= 0xF0;
	ADC0CF &= 0x08; // clk conversion 2.5MHz max et gain =1
	AD0EN = 1; // activation ADC0
	REF0CN = 0x03; // voltage ref 2.43V
	EIE2 |= 0x02; // enable interrupt ADC0
}

void config_timer3(){
	TMR3CN = 0x06; // enable timer3 and uses clock 22MHz
	TMR3RLL = 0x00;
	TMR3RLH = 0x00; // timer compte jusqu'à 2^16 soit un temps de 3ms
	EIE2 |= 0x01; // enable interrupt timer3
}

void interrupt_timer3(void) interrupt 14{
	TMR3CN &=0x7F;
}

void interupt_ADC0(void) interrupt 15{  // Mesure du courant toute les minutes 3 fois
	xdata int j =0;
	xdata int i=-1;
	AD0INT = 0;
	i++;
	if(i == 0){
		convert = ADC0;
		tension = convert*2430UL/4096UL; // en mV et attention a la plage de mesure et precision
		courant_conso = tension*1000UL / (50UL*20UL);
		ADC0 = 0x00;
	}
	if(j<4){
		if (i == 20000){ // on attent d'arriver à 60s et on repete 3 fois
			j++;
			convert = ADC0;
			tension = convert*2430UL/4096UL; // en mV et attention a la plage de mesure et precision
			courant_conso = tension*1000UL / (50UL*20UL);
			ADC0 = 0x00;
			i=0;
		}
	}
	autorisation = 0; // fin des mesures au bout de 3min
}


int conversion_adc0(char autorisation2, char mesure_continue){
	if(autorisation2 == 1){
		autorisation = autorisation2;
		if(mesure_continue ==0){ // Message MI
			ADC0CN &= 0xF3; // conversion initiated with AD0BUSY
			AD0BUSY = 1;
			while(AD0INT == 0){
			}
			convert = ADC0;
			tension = convert*2430UL/4096UL; // en mV et attention a la plage de mesure et precision
			courant_conso = tension*1000UL / (50UL*20UL);  //Rshunt = 50mohm et gain de 20
			AD0BUSY = 0;
			AD0INT = 0;
			ADC0 = 0x00;
		}
		else if (mesure_continue ==1){				// Message ME
			mesure_continue =0;
			AD0CM1 =0; // conversion initiated on rising edge on overflow timer 3	
			AD0CM0 =1;		
		}		
	}
return (courant_conso);
}


void config_courant(){
	//EA =1;
	//cfg_clk_ext();
	config_timer3();
	config_adc0();
}


/*void main(){
	//desactivation du watchdog
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	config_courant();
	autorisation =1;
	mesure_continue = 0;
	
	
	while(1){
		conversion_adc0();
	}
	
}*/






