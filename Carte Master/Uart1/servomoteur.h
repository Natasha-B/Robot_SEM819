#ifndef SERVOMOTEUR_H
#define SERVOMOTEUR_H

void delay(int j);
void cfg_timer4();
void cfg_servo();
unsigned int pos2timer_count(int pos);
void servo_pos(int pos);
void config_servo();

#endif