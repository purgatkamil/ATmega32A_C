//***************** (c) 2008 BTC Korporacja ***********************************
//                     http://www kamami.com
//
//    THE SOFTWARE INCLUDED IN THIS FILE IS FOR GUIDANCE ONLY.
//    BTC KORPORACJA SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
//    OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
//    FROM USE OF THIS SOFTWARE.
//
//******************************************************************************
#include <avr/io.h>
//-------------------------------------
// USART Initialization
//-------------------------------------
void USART_Init(int baud)
{
UBRRH = (unsigned char)(baud>>8);
UBRRL = (unsigned char)baud;
UCSRB = (1<<RXEN)|(1<<TXEN);
UCSRC = (1<<URSEL)|(3<<UCSZ0);
}
//-------------------------------------
// Send sgngle char 
//-------------------------------------
void USART_PutChar(unsigned char data)
{
while (!(UCSRA & (1 << UDRE)));
UDR = data;
}
//-------------------------------------
// Receive single char
//-------------------------------------
unsigned char USART_GetChar(void)
{
while (!(UCSRA & (1 << RXC)));
return UDR;
}
//-------------------------------------
// Send string
//-------------------------------------
void USART_PutString(char * s)
{
while(*s)
	USART_PutChar(*s++);
}
//-------------------------------------
// Receive string
//-------------------------------------
void USART_GetString(char * s)
{
char ch;

do{
  ch = USART_GetChar();
  USART_PutChar(ch);
  *s++ = ch;
  if(ch == 0x0D) *s = 0;
}while(ch != 0x0D);
}
//-------------------------------------
// Conver 8-bit value to hex
//-------------------------------------
char * Hex(char val, char*str)
{
*str = ((val >> 4) < 10 )?(val >> 4)+'0':(val >> 4)+'7';
*(str+1) = ((val & 0xF) < 10 )?(val & 0xF)+'0':(val & 0xF)+'7';
*(str+2) = 0;
return str;
}
//-------------------------------------
// End of file
//-------------------------------------
