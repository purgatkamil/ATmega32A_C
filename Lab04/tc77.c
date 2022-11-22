// ***************** (c) 2008 BTC Korporacja ***********************************
//                     http://www kamami.com
// PROJECT  : TC77
// BOARD    : ZL15AVR (with ATmega32 in socket U1)
// COMPILER : WinAVR 20080610
//
//******************************************************************************
//
//    THE SOFTWARE INCLUDED IN THIS FILE IS FOR GUIDANCE ONLY.
//    BTC KORPORACJA SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
//    OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
//    FROM USE OF THIS SOFTWARE.
//
//******************************************************************************
//
// DESCRIPTION :
//    Simple SPI communication demo.
//    Read temperature from temperature sensor (U7 - TC77)
//    and transmit it to PC via UART.
//
//    Settings and connections on ZL15AVR:
//        SIO (U7 TEM/con8) - PB6 (con16)
//        CS  (U7 TEM/con8) - PB4 (con16)
//        SCK (U7 TEM/con8) - PB7 (con16)
//
//      if RS232 cable connected:
//        RxD (con7) - PD0 (con18)
//        TxD (con7) - PD1 (con18)
//
//      if UART2USB converter (ZL1USB_A or ZL4USB) connected:
//        RxU (con7) - PD0 (con18)
//        TxU (con7) - PD1 (con18)
//
//******************************************************************************
#ifndef F_CPU  
  #define F_CPU   16000000    // 16 MHz - turn on external resonator (fuse bits)
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "USART.h"
#include "SPI.h"
//-------------------------------------
// main program
//-------------------------------------
int main(void)
{
char A1,A2;
char str[5];

USART_Init(BAUD(1200));
SPI_Init();

USART_PutString("\r");
USART_PutString("*************************\r");
USART_PutString("**     ZL15AVR  TC77   **\r");
USART_PutString("**    www.kamami.com   **\r");
USART_PutString("*************************\r");

do{
  PORTB &= ~(1 << PB4); // Enable CS
  A1 = SPI_Transmit(0); // read byte
  A2 = SPI_Transmit(0); // read byte
  PORTB |= (1 << PB4);  // Disable CS

  USART_PutString("TC77 output (hex): ");
  USART_PutString(Hex(A1,str)); // convert to hex and send 
  USART_PutString(Hex(A2,str)); // convert to hex and send
  USART_PutChar('\r');

}while(1);

return 0;
}
//-------------------------------------
// End of file
//-------------------------------------
