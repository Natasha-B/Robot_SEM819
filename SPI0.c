//------------------------------------------------------------------------------------
// SPI0.c
//------------------------------------------------------------------------------------
// DATE: 8/04/19
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
// DEVLOPPE PAR :  Groupe B2
//
//------------------------------------------------------------------------------------

#include <C8051F020.H>
#include "SPI0.h"


void init_SPI0(){
	SPI0CKR = 0x01;
	
	XBR0 |= 0x02;	
	SPI0CN = 0x01;
	
	EIE1 |= 0x01;
	EIP1 |= 0x01;
}