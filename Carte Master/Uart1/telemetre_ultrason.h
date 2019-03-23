#ifndef TELEMETRE_ULTRASON_H
#define TELEMETRE_ULTRASON_H



void delay_10u(int j);
void conf_port_sortie();
void activation_interrupt_6();
void cfg_timer0();
void config_telem_ultrason();
float calc_dist();

#endif