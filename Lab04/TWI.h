#include <avr/io.h>

#define ACK 1
#define NACK 0

void TWI_Init(void);
void TWI_Start(void);
void TWI_Stop(void);
char TWI_Write(char data);
char TWI_Read(char ack);

