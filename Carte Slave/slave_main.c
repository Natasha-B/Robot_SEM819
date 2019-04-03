//------------------------------------------------------------------------------------
// slave_main.c
//------------------------------------------------------------------------------------
// DATE: 11/04/19
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
// DEVLOPPE PAR : 
//
//------------------------------------------------------------------------------------

#include "pointeur.h"
#include <c8051f020.h>
#include <string.h>
#include "UART0.h"


int main() {
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	init();
	Welcome();
	Oscillator_Init();
	cfg_Clock_UART();
	cfg_UART0_mode1();
	while(1){
		putString0("salut michel");
		UART_receive();
	};
}
