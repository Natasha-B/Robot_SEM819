//------------------------------------------------------------------------------------
// Projet_BSE.h
//------------------------------------------------------------------------------------
// DATE: 19/12/18
//
// Target: C8051F02x
// Tool chain: KEIL Microvision 4
//
//  NOMS: Albert, Avinain, Bru, Jannin
//
// Num�ro du sujet: 1
// Bin�me : B (Avinain, Bru)
//
//------------------------------------------------------------------------------------

void init (void);
void UART_receive (void);
void Welcome();
float mesure (int);
void run (int angle);
int get_CSG_min(void);
int get_CSG_max(void);
void auto_mode();
void show_diagram(float);
