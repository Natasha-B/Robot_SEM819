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

/*  PINS

SCK -> P0.2
MISO -> P0.3
MOSI -> P0.4
NSS -> P3.0
*/
