//******************************************************************************
//
//    THE SOFTWARE INCLUDED IN THIS FILE IS FOR GUIDANCE ONLY.
//    BTC KORPORACJA SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
//    OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
//    FROM USE OF THIS SOFTWARE.
//
//******************************************************************************
#include <avr/io.h>

void SPI_Init(void)
{
DDRB = (1<<PB7)|(1<<PB4);
SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

char SPI_Transmit(char cData)
{
SPDR = cData;
while(!(SPSR & (1<<SPIF)));
return SPDR;
}
