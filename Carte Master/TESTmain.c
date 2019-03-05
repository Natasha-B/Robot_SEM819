#include <c8051f020.h>                    // SFR declarations
#include <stdio.h>
#include <intrins.h>
#include <string.h>

#include <UART0_RingBuffer_lib.h>
#ifndef CFG_Globale
  #define CFG_Globale
  #include <CFG_Globale.h>
#endif

sbit BP = P3^7;     


// ****************************************************************************
//
//*****************************************************************************
void Oscillator_Init()
{
    int i = 0;
    OSCXCN    = 0x67;
    for (i = 0; i < 3000; i++);  // Wait 1ms for initialization
    while ((OSCXCN & 0x80) == 0);
    OSCICN    = 0x0C;
}
void Port_IO_Init()
{
   P0MDOUT   =  0x01;
	 XBR0      = 0x04;
   XBR2      = 0x40;
	 P3        |= 0x80;
}

// **************************************************************************************************
// MAIN
// **************************************************************************************************
                     /* size of local string buffer */ 



void main(void) {
    char  c;
     WDTCN     = 0xDE;
     WDTCN     = 0xAD;
	   Oscillator_Init();
	   Port_IO_Init();
	   
	   cfg_Clock_UART();
     cfg_UART0_mode1();
     init_Serial_Buffer();                            
     EA = 1;                              /* allow interrupts to happen */
	   serOutstring("\n\rTest_Buffer_Circulaire\n\r");
	
while(1) {

} /* main */
