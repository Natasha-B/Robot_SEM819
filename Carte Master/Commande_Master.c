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
int i = 0;
char AngleRotation = '90';
int Longueur;
int r = 11.2;
char distance;
char TableauCaract[200][30];


/**
 * Conversion int to string
 * @param int v , valeur à convertir
 * @return string str2, chaine de caractères associée
 */
char *itos(unsigned int v) {
	char str1[20] = "", str2[20] = "";
	int unite = 0;
	int indice = 0, indice2 = 0;
	while(v != 0){
		unite = v%10; // Unité
		str1[indice] = 0x30 + unite;
		indice++;
		v = (v - unite)/10;
	}
	
	if(indice == 0) {
		indice = 1;
		str1[0] = '0';
	}
	
	// Reverse de string
	for(;indice > 0;indice--) {
		str2[indice-1] = str1[indice2];
		indice2++;
	}
	str2[indice2+1] = '\0';
	
	return str2;
}
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
	if(reception[1]=='D')&(reception[0]='R'){								// Rotation Droite 90°
		envoie = "digo 1:346:20 2:346:-20\r";
		}
	if(reception[1]=='G')&(reception[0]='R'){								// Rotation Gauche 90°
		envoie = "digo 1:346:-20 2:346:20\r"
		}	
	if(reception[1]=='C')&(reception[0]='R'){								// Rotation Complete 180°
		if (reception[3]=='D'){
			envoie = "digo 1:692:-20 2:692:20\r"
			}
		else{
			envoie = "digo 1:692:20 2:692:-20\r"
			}
		}	
	if(reception[1]=='A')&(reception[0]='R'){						// Rotation angle donné
		if (reception[3]=='D'){
			if (length(reception)>5){
				AngleRotation='';
				for (i=5;i<length(reception);i++){
					AngleRotation=strcat(AngleRotation,reception[i]);			// angle en degres
					}
				L = 3.14*r*atoi(AngleRotation)/180;						// Longueur en cm
				L = L/5.24;																		// Longueur en inch
				L = L/0.020;																	// Longueur en ticks
				distance = itos(L);
				envoie = strcat(strcat(strcat(strcat("digo 1:",distance),":20 2:"),distance),":-20\r");
				}
			else{
				envoie = "digo 1:346:20 2:346:-20\r";
				}
		else{
				if (length(reception)>5){
				AngleRotation='';
				for (i=5;i<length(reception);i++){
					AngleRotation=strcat(AngleRotation,reception[i]);			// angle en degres
					}
				L = 3.14*r*atoi(AngleRotation)/180;						// Longueur en cm
				L = L/5.24;																		// Longueur en inch
				L = L/0.020;																	// Longueur en ticks
				distance = itos(L);
				envoie = strcat(strcat(strcat(strcat("digo 1:",distance),":-20 2:"),distance),":20\r");
				}
			else{
				envoie = "digo 1:346:-20 2:346:20\r";
				}
			}
		}	
/*	if(SBUF0=='G'){						// Deplacement jusqu'a une coordonnee
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

	 XBR2 |= 0x44; // Enable crossbar
	 XBR0 |= 0x06; // Enable UART0 et SPI??
	 XBR1 |= 0x80;  // Enable SYSCLK
	
   WDTCN = 0xDE;   // Dévalidation du watchdog 
   WDTCN = 0xAD;
	
   /*
			CONFIGURATION UART-SERIAL
			*/
	 
	 EA=1; 						// Enable interrupt
	
	 OSCXCN = 0xE7;		//	COnfiguration Clock pour le baud-rate
	 OSCICN |= 0x08; 	//	Enable external clock
	 
	 TCON |= 0x40; 	//	Config Timer 1 pour le Baud-rate UART0 et UART1
	 TMOD |= 0x20;
	 TH1 = 0xDD;
	 
	 SCON0 |= 0x50; 	//	Config UART0
	 SCON1 |= 0x50; 	//	Config UART1
	 
	 ESO = 1; 				//	Flag UART0 activé
	 ES1 = 1; 				//	Flag UART1 activé
	 
	 PCON  &= 0x6F;		// 	on active le Baud-rate UART0 UART1 en divided by two
	 
	 RI0 = 0; 				// 	Flags de reception
	 RI1 = 0;
	
	
	while(1) 
        {

         }   
	               	
}