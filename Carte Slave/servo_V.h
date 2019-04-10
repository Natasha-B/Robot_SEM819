#ifndef SERVO_V_H
#define SERVO_V_H

void config_PCA();
void chg_servo_pos_v(int pos);
void cfg_clk_ext(void);
void delay(int j);
unsigned int pos2timer_count(int pos);
#endif
