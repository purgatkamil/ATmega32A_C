#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/sleep.h>
#include <avr/wdt.h>
#include "HD44780.h"

#include "USART.h"
#include "uart.h"
//#include "rprintf.h"
#include "SPI.h"

#include "TWI.c"
#include "rprintf.h"

int temperaturka = 0;

unsigned int W = 0;
unsigned int adc = 0;

char str[20];
char voltage_t[20];
char temperature[10];

int Godziny = 0;
int Minuty = 0;
int Sekundy = 0;
char time[7];

unsigned int volt = 0;

int B1, B2, B3;

int previous = 0;
int mode = 1;

int watchdog = 0;

char Message[10];

//int i = 0;
char *xd;

void adc_read()
{
	W = ADCL;
	W |= (ADCH << 8);
	adc = (W * 5 * 10) / 1024;
}


/*int adc_read(char channel)
{
	unsigned int W = 0;
	ADMUX |= channel;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADIF));
	ADCSRA |= (1 << ADIF);
	W = ADCL;
	W |= (ADCH << 8);
	adc = (W * 5 * 10) / 1024;
	return adc;
}*/

void SPI_Init(void)
{
	DDRB = (1<<PB7)|(1<<PB4);
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void ADC_Init(void)
{
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0) | (1 << ADIE) ;// | (1 << ADATE) | (1 << ADSC);// | (1 << ADIF);
}

void AutoTrigger_Init(void)
{
	ADCSRA |= (1 << ADATE);
	ADCSRA |= (1 << ADSC);
}

char SPI_Transmit(char cData)
{
	SPDR = cData;
	while(!(SPSR & (1<<SPIF)));
	return SPDR;
}

/*void ADC_Init(void)
{
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}*/

void Timer1_Init(){
	TCCR1B |= (1 <<CS12) | (1 << WGM12) | (1<<CS10);
	TIMSK |= (1 << OCIE1A);
	OCR1A = 1563;
}

void USART_Int_Init()
{
	UCSRB |= (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
}


ISR(ADC_vect)
{
	adc_read();
}



void setTime(int Hours, int Minutes, int Seconds)
{
	int jednosciHours = Hours % 10;
	int dziesiatkiHours =  (Hours - jednosciHours) / 10;
	int wynikHours = 0;
	dziesiatkiHours = dziesiatkiHours << 4;
	wynikHours = dziesiatkiHours + jednosciHours;
	
	int jednosciMinutes = Minutes % 10;
	int dziesiatkiMinutes =  (Minutes - jednosciMinutes) / 10;
	int wynikMinutes = 0;
	dziesiatkiMinutes = dziesiatkiMinutes << 4;
	wynikMinutes = dziesiatkiMinutes + jednosciMinutes;
	
	int jednosciSeconds = Seconds % 10;
	int dziesiatkiSeconds =  (Seconds - jednosciSeconds) / 10;
	int wynikSeconds = 0;
	dziesiatkiSeconds = dziesiatkiSeconds << 4;
	wynikSeconds = dziesiatkiSeconds + jednosciSeconds;
	
	TWI_Start();                        // start transmission
	TWI_Write(0xD0);                    // write addres of M41T00
	TWI_Write(0x00);                    // select hours register                                           // write address for reading data
	TWI_Write(wynikSeconds);			// read hours register
	TWI_Write(wynikMinutes);			// read hours register                                        // write address for reading data
	TWI_Write(wynikHours);				// read hours register
	TWI_Stop();							// stop transmission 
}

void getTime(int *hr,int *min,int *sec)
{
	TWI_Start();                        // start transmission
	TWI_Write(0xD0);                    // write addres of M41T00
	TWI_Write(0x00);                    // select hours registers
	TWI_Start();                        // repeated start
	TWI_Write(0xD1);                    // write address for reading data
	*sec = TWI_Read(ACK);                // read hours register                   // write address for reading data
	*min = TWI_Read(ACK);                // read hours register                  // write address for reading data
	*hr= TWI_Read(NACK);                // read hours register
	TWI_Stop();                         // stop transmission
	
	/*TWI_Start();                        // start transmission
	TWI_Write(0xD0);                    // write addres of M41T00
	TWI_Write(0x01);                    // select hours registers
	TWI_Start();                        // repeated start
	TWI_Write(0xD1);                    // write address for reading data
	*min = TWI_Read(NACK);                // read hours register
	TWI_Stop(); 
	 
	TWI_Start();                        // start transmission
	TWI_Write(0xD0);                    // write addres of M41T00
	TWI_Write(0x00);                    // select hours registers
	TWI_Start();                        // repeated start
	TWI_Write(0xD1);                    // write address for reading data
	*sec= TWI_Read(NACK);                // read hours register
	TWI_Stop();  */
	
}

enum TemperatureScales{Celsius = 0, Fahrenheit = 1};

int getTemp(int t)
{
	int A1,A2,A3;
	
	PORTB &= ~(1 << PB4); // Enable CS
	A1 = SPI_Transmit(0); // read byte
	A2 = SPI_Transmit(0); // read byte
	PORTB |= (1 << PB4);  // Disable CS
	
	A1 = A1 * 256;
	A2 = A2 - 7;
	A1 = (A1 + A2);
	A1 = A1 >> 3;
	A3 = A1;
	A3 *= 0.625;
	
	if(t == 0)
	{
		return A3;
	}
	else if(t == 1) 
	{
		A3 = ((A3 * 9) / 5 ) + 32;
		return A3;
	}
	else
	{
		A3 = 0;
		return A3;
	}
}

/*int getVoltage()
{
	int x = adc_read(0);
	return x;
}*/

void displayData(int mode, int x, int y)
{
	LCD_Clear();
	switch (mode)
	{
		case 1:
			
			LCD_GoTo(x,y);
			LCD_WriteText("Voltage: ");
			
			if (adc >= 10)
			{
				itoa(adc, voltage_t, 10);
				uartAddToTxBuffer('\r');
				uartAddToTxBuffer('\n');
				uartAddToTxBuffer(voltage_t[0]);
				uartAddToTxBuffer('.');
				uartAddToTxBuffer(voltage_t[1]);
				uartSendTxBuffer();
				
				LCD_GoTo(x+9,y);
				LCD_WriteData(voltage_t[0]);
				LCD_GoTo(x+10,y);
				LCD_WriteText(".");
				LCD_GoTo(x+11,y);
				LCD_WriteData(voltage_t[1]);
			}
			else
			{
				itoa(adc, voltage_t, 10);
				uartAddToTxBuffer('\r');
				uartAddToTxBuffer('\n');
				uartAddToTxBuffer(voltage_t[0]);

				uartSendTxBuffer();
				
				LCD_GoTo(x+9,y);
				LCD_WriteText("0");
				LCD_GoTo(x+10,y);
				LCD_WriteText(".");
				LCD_GoTo(x+11,y);
				LCD_WriteData(voltage_t[0]);
			}
			break;
			
		case 2:
				
				LCD_GoTo(x, y);
				LCD_WriteText("Temp: ");
				
				itoa(temperaturka, temperature, 10);
				uartAddToTxBuffer('\r');
				uartAddToTxBuffer('\n');
				uartAddToTxBuffer(temperature[0]);
				uartAddToTxBuffer(temperature[1]);
				uartAddToTxBuffer('.');
				uartAddToTxBuffer(temperature[2]);
				uartSendTxBuffer();
				LCD_GoTo(x+7, y);
				LCD_WriteData(temperature[0]);
				LCD_GoTo(x+8, y);
				LCD_WriteData(temperature[1]);
				LCD_GoTo(x+9, y);
				LCD_WriteText(".");
				LCD_GoTo(x+10, y);
				LCD_WriteData(temperature[2]);
				
				break;
		
		case 3:
				
				B2 = ((Godziny>>4)&7);     // convert seconds to ASCII char
				B3 = (Godziny&15);         // convert seconds to ASCII char
				itoa(B2, str, 10);
				LCD_GoTo(x, y);
				LCD_WriteData(str[0]);
				uartAddToTxBuffer('\r');
				uartAddToTxBuffer('\n');
				uartAddToTxBuffer(str[0]);
				
				itoa(B3, str, 20);
				LCD_GoTo(x+1, y);
				LCD_WriteData(str[0]);
				uartAddToTxBuffer(str[0]);
				//LCD_WriteText(str);

				LCD_GoTo(x+2,y);
				LCD_WriteText(":");
				uartAddToTxBuffer(':');

				B2 = ((Minuty>>4)&7);     // convert seconds to ASCII char
				B3 = (Minuty&15);         // convert seconds to ASCII char
				itoa(B2, str, 20);
				LCD_GoTo(x+3, y);
				LCD_WriteData(str[0]);
				uartAddToTxBuffer(str[0]);
				itoa(B3, str, 20);
				LCD_GoTo(x+4, y);
				LCD_WriteData(str[0]);
				uartAddToTxBuffer(str[0]);
				
				LCD_GoTo(x+5, y);
				LCD_WriteText(":");
				uartAddToTxBuffer(':');

				B2 = ((Sekundy>>4)&7);     // convert seconds to ASCII char
				B3 = (Sekundy&15);         // convert seconds to ASCII char
				itoa(B2, str, 20);
				LCD_GoTo(x+6, y);
				LCD_WriteData(str[0]);
				uartAddToTxBuffer(str[0]);
				itoa(B3, str, 20);
				LCD_GoTo(x+7, y);
				LCD_WriteData(str[0]);
				uartAddToTxBuffer(str[0]);
				
				uartSendTxBuffer();
				break;
	}
}

void checkBtn()
{
	if (!(PINB & 1))
	{
		if(previous == 0)
		{
			mode = mode + 1;
			LCD_WriteCommand(HD44780_CLEAR);
			previous = 1;
			if(mode >= 4)
			{
				mode = 1;
			}
		}
	}
	else
	{
		previous = 0;
	}
}

void USART_Receiving()
{
		cBuffer* bufferek = uartGetRxBuffer();
		
		bufferek->dataptr[bufferek->datalength - 1] = NULL;
		xd = bufferek->dataptr;
		bufferek->datalength = 0;
		if(strcmp(xd, "volt") == 0)
		{
			LCD_WriteText("Volt");
			mode = 1;
		}
		else if(strcmp(xd, "temp") == 0)
		{
			LCD_WriteText("Temp");
			mode = 2;
		}
		else if(strcmp(xd, "time") == 0)
		{
			LCD_WriteText("Time");
			mode = 3;
			watchdog = 1;
			while(1);
			
		}
		else if(strcmp(xd, "settime") == 0)
		{
			cli();	
			USART_GetString(&time);
			sei();
			int hours = ((time[0] - 48) * 10) + (time[1] - 48);		
			int minutes = ((time[2] - 48) * 10) + (time[3] - 48);
			int seconds = ((time[4] - 48) * 10) + (time[5] - 48);
			if((hours > 23) || minutes > 59 || seconds > 59)
			{
				USART_PutString("Wrong time format\r");	
				mode = 3;
			}
			else
			{
				setTime(hours, minutes, seconds);
				mode = 3;
			}
			
		}
		
		//displayData(mode, 0, 1);
		displayData(mode, 0, 1);		
}

ISR(TIMER1_COMPA_vect){
	getTime(&Godziny, &Minuty, &Sekundy);
	temperaturka = getTemp(Celsius);
	if(watchdog == 0)
		wdt_reset();
}

int main(void)
{
DDRD = 0xFF;
	
DDRD |= (1 << PD1);

LCD_Initialize();
ADC_Init();
AutoTrigger_Init();
SPI_Init();
TWI_Init();
Timer1_Init();

setTime(16, 17, 18);



uartInit();
uartSetBaudRate(9600);
uartInitBuffers();
rprintfInit(uartSendByte);
cBuffer* bufferek = uartGetRxBuffer();    

USART_Int_Init();

set_sleep_mode(SLEEP_MODE_IDLE);
sleep_enable();
wdt_enable(7);
sei();

while(1)
{
	//wdt_reset();
	while(bufferek->dataptr[bufferek->datalength - 1] != 13); 
	{
		//wdt_reset();
		sleep_cpu();	
	}
	USART_Receiving();
	

//_delay_ms(100);
	}
return 0;
}
//-------------------------------------
// End of file
//-------------------------------------
