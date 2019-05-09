#include <c8051f020.h>                    // SFR declarations

#ifndef CFG_Globale
   #define CFG_Globale
   #include <CFG_Globale.h>
#endif

#define Preload_Timer0 (SYSCLK/(BAUDRATE*16))
#if Preload_Timer0 > 255 
#error "Valeur Preload Timer0 HORS SPECIFICATIONS"
#endif 

void cfg_Clock_UART(void){
  CKCON |= 0x10;      // T1M: Timer 1 use the system clock.
  TMOD |= 0x20;       //  Timer1 CLK = system clock
	TMOD &= 0x2f;			  // Timer1 configur� en timer 8 bit avec auto-reload	
	TF1 = 0;				  	// Flag Timer effac�
	
	TH1 = -(Preload_Timer0);
	ET1 = 0;				   // Interruption Timer 1 d�valid�e
	TR1 = 1;				   // Timer1 d�marr�

	T2CON |= 0x04;		// Timer 4 auto-reload, clock divise par 12(crossbar) avec comme valeur max 65536 donc 0.5425 us par coup
}

//*************************************************************************************************
//  CONFIGURATION BAS NIVEAU de L'UART0
		//mode1 

//*************************************************************************************************

void cfg_UART0_mode1(void)
{
		RCLK0 = 0;     // Source clock Timer 1
		TCLK0 = 0;
		PCON  |= 0x80; //SMOD0: UART0 Baud Rate Doubler Disabled.
		PCON &= 0xBF;  // SSTAT0=0
		SCON0 = 0x70;   // Mode 1 - Check Stop bit - stock valid�e
		TI0 = 1;        // Transmission: octet transmis (pr�t � recevoir un char)
					          // pour transmettre			
    ES0 = 1;        // interruption UART0 autoris�e	
}

//*************************************************************************************************
//  CONFIGURATION BAS NIVEAU de L'UART1
			//Mode1
//*************************************************************************************************
void cfg_UART1_mode1(void)
{
		PCON  |= 0x10; //SMOD0: UART1 Baud Rate Doubler Disabled.
		PCON &= 0xF7;  // SSTAT1=0
		SCON1 = 0x70;   // Mode 1 - Check Stop bit - stock valid�e
	
    IE |= 0x20;        // interruption UART1 autoris�e	ES1=1
}