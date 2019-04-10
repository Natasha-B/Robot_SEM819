#include <C8051F020.H>
#include "SPI0.h"

sbit NSS = P3 ^ 0;

// delai de environ 7 us x j 
void delay_SPI(int j){
	unsigned int n, cp;
	for(n=0;n<j;n++){
		for(cp=0;cp<5;cp++){
		};
	}
}
void init_SPI0(){
	SPI0CKR = 0x31;
	
	XBR0 |= 0x02;	
	SPI0CN = 0x03;
	
}
void envoi_SPI(char* mess){
	int cpt = 0;
	while(mess[cpt] != '\0'){
		delay_SPI(2);
		NSS = 0;
		SPI0DAT = mess[cpt];
		while(SPIF == 0);
		NSS = 1;
		SPIF = 0;
		cpt ++;
	}
}
void Oscillator_Init(){
    int i = 0;
    OSCXCN    = 0x67;
    for (i = 0; i < 3000; i++);  // Wait 1ms for initialization
    while ((OSCXCN & 0x80) == 0);
    OSCICN    = 0x0C;
}
int main() {
	WDTCN = 0xDE;
	WDTCN = 0xAD;
	XBR2 |= 1<<6;
	XBR0 |= 0x04;
	P0MDOUT = 0x14;
	P3MDOUT = 0x01;
	
	Oscillator_Init();
	init_SPI0();
	
	
	envoi_SPI("bonjour\r");
	envoi_SPI("fromage\r");
	while(1){
	};
}