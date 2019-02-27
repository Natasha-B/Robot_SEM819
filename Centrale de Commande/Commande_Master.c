//------------------------------------------------------------------------------------
// PROJET TRANSVERSALE
//------------------------------------------------------------------------------------
// DATE: 
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS: TRANSVERSALE
//
//------------------------------------------------------------------------------------

#include "c8051F020.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

sbit ESO  =  IE^4; //Enable flag du UART
char reception = '';
char envoie = '';
char TVvitesse = "20";
char vitesse = '';
int i =0;

/*
			Interruptions RECEPTION UART0 : Fonction d'interruption
	*/
void Carac1() interrupt  4{
	if(SBUF0 != '\r'){
		if(SBUF0 == 'Q'){																	// ARRET D'URGENCE
			envoie = "reset\r";
			TransmissionUART1(envoie);
			}
		else{
			str.append(reception,SBUF0);
		}
	}
	else{
		test();
		reception = '';
		
	}
	RI0 = 0;
}

/*
			Interruptions TRANSMISSION UART1 : Fonction d'interruption
	*/
void TransmissionUART1 (char* transmit) {
	for(i = 0; i<=length(transmit);i++){
		SBUF1 = transmit[i];
		while(TI1 == 0){
			}
		TI1 = 0;
		}
	envoie = '';
}

/*
			FONCTION de test : on verifie ce que l'on a envoyé dans la liaison UART0 pour savoir quoi renvoyer dans la liaison UART1
	*/
void test(){
/*	if(reception[0]=='D'){																// Début Epreuve
			
		}
	if(reception[0]=='E'){																	// Fin Epreuve
		}*/
	if(reception[1]=='V')&(reception[0]='T'){								// TV vitesse
		TVvitesse = '';
		for (i=3;i<=length(reception);i++){
			str.append(TVvitesse,reception[i]);
			}
		}
	if(reception[0]=='A'){																	// Avancer	
		if (length(reception)==1){
			envoie = strcat(strcat(strcat(strcat("mogo 1:",TVvitesse)," 2:"),TVvitesse),"\r");
			}
		else{
			vitesse = '';
			for (i = 2;i<length(reception);i++){
					str.append(vitesse,reception[i]);
				}
			envoie = strcat(strcat(strcat(strcat("mogo 1:",vitesse)," 2:"),vitesse),"\r");
			}
		}
	if(reception[0]=='B'){																	// Reculer
		if (length(reception)==1){
			envoie = strcat(strcat(strcat(strcat("mogo 1:-",TVvitesse)," 2:-"),TVvitesse),"\r");
			}
		else{
			vitesse = '';
			for (i = 2;i<length(reception);i++){
					str.append(vitesse,reception[i]);
				}
			envoie = strcat(strcat(strcat(strcat("mogo 1:-",vitesse)," 2:-"),vitesse),"\r");
			}
		}
	if(reception[0]=='S'){																	// STOP
		envoie = "stop\r";
		}
	}
	if(reception[1]=='D')&(reception[0]='R'){						// Rotation Droite 90°
		envoie = "mogo 1: 2:0\r");
		}
/*	if(SBUF0=='G')&(last_letter='R'){						// Rotation Gauche 90°
		}	
	if(SBUF0=='C')&(last_letter='R'){						// Rotation Complete 180°
		}	
	if(SBUF0=='A')&(last_letter='R'){						// Rotation angle donné
		}	
	if(SBUF0=='G'){						// Deplacement jusqu'a une coordonnee
		}
	if(SBUF0=='S')&(last_letter='A'){						// Acquisition signaux sonores
		}	
	if(SBUF0=='I')&(last_letter='M'){						// Mesure courant
		}	
	if(SBUF0=='E')&(last_letter='M'){						// Mesure energie
		}
	if(SBUF0=='P')&(last_letter='I'){						// Rotation angle donné
		}	
	*/
	TransmissionUART1 (envoie);
}

void main (void) 
{

	 XBR2 |= 0x40; // Enable crossbar
	 XBR0 |= 0x04; // Enable UART0
	 XBR1 |=0x80;  // Enable SYSCLK
	
   WDTCN = 0xDE;   // Dévalidation du watchdog 
   WDTCN = 0xAD;
	
   /*
			CONFIGURATION UART-SERIAL
			*/
	 
	 EA=1; 						// Enable interrupt
	
	 OSCXCN = 0xE7;		//	COnfiguration Clock pour le baud-rate
	 OSCICN |= 0x08; 	//	Enable external clock
	 
	 T2CON |= 0x34; 	//	Config Timer 2 pour le Baud-rate UART0 UART1
	 RCAP2L = 0xDC; RCAP2H = 0xFF;
	 
	 SCON0 |= 0x70; 	//	Config UART0
	 SCON1 |= 0x70; 	//	Config UART1
	 
	 ESO = 1; 				//	Flag UART0 activé
	 ES1 = 1; 				//	Flag UART0 activé
	 
	 PCON  &= 0x77;		// 	on active le Baud-rate UART0 UART1 en divided by two
	 
	 RI0 = 0; 				// 	Flags de reception
	 RI1 = 0;
	
	
	while(1) 
        {

         }   
	               	
}