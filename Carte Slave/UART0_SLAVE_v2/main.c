#include <c8051f020.h>
#include <string.h>

#define SYSCLK 22118400UL //approximate SYSCLK frequency in Hz
#define BAUDRATE  19200UL          // Baud rate of UART in bps
#define Preload_Timer0 (SYSCLK/(BAUDRATE*16))


void Oscillator_Init()
{
    int i = 0;
    OSCXCN    = 0x67;
    for (i = 0; i < 3000; i++);  // Wait 1ms for initialization
    while ((OSCXCN & 0x80) == 0);
    OSCICN    = 0x0C;
}

void cfg_Clock_UART(void){
  CKCON |= 0x10;      // T1M: Timer 1 use the system clock.
  TMOD |= 0x20;       //  Timer1 CLK = system clock
	TMOD &= 0x2f;			  // Timer1 configuré en timer 8 bit avec auto-reload	
	TF1 = 0;				  // Flag Timer effacé
	
	TH1 = -(Preload_Timer0);
	ET1 = 0;				   // Interruption Timer 1 dévalidée
	TR1 = 1;				   // Timer1 démarré

}

void cfg_UART0_mode1(void){
		RCLK0 = 0;     // Source clock Timer 1
		TCLK0 = 0;
		PCON  |= 0x80; //SMOD0: UART0 Baud Rate Doubler Disabled.
		PCON &= 0xBF;  // SSTAT0=0
		SCON0 = 0x70;   // Mode 1 - Check Stop bit - stock validée
	
		XBR0 |= 0x04;
		XBR2 |= 0x40;
		P0MDOUT |= 0x01;
	
    ES0 = 1;        // interruption UART0 autorisée	

}

void putChar0(char carac){
	SBUF0=carac;
	while((SCON0 & 0x02)== 0){
		}
	SCON0 &= 0xFD;
	}
	
void putString0(char chaine[32]){
		int i;
		for (i=0;i<strlen(chaine);i++){
			putChar0(chaine[i]);
		}
}

void main(){
	WDTCN     = 0xDE;
  WDTCN     = 0xAD;
	Oscillator_Init();
	cfg_Clock_UART();
	cfg_UART0_mode1();


	
putString0("salut michel");
while(1){
}
}