void init_Serial_Buffer(void);
unsigned char serOutchar(char c);
char serInchar(void); 
unsigned int serInchar_Bin(void);
unsigned char serOutstring(char *buf);
void cfg_Clock_UART(void);
void cfg_UART0_mode1(void);
void cfg_UART1_mode1(void);
void ecoute(void);
void sendUART0(char c);
void mon_itoa(int chiffre, char* distance_obs);
	