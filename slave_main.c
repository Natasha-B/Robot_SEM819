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
#include "SPI0.h"

char receive;

void reception_SPI() interrupt 6{
	
	receive = SPI0DAT;
	
	SPIF = 0;
}
int main() {
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	init();
	Oscillator_Init();
	cfg_Clock_UART();
	cfg_UART0_mode1();
	init_SPI0();
	EA=1;
	
	while(1){
	};
}