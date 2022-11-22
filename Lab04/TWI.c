//******************************************************************************
//
//    THE SOFTWARE INCLUDED IN THIS FILE IS FOR GUIDANCE ONLY.
//    BTC KORPORACJA SHALL NOT BE HELD LIABLE FOR ANY DIRECT, INDIRECT
//    OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
//    FROM USE OF THIS SOFTWARE.
//
//******************************************************************************
#include "TWI.h"

#define ADRW_ACK 18
#define DATA_ACK 28
//-------------------------------------
// Generate START signal
//-------------------------------------
void TWI_Start(void)
{
TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
while (!(TWCR & (1<<TWINT)));
}
//-------------------------------------
// Generate STOP signal
//-------------------------------------
void TWI_Stop(void)
{
TWCR = (1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
while ((TWCR & (1<<TWSTO)));
}
//-------------------------------------
// Write byte to slave
//-------------------------------------
char TWI_Write(char data)
{
TWDR = data;
TWCR = (1<<TWINT) | (1<<TWEN);
while (!(TWCR & (1<<TWINT)));
if((TWSR == ADRW_ACK) | (TWSR == DATA_ACK))
  return 0;
else
  return 1;
}
//-------------------------------------
// Read byte from slave
//-------------------------------------
char TWI_Read(char ack)
{
TWCR = ack 
	   ? ((1 << TWINT) | (1 << TWEN) | (1 << TWEA)) 
	   : ((1 << TWINT) | (1 << TWEN)) ;
while (!(TWCR & (1<<TWINT)));
return TWDR;
}
//-------------------------------------
// Initialize TWI
//-------------------------------------
void TWI_Init(void)
{
TWBR = 100;
}
//-------------------------------------
// End of file
//-------------------------------------
