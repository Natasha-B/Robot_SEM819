//------------------------------------------------------------------------------------
// Projet_Transversal
//------------------------------------------------------------------------------------
// DATE: 27/02/19
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS: Blasco, Bru, Renotton
//
// Groupe: B2
//
//------------------------------------------------------------------------------------
#include "c8051F020.h"
#include "Communication.h"


//------------------------------------------------------------------------------------
// MAIN Routine
//------------------------------------------------------------------------------------
void main (void) 
{
	// Dévalidation du watchdog 
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	
	init();
	
	Welcome();
	
	while(1) {
		UART_receive();
	}         	
}