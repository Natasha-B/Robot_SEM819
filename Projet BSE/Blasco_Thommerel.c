//------------------------------------------------------------------------------------
// TP4_BSE.c
//------------------------------------------------------------------------------------
// DATE: Blasco Thommerel
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS:
//
// Num?ro du sujet:
//
//------------------------------------------------------------------------------------
#include "c8051F020.h"
sfr Reg    =	 0xFF;
sbit LED   =   P1^6;
sbit ESO  =  IE^4;
sbit BP = P3^7;
int cp_led = 0x0;
int cpt =0;
int copy=0;
int dizaine = 0;
int centaine = 0;

//------------------------------------------------------------------------------------
// MAIN Routine
//------------------------------------------------------------------------------------

void demarrage(){
			unsigned int k;
			for(k=0;k<6;k++){
				TI0=1;
				SBUF0 = 0x4B;
				TI0 = 0;
				SBUF0 = 0x4B;
				while( TI0 == 0){
				}
				TI0 = 0;
		}
	
}
void action() interrupt 19{
		TI0 = 0;
		SBUF0=0x41;
		while( TI0 == 0){
				}
		TI0 = 0;
		SBUF0=0x63;
		while( TI0 == 0){
				}
		TI0 = 0;
		SBUF0=0x74;
		while( TI0 == 0){
				}
		TI0 = 0;
		SBUF0=0x69;
		while( TI0 == 0){
				}
		TI0 = 0;
		SBUF0=0x6F;
		while( TI0 == 0){
				}
		TI0 = 0;
		SBUF0=0x6E;
		while( TI0 == 0){
				}
		TI0 = 0;
		SBUF0=0x20;
		while( TI0 == 0){
				}
		TI0 = 0;
		SBUF0=0x21;
		while( TI0 == 0){
				}
		TI0 = 0;
	
	P3IF &= 0x7F;
}
void LED_cligno() interrupt  16{
		if (cpt<4){
			T4CON &=0x7F;
			cpt+=1;
		}
		else{
		if(LED ==0){
			LED=1;
			cp_led+=1;
		}
		else{
			LED = 0;
			cp_led+=1;
		}
		//3HZ
		RCAP4L = 0x00;
		RCAP4H = 0x00;
	  T4CON &=0x7F;
		cpt=0;
		
	}
}
void echo(){
	  TI0 = 0;
		SBUF0=copy;
		while( TI0 == 0){
				}
		TI0 = 0;
		RI0=0;
	
}
void Carac() interrupt  4{
	  copy=SBUF0;
		if(SBUF0==0x4E){
			LED=1;
			T4CON &=0xFB;
			echo();
		}
		if(SBUF0==0x4C){
			LED=0;
			T4CON &=0xFB;
			echo();
		}
		if(SBUF0==0x4D){
			T4CON |= 0x0C; //Config Timer 4
			echo();
		}
		if(SBUF0==0x31){
			TI0 = 0;
			centaine = (cp_led-cp_led%100)/100;
			cp_led = cp_led -centaine*100;
			dizaine = (cp_led - cp_led%10)/10;
			cp_led = cp_led -dizaine*10;
			SBUF0=centaine+30;
			while( TI0 == 0){
					}
			TI0 = 0;
			RI0=0;
			SBUF0=dizaine+30;
			while( TI0 == 0){
					}
			TI0 = 0;
			RI0=0;
			
			SBUF0=cp_led+30;
			while( TI0 == 0){
					}
			TI0 = 0;
			RI0=0;
				}
		
		
}


void main (void) 
{
   WDTCN = 0xDE;   // D?validation du watchdog 
   WDTCN = 0xAD;

			// Ins?rer les diff?rentes fonctions de configuration
			XBR2 |= 0x40; // Enable crossbar
			XBR0 |= 0x04; // Enable UART0
			XBR1 |=0x80; // Enable SYSCLK
			OSCXCN = 0xE7;// COnfiguration Clock
			OSCICN |= 0x08; //Enable external clock
	
		  P0MDOUT = P0MDOUT | 0x07; // Activation P0.1
	
			//Setting baud rate
		  RCAP2L = 0xB8;
		  RCAP2H = 0xFF;
	
			T2CON |= 0x34; //Config Timer 2
	
		  SCON0 |= 0x70; //Config UART0
		  PCON  &= 0x7F;
	
			EA=0;
			demarrage();
			
			//3HZ
		  RCAP4L = 0x00;
		  RCAP4H = 0x00;
			
			
			EA=1; // Enable interrupt
			EIE2 |= 0x04; // Enable T4 int
			T4CON |= 0x0C; //Config Timer 4
			
			P1MDOUT = P1MDOUT | 0x40; // Activation LED
			LED = 1;
			
			ESO =1; //Active int UART0
			RI0 = 0; // Reception
			BP=1;
			
			EIE2 |= 0x20; // Enable int BP
			P3IF |= 0x08; // Rising edge
			
	while(1) 
        {
            // vous pouvez ins?rer du code ici
					 
					
					
					
         }   
	               	
}