#include <C8051F020.H>
#include <stdio.h>
#include <string.h>

#include "information.h"
#include "UART0.h"
#include "pointeur.h"


char* clear(char* message){
	int cpt1 = 0;
	int cpt2 = 0;
	xdata char clr[32] = "";
	int taille = strlen(message);
	while(cpt1 < taille){
		if((message[cpt1] == ' ')||(message[cpt1] == '0')||(message[cpt1] == '1')||(message[cpt1] == '2')||(message[cpt1] == '3')||(message[cpt1] == '4')||(message[cpt1] == '5')
				||(message[cpt1] == '6')||(message[cpt1] == '7')||(message[cpt1] == '8')||(message[cpt1] == '9')){
				clr[cpt2] = message[cpt1];
				cpt2++;
				}
		cpt1++;
	}
	clr[cpt2] = '\0';
	return clr;
}

void information(char* message){
	char prem[3];
	int cpt_p = 0;
	int t_up, t_down, n_period, intensity;
	
	sscanf(message,"%s ",prem);
	
	if(strcmp(prem, "ASS") == 0 || (strcmp(prem, "SD") == 0)){
		putString0(message);
	}
	else if(strcmp(prem, "L") == 0){
		sscanf(clear(message), "%d %d %d %d", &t_up, &t_down, &n_period, &intensity); 
		start_blink_led(t_up, t_down, n_period, intensity);
		
	}
	else if(strcmp(prem, "LS") == 0){
		led_off();
		
	}
	else if(strcmp(prem, "CS") == 0){
		int angle;
		sscanf(clear(message),"%d",&angle);
		///servo_vertical(angle);
		}
	}