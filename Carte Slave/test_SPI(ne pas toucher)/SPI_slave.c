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
#include "SPI_slave.h"


void init_SPI0(){
	SPI0CKR = 0x01;
	
	XBR0 |= 0x02;	
	SPI0CN = 0x01;
	
	EIE1 |= 0x01;
	EIP1 |= 0x01;
}
/*  PINS

SCK -> P0.2
MISO -> P0.3
MOSI -> P0.4
NSS -> P0.5

faut peut-être mettre NSS en PP mais sa marche sans
*/