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

sbit Cmd_PdV = P1^4;
xdata char receive;
xdata char test;
xdata int cpt_SPI = 0;
xdata char stock[32] = "";
xdata char messageok = 0;	//Flag permettant d'avertir que le message est pr�t � �tre analys�

void reception_SPI() interrupt 6{
	if(SPI0DAT != '\r'){
		test = SPI0DAT;
		stock[cpt_SPI] = SPI0DAT;
		cpt_SPI++;
	}
	else{
		stock[cpt_SPI] = '\r';
		stock[cpt_SPI+1] = '\0';
		cpt_SPI = 0;
		messageok = 1;	//message pr�t pour l'analyse
	}
	SPIF = 0;
}
int main() {
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	Cmd_PdV = 0;
	init();
	Oscillator_Init();
	cfg_Clock_UART();
	cfg_UART0_mode1();
	config_PCA();
	config_servo_v();
	
	init_SPI0();
	led_off();
	EA=1;
	
	
	
	while(1){
		//Test si le message est pr�t pour l'analyse
		if (messageok == 1){
			//Lancement de l'analyse de l'instruction
			information(stock);
			//Clear le flag;
			messageok=0;
		}
	};
}