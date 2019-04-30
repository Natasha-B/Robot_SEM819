//------------------------------------------------------------------------------------
// slave_main.c
//------------------------------------------------------------------------------------
// DATE: 11/04/19
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
// DEVLOPPE PAR :  Groupe B2
//
//------------------------------------------------------------------------------------

#include <C8051F020.H>
#include <string.h>
#include "pointeur.h"
#include "UART0.h"
#include "SPI_slave.h"
#include "information.h"
#include "servo_V.h"

xdata char receive;
xdata int cpt_SPI = 0;
xdata char stock[32] = "";
xdata char messageok = 0;	//Flag permettant d'avertir que le message est prêt à être analysé

void reception_SPI() interrupt 6{
	if(SPI0DAT != '\r'){
		stock[cpt_SPI] = SPI0DAT;
		cpt_SPI++;
	}
	else{
		stock[cpt_SPI] = '\0';
		cpt_SPI = 0;
		messageok = 1;	//message prêt pour l'analyse
	}
	SPIF = 0;
}
int main() {
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	init();
	Oscillator_Init();
	cfg_Clock_UART();
	cfg_UART0_mode1();
	config_servo_v();
	config_PCA();
	init_SPI0();
	EA=1;
	
	while(1){
		//Test si le message est prêt pour l'analyse
		if (messageok == 1){
			//Lancement de l'analyse de l'instruction
			information(stock);
			//Clear le flag;
			messageok=0;
		}
	};
}