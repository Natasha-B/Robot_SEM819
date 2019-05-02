#include <C8051F020.H>
#include <stdio.h>
#include <string.h>
#include "servo_V.h"
#include "information.h"
#include "UART0.h"
#include "pointeur.h"


char* clear(char* message){
	xdata int cpt1 = 0;
	xdata int cpt2 = 0;
	xdata char clr[32] = "";
	xdata int taille = strlen(message);
	while(cpt1 < taille){
		if((message[cpt1] == ' ')||(message[cpt1] == '0')||(message[cpt1] == '1')||(message[cpt1] == '2')||(message[cpt1] == '3')||(message[cpt1] == '4')||(message[cpt1] == '5')
				||(message[cpt1] == '6')||(message[cpt1] == '7')||(message[cpt1] == '8')||(message[cpt1] == '9')||(message[cpt1] == '-')){
				clr[cpt2] = message[cpt1];
				cpt2++;
				}
		cpt1++;
	}
	clr[cpt2] = '\0';
	return clr;
}

void information(char* message){
	xdata char prem[3];
	xdata int cpt_p = 0;
	xdata int t_up, t_down, n_period, intensity;
	
	sscanf(message,"%s ",prem);
	
	if(strcmp(prem, "ASS") == 0 || (strcmp(prem, "SD") == 0)){
	  xdata char envoi[32];
		sprintf(envoi,"%s\r",message);
		putString0(envoi);
	}
	else if(strcmp(prem, "L") == 0){
		sscanf(clear(message), "%d %d %d %d", &t_up, &t_down, &n_period, &intensity); 
		start_blink_led(t_up, t_down, n_period, intensity);
		
	}
	else if(strcmp(prem, "LS") == 0){
		led_off();
		
	}
	//Test si la commande est relative au servomoteur vertical
	else if(strcmp(prem, "CS") == 0){
		xdata int angle;
		//Recuperation de la valeur de l'angle souhaite
		sscanf(clear(message),"%d",&angle);
		if (((angle)>= -90)&&(angle<=90)){
		//Fonction permettant de faire roter le servomoteur selon l'angle souhaite
			chg_servo_pos_v(angle);
		}
		}
	}