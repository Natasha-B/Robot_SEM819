#ifndef MESURE_COURANT_H
#define MESURE_COURANT_H

//void delay3_10u(int j);
//void cfg_clk_ext_3();
void config_adc0();
void config_timer3();
void config_courant();
int conversion_adc0(char autorisation, char mesure_continue);


#endif
