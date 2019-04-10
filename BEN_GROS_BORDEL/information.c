#include <C8051F020.H>
#include <stdio.h>
#include <string.h>

#include "information.h"
#include "UART0.h"
#include "pointeur.h"



void information(char* message){
	char prem[3];
	int cpt_p = 0;
	int t_up, t_down, n_period, intensity;
	char* b1,b2,b3,b4;
	
	sscanf(message,"%s ",prem);
	
	if(strcmp(prem, "ASS") == 0 || (strcmp(prem, "SD") == 0)){
		putString0(message);
	}
	else if(strcmp(prem, "L") == 0){
		sscanf(message, "%s:%d %s:%d %s:%d %s:%d",b1, &t_up,b2, &t_down,b3, &n_period,b4, &intensity); 
		
		start_blink_led(t_up, t_down, n_period, intensity);
	}
	else if(strcmp(prem, "LS") == 0){
		led_off();
		
	}
	else if(strcmp(prem, "CS") == 0){
	}
	else{};
}